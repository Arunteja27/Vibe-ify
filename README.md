# 🎵 Vibe-ify

A low-latency C++ music engine with real-time DSP effects, FFT spectrum analysis, YouTube integration, and a REST API. All built from scratch with raw pointers, manual memory management, and zero external libraries.

See demo of the engine features served via the REST API, showcased on a simple `testUI.html` file (UNMUTE video below!). 

https://github.com/user-attachments/assets/f96565d5-ffeb-4c73-bea1-63628c55396b



## Features

### 🎛️ Real-Time DSP Effects
Five chainable audio effects processed in real-time on the PCM buffer:

| Effect | Command | Range | Description |
|--------|---------|-------|-------------|
| Echo | `echo <0-100>` | Delay-based echo with feedback | Configurable delay line |
| Reverb | `reverb <0-100>` | Multi-tap room simulation | Simulates acoustic space |
| Bass Boost | `bass <0-100>` | Low-frequency amplification | Single-pole IIR filter |
| Distortion | `distort <0-100>` | Hard/soft clipping | Waveshaper with tanh curve |
| Speed | `speed <25-400>` | Playback speed (%) | Linear interpolation resampler |

Effects are applied in a dynamic chain: **Source → Speed → Bass → Echo → Reverb → Distortion → Volume**

### 📊 FFT Spectrum Analyzer
- Cooley-Tukey radix-2 FFT (1024-point)
- Hann window to reduce spectral leakage
- 64 frequency bands with logarithmic distribution
- Real-time analysis on every audio buffer
- ASCII visualizer in CLI, JSON array via API

### ▶️ YouTube Integration
Search and play music from YouTube directly:
- Powered by `yt-dlp` + `ffmpeg` (auto-detected at startup)
- Downloads as 44.1kHz stereo 16-bit WAV
- File caching in `media/cache/` (skips re-download)
- Auto-saves to `media/samples/` for persistent library

### 🌐 REST API Server
Raw Winsock2 HTTP server on `localhost:8080`, running alongside the CLI on a separate thread. Full CORS support for browser frontends.

#### GET Endpoints
| Endpoint | Description |
|----------|-------------|
| `/api/tracks` | List all loaded tracks |
| `/api/now` | Current playback status |
| `/api/spectrum` | 64-band frequency data (JSON array) |
| `/api/effects` | Current effect levels |

#### POST Endpoints
| Endpoint | Body | Description |
|----------|------|-------------|
| `/api/play` | `{"id": 1}` | Play track by ID |
| `/api/pause` | — | Pause playback |
| `/api/resume` | — | Resume playback |
| `/api/stop` | — | Stop playback |
| `/api/volume` | `{"level": 75}` | Set volume (0-100) |
| `/api/effects` | `{"bass": 50, "echo": 30}` | Set effect levels |
| `/api/effects` | `{"clear": "true"}` | Reset all effects |
| `/api/yt/search` | `{"query": "song name"}` | Search YouTube |
| `/api/yt/play` | `{"index": 1}` | Play from search results |
| `/api/search` | `{"query": "artist"}` | Search local library |

## Architecture

```
┌──────────────────────────────────────────────┐
│                  main.cc                     │
│     CLI (PlaybackController) + HTTP Server   │
├───────────────┬──────────────────────────────┤
│   Terminal    │     REST API (port 8080)     │
│   Commands    │     ApiController            │
├───────────────┴──────────────────────────────┤
│                AudioEngine                   │
│  ┌─────────────────────────────────────────┐ │
│  │  Effect Chain (dynamic rebuild)         │ │
│  │  Source → Speed → Bass → Echo →         │ │
│  │  Reverb → Distortion → Volume           │ │
│  ├─────────────────────────────────────────┤ │
│  │  SpectrumAnalyzer (FFT on each buffer)  │ │
│  ├─────────────────────────────────────────┤ │
│  │  Windows WaveOut (double-buffered PCM)  │ │
│  └─────────────────────────────────────────┘ │
├──────────────────────────────────────────────┤
│  TrackLibrary ← WAV files (media/samples/)   │
│  YouTubeSource ← yt-dlp (media/cache/)       │
│  StreamCache ← persistent download manager   │
└──────────────────────────────────────────────┘
```

## Build

**Requirements:** MinGW g++ (C++17), Windows

```bash
mingw32-make clean
mingw32-make
```

**Optional (for YouTube):**
```bash
winget install yt-dlp
winget install ffmpeg
```

## Usage

```bash
.\vibeify.exe
```

The engine starts both the CLI and the API server on port 8080.

### CLI Commands

```
Playback:     play <id>, pause, resume, stop, skip, prev
Queue:        queue <id>, list, now, search <query>
Effects:      echo <0-100>, reverb <0-100>, bass <0-100>
              distort <0-100>, speed <25-400>
              effects, clear, spectrum
YouTube:      yt-search <query>, yt-play <1-5>
General:      help, quit
```

### API Examples

```bash
# List tracks
curl http://localhost:8080/api/tracks

# Play track 1
curl -X POST http://localhost:8080/api/play -d "{\"id\": 1}"

# Bass boost + echo
curl -X POST http://localhost:8080/api/effects -d "{\"bass\": 60, \"echo\": 40}"

# Get spectrum data
curl http://localhost:8080/api/spectrum

# Search YouTube
curl -X POST http://localhost:8080/api/yt/search -d "{\"query\": \"lofi beats\"}"

# Play first result
curl -X POST http://localhost:8080/api/yt/play -d "{\"index\": 1}"
```

## File Structure

```
├── main.cc                 Entry point, starts CLI + API server
├── AudioEngine.h/.cc       Core audio renderer (WaveOut, effect chain, FFT)
├── AudioNode.h             Base class for all audio sources
├── AudioEffect.h/.cc       DSP effects (Echo, Reverb, Bass, Distortion, Speed)
├── AudioBuffer.h           Float PCM buffer with clamping
├── SpectrumAnalyzer.h/.cc  Cooley-Tukey FFT, Hann window, frequency bands
├── Track.h/.cc             WAV file loader and PCM source
├── Playlist.h/.cc          Double-linked-list track queue
├── TrackLibrary.h/.cc      Raw pointer array track collection
├── PlaybackController.h/.cc  Terminal CLI interface
├── HttpServer.h/.cc        Raw Winsock2 HTTP server (threaded)
├── ApiController.h/.cc     REST endpoint router with JSON I/O
├── YouTubeSource.h/.cc     yt-dlp wrapper (search + WAV download)
├── StreamCache.h/.cc       Download cache manager
├── WavGenerator.h          Procedural waveform generator (sine, square, saw)
├── Makefile                MinGW build system
└── media/
    ├── samples/            Persistent WAV library (auto-loaded on startup)
    └── cache/              YouTube download cache
```

## Technical Highlights

- **Zero external libraries** — raw Win32 API (WaveOut, Winsock2), manual everything
- **Raw pointer memory management** — `new`/`delete`, no smart pointers, no STL containers for core data
- **Double-buffered audio** — callback-driven WaveOut with two alternating PCM buffers
- **Dynamic effect chain** — rebuilds on parameter change, only active effects are processed
- **In-place FFT** — Cooley-Tukey butterfly with bit-reversal permutation
- **Hand-rolled HTTP** — raw TCP socket, manual HTTP parsing, JSON building
- **Dual interface** — CLI and REST API control the same engine instance simultaneously
