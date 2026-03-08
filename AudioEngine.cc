#include "AudioEngine.h"
#include <cmath>
#include <cstring>

using namespace std;

AudioEngine::AudioEngine()
    : hWaveOut(nullptr), currentNode(nullptr), rawSource(nullptr),
      echoLevel(0.0f), reverbLevel(0.0f), bassLevel(0.0f),
      distortionLevel(0.0f), speedFactor(1.0f), sampleRate(44100),
      numChannels(2), bufferFrames(4096), currentBuffer(0), initialized(false),
      playing(false), paused(false), volume(1.0f) {
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

  waveFormat.wFormatTag = WAVE_FORMAT_PCM;
  waveFormat.nChannels = numChannels;
  waveFormat.nSamplesPerSec = sampleRate;
  waveFormat.wBitsPerSample = 16;
  waveFormat.nBlockAlign = numChannels * (waveFormat.wBitsPerSample / 8);
  waveFormat.nAvgBytesPerSec = sampleRate * waveFormat.nBlockAlign;
  waveFormat.cbSize = 0;

  MMRESULT result = waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveFormat,
                                (DWORD_PTR)waveOutCallback, (DWORD_PTR)this,
                                CALLBACK_FUNCTION);

  if (result != MMSYSERR_NOERROR) {
    cerr << "AudioEngine::init: waveOutOpen failed (error " << result << ")"
         << endl;
    return false;
  }

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

void AudioEngine::cleanupEffectChain() {
  if (!currentNode || currentNode == rawSource)
    return;

  // Walk the chain and delete effects (but NOT the raw source)
  AudioNode *node = currentNode;
  while (node && node != rawSource) {
    AudioEffect *effect = dynamic_cast<AudioEffect *>(node);
    if (effect) {
      AudioNode *src = effect->getSource();
      delete effect;
      node = src;
    } else {
      break;
    }
  }
  currentNode = nullptr;
}

void AudioEngine::rebuildEffectChain() {
  if (!rawSource)
    return;

  cleanupEffectChain();

  // Build chain: source → speed → bass → echo → reverb → distortion → volume
  AudioNode *chain = rawSource;

  if (speedFactor != 1.0f)
    chain = new SpeedEffect(chain, speedFactor);

  if (bassLevel > 0.0f)
    chain = new BassBoostEffect(chain, bassLevel * 3.0f, 200.0f);

  if (echoLevel > 0.0f)
    chain = new EchoEffect(chain, (int)(sampleRate * 0.3f), echoLevel);

  if (reverbLevel > 0.0f)
    chain = new ReverbEffect(chain, 1.0f + reverbLevel, reverbLevel * 0.7f);

  if (distortionLevel > 0.0f)
    chain = new DistortionEffect(chain, 1.0f + distortionLevel * 10.0f);

  // Volume is always last
  chain = new VolumeEffect(chain, volume);

  currentNode = chain;
}

bool AudioEngine::play(AudioNode *node) {
  if (!initialized || !node)
    return false;
  stop();

  rawSource = node;
  rebuildEffectChain();

  playing = true;
  paused = false;

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
  cleanupEffectChain();
  rawSource = nullptr;
}

void AudioEngine::setVolume(float vol) {
  if (vol < 0.0f)
    vol = 0.0f;
  if (vol > 1.0f)
    vol = 1.0f;
  volume = vol;

  // Find the VolumeEffect in the chain and update it
  AudioNode *node = currentNode;
  while (node) {
    VolumeEffect *ve = dynamic_cast<VolumeEffect *>(node);
    if (ve) {
      ve->setGain(volume);
      return;
    }
    AudioEffect *ef = dynamic_cast<AudioEffect *>(node);
    if (ef)
      node = ef->getSource();
    else
      break;
  }
}

float AudioEngine::getVolume() const { return volume; }

void AudioEngine::setEcho(float level) {
  echoLevel = level;
  if (playing && rawSource)
    rebuildEffectChain();
}

void AudioEngine::setReverb(float level) {
  reverbLevel = level;
  if (playing && rawSource)
    rebuildEffectChain();
}

void AudioEngine::setBassBoost(float level) {
  bassLevel = level;
  if (playing && rawSource)
    rebuildEffectChain();
}

void AudioEngine::setDistortion(float level) {
  distortionLevel = level;
  if (playing && rawSource)
    rebuildEffectChain();
}

void AudioEngine::setSpeed(float factor) {
  if (factor < 0.25f)
    factor = 0.25f;
  if (factor > 4.0f)
    factor = 4.0f;
  speedFactor = factor;
  if (playing && rawSource)
    rebuildEffectChain();
}

float AudioEngine::getEcho() const { return echoLevel; }
float AudioEngine::getReverb() const { return reverbLevel; }
float AudioEngine::getBassBoost() const { return bassLevel; }
float AudioEngine::getDistortion() const { return distortionLevel; }
float AudioEngine::getSpeed() const { return speedFactor; }

void AudioEngine::resetEffectParams() {
  echoLevel = 0.0f;
  reverbLevel = 0.0f;
  bassLevel = 0.0f;
  distortionLevel = 0.0f;
  speedFactor = 1.0f;
}

void AudioEngine::clearEffects() {
  resetEffectParams();
  if (playing && rawSource)
    rebuildEffectChain();
}

bool AudioEngine::isPlaying() const { return playing; }
bool AudioEngine::isPaused() const { return paused; }
bool AudioEngine::isInitialized() const { return initialized; }

int AudioEngine::getBufferFrames() const { return bufferFrames; }

const float *AudioEngine::getLastBuffer() const {
  int idx = (currentBuffer == 0) ? 1 : 0;
  if (renderBuffers[idx])
    return renderBuffers[idx]->raw();
  return nullptr;
}

void AudioEngine::prepareBuffer(int bufferIndex) {
  AudioBuffer *renderBuf = renderBuffers[bufferIndex];
  renderBuf->clear();

  int framesRead = 0;
  if (currentNode && playing)
    framesRead = currentNode->process(renderBuf->raw(), bufferFrames);

  if (framesRead == 0 && playing)
    playing = false;

  renderBuf->clamp();
  convertFloatToInt16(renderBuf->raw(), outputBuffers[bufferIndex],
                      bufferFrames * numChannels);
}

void AudioEngine::convertFloatToInt16(const float *src, short *dst,
                                      int numSamples) {
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

  WAVEHDR *completedHeader = reinterpret_cast<WAVEHDR *>(dwParam1);
  int bufIdx = (completedHeader == &engine->waveHeaders[0]) ? 0 : 1;

  engine->prepareBuffer(bufIdx);
  waveOutWrite(engine->hWaveOut, &engine->waveHeaders[bufIdx], sizeof(WAVEHDR));
}

void AudioEngine::renderBlock() {
  if (!playing || paused)
    return;
  prepareBuffer(currentBuffer);
  waveOutWrite(hWaveOut, &waveHeaders[currentBuffer], sizeof(WAVEHDR));
  currentBuffer = (currentBuffer + 1) % 2;
}
