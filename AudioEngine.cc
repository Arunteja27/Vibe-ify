#include "AudioEngine.h"
#include <cmath>
#include <cstring>


AudioEngine::AudioEngine()
    : hWaveOut(nullptr), currentNode(nullptr), volumeEffect(nullptr),
      sampleRate(44100), numChannels(2), bufferFrames(4096), currentBuffer(0),
      initialized(false), playing(false), paused(false), volume(1.0f) {
  outputBuffers[0] = nullptr;
  outputBuffers[1] = nullptr;
  renderBuffers[0] = nullptr;
  renderBuffers[1] = nullptr;
  memset(&waveFormat, 0, sizeof(WAVEFORMATEX));
  memset(waveHeaders, 0, sizeof(waveHeaders));
}

AudioEngine::~AudioEngine() { shutdown(); }

bool AudioEngine::init(int sr, int ch, int bf) {
  if (initialized)
    shutdown();

  sampleRate = sr;
  numChannels = ch;
  bufferFrames = bf;

  // Setup WAVEFORMATEX for 16-bit PCM output
  waveFormat.wFormatTag = WAVE_FORMAT_PCM;
  waveFormat.nChannels = numChannels;
  waveFormat.nSamplesPerSec = sampleRate;
  waveFormat.wBitsPerSample = 16;
  waveFormat.nBlockAlign = numChannels * (waveFormat.wBitsPerSample / 8);
  waveFormat.nAvgBytesPerSec = sampleRate * waveFormat.nBlockAlign;
  waveFormat.cbSize = 0;

  // Open waveOut device
  MMRESULT result = waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveFormat,
                                (DWORD_PTR)waveOutCallback, (DWORD_PTR)this,
                                CALLBACK_FUNCTION);

  if (result != MMSYSERR_NOERROR) {
    cerr << "AudioEngine::init: waveOutOpen failed (error " << result << ")"
         << endl;
    return false;
  }

  // Allocate double buffers
  int bufferSamples = bufferFrames * numChannels;

  for (int i = 0; i < 2; ++i) {
    outputBuffers[i] = new short[bufferSamples];
    memset(outputBuffers[i], 0, bufferSamples * sizeof(short));

    renderBuffers[i] = new AudioBuffer(bufferFrames);

    waveHeaders[i].lpData = reinterpret_cast<LPSTR>(outputBuffers[i]);
    waveHeaders[i].dwBufferLength = bufferSamples * sizeof(short);
    waveHeaders[i].dwFlags = 0;

    waveOutPrepareHeader(hWaveOut, &waveHeaders[i], sizeof(WAVEHDR));
  }

  initialized = true;
  currentBuffer = 0;
  return true;
}

void AudioEngine::shutdown() {
  if (!initialized)
    return;

  stop();

  // Unprepare and free buffers
  for (int i = 0; i < 2; ++i) {
    waveOutUnprepareHeader(hWaveOut, &waveHeaders[i], sizeof(WAVEHDR));

    delete[] outputBuffers[i];
    outputBuffers[i] = nullptr;

    delete renderBuffers[i];
    renderBuffers[i] = nullptr;
  }

  waveOutClose(hWaveOut);
  hWaveOut = nullptr;
  initialized = false;
}

bool AudioEngine::play(AudioNode *node) {
  if (!initialized || !node)
    return false;

  stop();

  currentNode = node;

  // Wrap in VolumeEffect for real-time volume control
  if (volumeEffect) {
    delete volumeEffect;
  }
  volumeEffect = new VolumeEffect(currentNode, volume);

  playing = true;
  paused = false;

  // Prime both buffers and start playback
  prepareBuffer(0);
  waveOutWrite(hWaveOut, &waveHeaders[0], sizeof(WAVEHDR));

  prepareBuffer(1);
  waveOutWrite(hWaveOut, &waveHeaders[1], sizeof(WAVEHDR));

  currentBuffer = 0;
  return true;
}

void AudioEngine::pause() {
  if (!playing || paused)
    return;
  waveOutPause(hWaveOut);
  paused = true;
}

void AudioEngine::resume() {
  if (!playing || !paused)
    return;
  waveOutRestart(hWaveOut);
  paused = false;
}

void AudioEngine::stop() {
  if (!initialized)
    return;

  playing = false;
  paused = false;

  waveOutReset(hWaveOut);

  if (volumeEffect) {
    delete volumeEffect;
    volumeEffect = nullptr;
  }
  currentNode = nullptr;
}

void AudioEngine::setVolume(float vol) {
  if (vol < 0.0f)
    vol = 0.0f;
  if (vol > 1.0f)
    vol = 1.0f;
  volume = vol;
  if (volumeEffect) {
    volumeEffect->setGain(volume);
  }
}

float AudioEngine::getVolume() const { return volume; }
bool AudioEngine::isPlaying() const { return playing; }
bool AudioEngine::isPaused() const { return paused; }
bool AudioEngine::isInitialized() const { return initialized; }

void AudioEngine::prepareBuffer(int bufferIndex) {
  AudioBuffer *renderBuf = renderBuffers[bufferIndex];
  renderBuf->clear();

  int framesRead = 0;
  if (volumeEffect && playing) {
    framesRead = volumeEffect->process(renderBuf->raw(), bufferFrames);
  }

  // If no more frames, fill with silence (already cleared)
  if (framesRead == 0 && playing) {
    playing = false;
  }

  // Clamp to prevent clipping
  renderBuf->clamp();

  // Convert float [-1,1] to int16 for waveOut
  convertFloatToInt16(renderBuf->raw(), outputBuffers[bufferIndex],
                      bufferFrames * numChannels);
}

void AudioEngine::convertFloatToInt16(const float *src, short *dst,
                                      int numSamples) {
  // Direct pointer-based conversion — no allocation, minimized CPU cycles
  for (int i = 0; i < numSamples; ++i) {
    float sample = src[i];
    if (sample > 1.0f)
      sample = 1.0f;
    if (sample < -1.0f)
      sample = -1.0f;
    dst[i] = static_cast<short>(sample * 32767.0f);
  }
}

void CALLBACK AudioEngine::waveOutCallback(HWAVEOUT hwo, UINT uMsg,
                                           DWORD_PTR dwInstance,
                                           DWORD_PTR dwParam1,
                                           DWORD_PTR dwParam2) {
  if (uMsg != WOM_DONE)
    return;

  AudioEngine *engine = reinterpret_cast<AudioEngine *>(dwInstance);
  if (!engine || !engine->playing || engine->paused)
    return;

  // Determine which buffer just finished
  WAVEHDR *completedHeader = reinterpret_cast<WAVEHDR *>(dwParam1);
  int bufIdx = (completedHeader == &engine->waveHeaders[0]) ? 0 : 1;

  // Refill the completed buffer and resubmit
  engine->prepareBuffer(bufIdx);
  waveOutWrite(engine->hWaveOut, &engine->waveHeaders[bufIdx], sizeof(WAVEHDR));
}

void AudioEngine::renderBlock() {
  // Manual render for non-callback mode (e.g. testing)
  if (!playing || paused)
    return;
  prepareBuffer(currentBuffer);
  waveOutWrite(hWaveOut, &waveHeaders[currentBuffer], sizeof(WAVEHDR));
  currentBuffer = (currentBuffer + 1) % 2;
}
