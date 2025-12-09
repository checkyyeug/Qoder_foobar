# Professional Music Player

A cross-platform, professional music player with extensible plugin architecture, inspired by foobar2000.

## Features

- **Microkernel Architecture**: Minimal core with extensive plugin ecosystem
- **Cross-Platform**: Windows, macOS, and Linux support
- **Foobar2000 Compatibility**: Load and use existing foobar2000 plugins
- **GPU Acceleration**: Modern GPU-accelerated UI and visualizations using Skia/Vulkan
- **Professional Audio**: High-quality audio processing with low latency
- **Extensible**: Plugin-based architecture for decoders, DSP, UI components
- **Configuration Management**: JSON-based configuration with auto-save and schema versioning
- **Gapless Playback**: Designed for seamless track transitions (implementation in progress)

## Building

### Prerequisites

- CMake 3.20 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Platform-specific dependencies:
  - **Linux**: ALSA/PulseAudio development libraries, X11/Wayland libraries
  - **macOS**: Xcode command line tools
  - **Windows**: Visual Studio 2017 or higher

### Linux Build Instructions

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install build-essential cmake libasound2-dev libpulse-dev \
    libx11-dev libxrandr-dev libxi-dev libxcursor-dev libxinerama-dev \
    libjsoncpp-dev

# Clone and build
git clone <repository-url>
cd opencode_foobar
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run
./bin/music-player
```

### Build Options

- `BUILD_TESTS=ON/OFF` - Build unit tests (default: ON)
- `BUILD_PLUGINS=ON/OFF` - Build bundled plugins (default: ON)
- `ENABLE_FOOBAR_COMPAT=ON/OFF` - Enable foobar2000 compatibility (default: ON)
- `ENABLE_GPU=ON/OFF` - Enable GPU acceleration (default: ON)

Example:
```bash
cmake -DBUILD_TESTS=OFF -DENABLE_GPU=ON ..
```

## Architecture

The application follows a microkernel architecture:

- **Core Engine**: Minimal core providing essential services
- **Plugin Host**: Dynamic plugin loading and management
- **Platform Abstraction**: Cross-platform compatibility layer
- **Event Bus**: Inter-component communication
- **Audio Pipeline**: Format-agnostic audio processing (see [Audio Pipeline Spec](docs/AUDIO_PIPELINE.md))
- **Configuration Manager**: JSON-based persistent configuration with auto-save

See [Design Document](.qoder/quests/music-player-development.md) for detailed architecture information.

## Plugin Development

See the [Plugin Development Guide](docs/PLUGIN_DEVELOPMENT_GUIDE.md) for comprehensive plugin development tutorials and examples.

**Quick Start**:
1. Review the [Audio Pipeline Specification](docs/AUDIO_PIPELINE.md)
2. Follow the step-by-step guide to create your first plugin
3. Check example plugins in `plugins/` directory

## Documentation

- [Audio Pipeline Specification](docs/AUDIO_PIPELINE.md) - Complete audio subsystem design
- [Plugin Development Guide](docs/PLUGIN_DEVELOPMENT_GUIDE.md) - How to create plugins
- [Implementation Progress](IMPLEMENTATION_PROGRESS.md) - Current development status
- [Implementation Summary](IMPLEMENTATION_SUMMARY.md) - Phase 1 completion details

## License

[License information to be added]

## Contributing

[Contribution guidelines to be added]
