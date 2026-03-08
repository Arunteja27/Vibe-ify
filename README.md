# 🎵 Vibe-ify

A low-latency C++ music engine and CLI music player built from scratch. Plays real WAV audio through the Windows waveOut API with double-buffered output for gapless playback.

## Features

- **WAV Playback** — Loads and plays 8/16/32-bit PCM WAV files in real time
- **Audio Pipeline** — Modular node graph: `Track → VolumeEffect → AudioEngine`
- **Playlist Queue** — Queue multiple tracks with auto-advance and gapless transitions
- **Audio Effects** — Volume control and fade-in/fade-out effects (in-place DSP)
- **Search** — Case-insensitive search by artist, title, or genre
- **Manual Memory Management** — All data structures use raw pointers and `new`/`delete`
- **Double-Buffered Output** — Callback-driven buffer swapping for low-latency playback

## Prerequisites

- **Windows** (uses `waveOut` multimedia API)
- **MinGW g++** with C++17 support
- **GNU Make** (`mingw32-make`)

## Building

```powershell
mingw32-make clean
mingw32-make
```

## Running

1. Place `.wav` files in `media/samples/` using the naming format:
   ```
   Artist - Title.wav
   ```

2. Run the player:
   ```powershell
   .\vibeify.exe
   ```

## CLI Commands

| Command | Description |
|---------|-------------|
| `list` | Show all tracks in library |
| `play <id>` | Play track by ID |
| `pause` | Pause playback |
| `resume` | Resume playback |
| `stop` | Stop playback |
| `skip` | Skip to next track |
| `prev` | Go to previous track |
| `queue <id>` | Add track to playlist queue |
| `search <query>` | Search by artist or title |
| `volume <0-100>` | Set playback volume |
| `now` | Show now playing info |
| `help` | Show available commands |
| `quit` | Exit |

## Architecture

```
AudioNode (abstract)
├── Track          — Loads WAV, outputs float PCM
├── Playlist       — Queue of Track*, auto-advance
└── AudioEffect    — Wraps a source node
    ├── VolumeEffect
    └── FadeEffect

AudioEngine        — waveOut double-buffered output
AudioBuffer        — Raw float* buffer with mix/scale/clamp
TrackLibrary       — Owns all Track objects (raw pointer array)
PlaybackController — CLI interface
WavGenerator       — Synthetic WAV file generator (testing)
```

## File Structure

```
├── main.cc                 — Entry point
├── AudioNode.h             — Abstract base class
├── AudioBuffer.h           — PCM buffer (header-only)
├── Track.h / Track.cc      — WAV loader + PCM player
├── Playlist.h / Playlist.cc — Track queue
├── TrackLibrary.h / .cc    — Track collection + directory loader
├── AudioEffect.h / .cc     — Volume and fade effects
├── AudioEngine.h / .cc     — waveOut playback engine
├── PlaybackController.h/.cc — CLI
├── WavGenerator.h          — Sine/chord WAV generator (header-only)
├── Makefile                — Build config
└── media/samples/          — WAV files go here
```
