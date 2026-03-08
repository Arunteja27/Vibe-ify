# Vibe-ify Music Engine (Spotify Backend Clone)

A **low-latency C++ music engine** that emulates the backend audio pipeline of streaming platforms like Spotify. Built to showcase **polymorphism**, **dynamic memory management**, and **real-time audio processing** using raw pointers and manual memory control.

## Architecture

```
                     AudioNode (abstract base)
                    /         |           \
                 Track     Playlist    AudioEffect
                (owns PCM)  (queue of    /        \
                              Track*)  VolumeEffect  FadeEffect

 TrackLibrary ──owns──▶ Track* array (raw pointers, manual new/delete)
      │
      ▼
 PlaybackController ──uses──▶ AudioEngine ──uses──▶ AudioNode* (polymorphic)
      │                           │
      │                    waveOut* (winmm.lib)
      ▼                           │
 Terminal CLI                 Speaker Output
```

### Key Design Decisions

- **Polymorphic audio pipeline**: Abstract `AudioNode` base class with pure virtual `process()`. `Track`, `Playlist`, and `AudioEffect` all derive from it, enabling the `AudioEngine` to process any node type through a single interface.
- **Raw pointer memory management**: `AudioBuffer` uses `float*` with `new[]`/`delete[]`. `TrackLibrary` owns `Track**` arrays with manual grow/resize. No smart pointers — mirrors hardware-level memory constraints.
- **Real-time playback**: Windows `waveOut*` API with double-buffered output for gapless audio. The `AudioEngine` pulls PCM data from the polymorphic `AudioNode*` chain.
- **In-place buffer processing**: `VolumeEffect` and `FadeEffect` modify PCM buffers in-place via pointer arithmetic to minimize CPU cycles and memory allocations.

## Source Files

| File | Description |
|------|-------------|
| `AudioNode.h` | Abstract base class — polymorphic interface for the audio pipeline |
| `AudioBuffer.h` | Raw pointer PCM buffer with manual memory management |
| `Track.h/cc` | Concrete AudioNode — loads WAV files, owns `float*` PCM data |
| `AudioEffect.h/cc` | VolumeEffect and FadeEffect — polymorphic audio processors |
| `Playlist.h/cc` | Derived AudioNode — manages a queue of `Track*` references |
| `TrackLibrary.h/cc` | Owning container of `Track*` — raw pointer array with manual resize |
| `AudioEngine.h/cc` | Real-time playback engine using Windows `waveOut*` API |
| `PlaybackController.h/cc` | Terminal CLI — connects library, playlist, and engine |
| `WavGenerator.h` | Utility to generate synthetic WAV files for demos |
| `main.cc` | Entry point — initializes components and launches the CLI |

## Building

Requires **g++ (MinGW)** with C++17 support on Windows:

```bash
make clean && make
```

This produces the `vibeify` executable.

## Usage

```bash
./vibeify
```

### Commands

| Command | Description |
|---------|-------------|
| `list` | Show all tracks in library |
| `play <id>` | Play a track by its ID |
| `pause` | Pause playback |
| `resume` | Resume playback |
| `stop` | Stop playback |
| `skip` | Skip to the next track in queue |
| `prev` | Go back to the previous track |
| `queue <id>` | Add a track to the playback queue |
| `search <query>` | Search tracks by artist or title |
| `volume <0-100>` | Set playback volume |
| `now` | Show currently playing track info |
| `help` | Display all commands |
| `quit` | Exit Vibe-ify |

## Technologies

- **C++17** with STL (string, fstream, iostream)
- **Object-Oriented Design** — polymorphism, inheritance, encapsulation
- **Raw Pointers & Manual Memory Management** — new/delete, pointer arithmetic
- **Windows Multimedia API** (winmm.lib) — waveOut for real-time audio output
- **Git** for version control
