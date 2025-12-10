# Professional Music Player

A truly cross-platform, professional music player with microkernel architecture and extensible plugin ecosystem, inspired by foobar2000's design philosophy.

## 🌟 Features

### Cross-Platform Support
- **Windows**: WASAPI audio backend
- **macOS**: CoreAudio backend (ready)
- **Linux**: ALSA/PulseAudio backend with graceful fallback
- **Automatic Detection**: Runtime platform and dependency detection
- **Graceful Degradation**: Works even without audio libraries (stub mode)

### Architecture
**Microkernel Design**: Minimal core providing essential services with maximum extensibility through plugins:
- **Service Registry**: Central service discovery and dependency injection
- **Event Bus**: Asynchronous pub/sub communication between components
- **Plugin Host**: Hot-loading and lifecycle management
- **Audio Pipeline**: Abstracted audio processing chain
- **Platform Abstraction**: Cross-platform audio output with automatic backend selection

### Current Status: v0.3.0 (Cross-Platform Enhanced)

#### ✅ **Implemented Components**
- **Core Engine**: Complete microkernel architecture
- **Cross-Platform Layer**: Automatic platform detection (Windows/macOS/Linux)
- **Audio Abstraction**: Unified audio API with multiple backend support
- **Dependency Detection**: Runtime checking of required libraries
- **Service Registry**: Dependency injection and service discovery
- **Event Bus**: High-performance async messaging
- **Plugin System**: Dynamic loading with ABI stability
- **Configuration Manager**: JSON-based with auto-save and schema versioning
- **Playback Engine**: Gapless-capable audio pipeline
- **Playlist Management**: Multiple playlist support with metadata
- **Visualization Engine**: Real-time audio visualization
- **Fallback System**: Stub implementations for missing dependencies

#### 🔧 **Supported Formats**
- **WAV**: Native support (16/24/32-bit, various sample rates)
- **MP3**: Support via minimp3 (header-only, no dependencies)
- **FLAC**: Support available when libflac-dev is installed
- **Platform-specific**: Automatic format conversion (16-bit → 32-bit float)

#### 🎯 **Platform-Specific Features**
- **Windows**:
  - WASAPI low-latency audio
  - Automatic COM management
  - Visual Studio integration
- **macOS**:
  - CoreAudio integration (framework ready)
  - AudioUnit support (planned)
  - Xcode compatibility
- **Linux**:
  - ALSA support with graceful fallback
  - PulseAudio compatibility (planned)
  - GCC/Clang optimization

## Quick Start

### Prerequisites

**Required:**
- **CMake 3.20+** - Build system
- **C++17 Compatible Compiler** - Modern C++ features
- **Git** - Version control

**Optional (for full features):**
- **Linux**:
  - `libasound2-dev` - ALSA audio support
  - `libflac-dev` - FLAC format support
  - `libopenmp-dev` - Parallel processing
- **macOS**: Xcode command line tools
- **Windows**: Visual Studio 2017+ (automatically configured)

### Build Instructions

```bash
# Clone repository
git clone <repository-url>
cd Qoder_foobar

# Configure build (automatically detects platform and dependencies)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build all components
cmake --build build -j4

# Run tests
./build/bin/test_cross_platform  # Check platform and dependencies
./build/bin/test_audio_direct    # Test audio backend
```

### Install Dependencies (Linux)

For the best experience on Linux, install audio libraries:

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y libasound2-dev libflac-dev libopenmp-dev

# CentOS/RHEL/Fedora
sudo dnf install -y alsa-lib-devel flac-devel libgomp-devel

# Arch Linux
sudo pacman -S alsa-lib flac openmp
```

See [docs/INSTALL_LINUX.md](docs/INSTALL_LINUX.md) for detailed installation instructions.

## Usage

### Command Line Interface

```bash
# Test platform and dependencies
./build/bin/test_cross_platform

# List available audio backends
./build/bin/music-player --list-backends

# Play audio file (automatically detects format)
./build/bin/music-player audio.wav
./build/bin/music-player music.flac
./build/bin/music-player song.mp3

# Test audio playback only
./build/bin/test_audio_direct

# Play with specific backend
./build/bin/music-player --backend alsa file.wav
./build/bin/music-player --backend stub file.wav
```

### Plugin System

The player uses a **pure plugin architecture**:

```
File → Decoder Plugin → DSP Plugins → Audio Output Plugin → Speakers
```

**Available Decoders:**
- ✅ WAV (native)
- ✅ MP3 (minimp3, no external deps)
- ✅ FLAC (libflac, optional)

## Development

### Cross-Platform Development

The project provides comprehensive cross-platform support:

```cpp
// Platform detection is automatic
#include "core/platform_utils.h"

if (MP_IS_WINDOWS()) {
    // Windows-specific code
} else if (MP_IS_LINUX()) {
    // Linux-specific code
} else if (MP_IS_MACOS()) {
    // macOS-specific code
}
```

### Architecture Overview

```
┌─────────────────────────────────────────┐
│              Core Engine                │
│  ┌─────────────────────────────────────┐ │
│  │        Service Registry             │ │
│  │          Event Bus                  │ │
│  │        Plugin Host                  │ │
│  │      Configuration Manager           │ │
│  └─────────────────────────────────────┘ │
└─────────────────────────────────────────┘
                   │
        ┌──────────┴──────────┐
        │    Plugin System     │
        └──────────┬──────────┘
    ┌─────────────┼─────────────┐
    │             │             │
┌───▼───┐ ┌─────▼─────┐ ┌───▼───┐
│Decoder│ │    DSP    │ │Output │
│Plugins│ │  Plugins  │ │Plugins│
└───────┘ └───────────┘ └───────┘
    │             │             │
    └──────┬──────┘             │
           ▼                     ▼
    [Audio Processing]    [Platform Audio]
                         Windows(WASAPI)
                         macOS(CoreAudio)
                         Linux(ALSA/Pulse)
```

### Testing

The project includes comprehensive testing:

```bash
# Cross-platform detection test
./build/bin/test_cross_platform

# Shows:
# - Platform: Linux x64 (GCC)
# - Audio Backend: ALSA/Stub
# - Dependencies: Available/Not Available
# - Backend Testing: auto/stub/alsa

# WAV format test
./build/bin/final_wav_player test.wav

# Shows:
# - WAV parsing
# - Format conversion (16→32-bit)
# - Audio data processing
```

## Troubleshooting

### Common Issues

1. **"ALSA not found"** on Linux
   ```bash
   # Install ALSA development libraries
   sudo apt-get install libasound2-dev  # Ubuntu/Debian
   # Reconfigure and rebuild
   rm -rf build && cmake -B build && cmake --build build
   ```

2. **Audio not playing**
   - Check if user is in audio group: `groups $USER`
   - Try stub backend: `--backend stub`
   - Check permissions: `sudo usermod -a -G audio $USER`

3. **Compilation errors**
   - Ensure GCC 7+ or Clang 5+: `gcc --version`
   - Check CMake 3.20+: `cmake --version`

## Documentation

- [Installation Guide](docs/INSTALL_LINUX.md) - Detailed Linux installation
- [Audio Pipeline Specification](docs/AUDIO_PIPELINE.md) - Complete audio subsystem design
- [Plugin Development Guide](docs/PLUGIN_DEVELOPMENT_GUIDE.md) - How to create plugins
- [Implementation Progress](IMPLEMENTATION_PROGRESS.md) - Current development status

## Platform-Specific Notes

### Windows
- Automatic WASAPI backend selection
- Visual Studio integration
- DLL export support for plugins

### macOS
- CoreAudio framework support (ready)
- AudioUnit integration planned
- Xcode project support planned

### Linux
- ALSA support with automatic detection
- Graceful fallback to stub mode
- PulseAudio support planned
- Package manager integration (apt/dnf/pacman)

## Contributing

1. Fork the repository
2. Create a feature branch
3. Test on your target platform
4. Ensure cross-platform compatibility
5. Submit a pull request

## License

[License information to be added]

---

**Note**: This project demonstrates professional cross-platform C++ development with automatic platform detection, graceful dependency handling, and modular architecture suitable for production use.