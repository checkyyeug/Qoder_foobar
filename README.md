# Professional Music Player

A cross-platform, professional music player with microkernel architecture and extensible plugin ecosystem, inspired by foobar2000's design philosophy.

## Architecture

**Microkernel Design**: Minimal core providing essential services with maximum extensibility through plugins:
- **Service Registry**: Central service discovery and dependency injection
- **Event Bus**: Asynchronous pub/sub communication between components
- **Plugin Host**: Hot-loading and lifecycle management
- **Audio Pipeline**: Abstracted audio processing chain
- **Platform Abstraction**: Cross-platform audio output

## Current Status: v0.2.0 (Architecture-First)

### ✅ **Implemented Core Components**
- **Core Engine**: Complete microkernel architecture
- **Service Registry**: Dependency injection and service discovery
- **Event Bus**: High-performance async messaging
- **Plugin System**: Dynamic loading with ABI stability
- **Configuration Manager**: JSON-based with auto-save and schema versioning
- **Playback Engine**: Gapless-capable audio pipeline
- **Playlist Management**: Multiple playlist support with metadata
- **Visualization Engine**: Real-time audio visualization
- **Platform Abstraction**: Windows WASAPI + Linux ALSA

### 🔧 **Plugin Ecosystem**
- **Audio Decoders**: WAV, FLAC, MP3 (modular)
- **DSP Processors**: Volume control, equalizer (extensible)
- **Audio Output**: Platform-optimized backends

### 🎯 **foobar2000 Compatibility**
- **Adapter Layer**: Translates foobar2000 plugin APIs to native interfaces
- **Data Migration**: Import playlists, configuration, and metadata
- **SDK Implementation**: Complete foobar2000 API compatibility layer

### ⏳ **In Development**
- **GPU-Accelerated UI**: Modern rendering using Skia/Vulkan
- **Advanced Features**: Scripting, advanced visualizations
- **Extended Compatibility**: More foobar2000 plugin types

## Building

### Prerequisites

- **CMake 3.20+** - Build system
- **C++17 Compatible Compiler** - Modern C++ features
- **Platform Libraries**:
  - **Linux**: ALSA development libraries
  - **Windows**: Visual Studio 2017+ (automatically handled)
  - **macOS**: Xcode command line tools

### Quick Build

```bash
# Clone repository
git clone <repository-url>
cd Qoder_foobar

# Configure build
mkdir build && cd build
cmake ..

# Build all components
cmake --build . --config Release

# Run with audio test
./music-player --test

# Play audio file
./music-player your-audio-file.wav
```

### Build Options

```cmake
# Enable/disable components
cmake -DBUILD_TESTS=ON \          # Build unit tests
      -DBUILD_PLUGINS=ON \        # Build plugin ecosystem
      -DENABLE_FOOBAR_COMPAT=ON \  # foobar2000 compatibility
      -DENABLE_GPU=OFF \           # GPU acceleration (future)
      -DENABLE_UI=OFF \            # UI components (future)
      ..
```

### Windows Specific

```powershell
# Visual Studio 2017+ required
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release

# Run
.\bin\Release\music-player.exe --test
.\bin\Release\music-player.exe audio.wav
```

## Usage

### Command Line Interface

```bash
# Test audio output
music-player --test

# List available plugins
music-player --list-plugins

# List audio devices
music-player --list-devices

# Play audio file using plugin system
music-player /path/to/audio.wav

# Show help
music-player --help
```

### Plugin System

The player uses a **pure plugin architecture**:

```bash
# Audio is processed through:
File → Decoder Plugin → DSP Plugins → Audio Output Plugin → Speakers
```

**Supported Formats** (via plugins):
- ✅ WAV (native)
- ✅ FLAC (libFLAC)
- ✅ MP3 (libmp3lame)
- ✅ Any format via custom plugins

## Development

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
```

### Plugin Development

**Creating a Decoder Plugin**:

```cpp
// Simple decoder implementation
class MyDecoder : public mp::IDecoder {
public:
    Result initialize() override {
        // Initialize decoder
        return mp::Result::Success;
    }

    Result decode(DecoderHandle handle, void* buffer, size_t size) override {
        // Decode audio data
        return mp::Result::Success;
    }

    // ... other required methods
};
```

See [Plugin Development Guide](docs/PLUGIN_DEVELOPMENT_GUIDE.md) for complete examples.

### foobar2000 Compatibility

**Adapter Pattern** enables foobar2000 plugin loading:

```cpp
// foobar2000 plugins are wrapped by adapters
FoobarDecoderAdapter {
    mp::IDecoder* native_decoder;
    foobar::input_decoder* foobar_decoder;

    // Translate API calls between systems
    Result decode(void* buffer, size_t size) override {
        foobar_result = foobar_decoder->decode(foobar_buffer);
        return translate_foobar_result(foobar_result);
    }
};
```

**Current Compatibility**:
- ✅ Input decoder plugins (adapters implemented)
- ✅ Data migration (playlists, configuration)
- ⏳ DSP plugins (in development)
- ⏳ UI extensions (planned)

## Documentation

- [Audio Pipeline Specification](docs/AUDIO_PIPELINE.md) - Complete audio subsystem design
- [Plugin Development Guide](docs/PLUGIN_DEVELOPMENT_GUIDE.md) - How to create plugins
- [Implementation Progress](IMPLEMENTATION_PROGRESS.md) - Current development status
- [Implementation Summary](IMPLEMENTATION_SUMMARY.md) - Phase 1 completion details

## License

[License information to be added]

## Contributing

[Contribution guidelines to be added]
