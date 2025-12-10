# foobar2000 Plugin Integration Guide

## Overview

Qoder foobar now supports loading and using foobar2000 plugins directly, providing compatibility with a vast ecosystem of audio processing components. This integration allows the music player to handle virtually any audio format supported by foobar2000 plugins.

## Features

### ✅ Implemented Features

1. **Plugin Loading System**
   - Dynamic loading of foobar2000 DLL files (Windows) and shared libraries (Linux)
   - Automatic service discovery and registration
   - Plugin metadata extraction

2. **Audio Format Support**
   - WAV, FLAC, MP3, OGG, AAC, M4A, WMA, APE, and more
   - Automatic decoder selection based on file extension
   - Metadata extraction (artist, title, album, etc.)

3. **Sample Rate Conversion**
   - Integration with existing high-quality resamplers
   - Automatic conversion to output sample rate
   - Quality selection (Fast, Good, High, Best)

4. **Cross-Platform Compatibility**
   - Windows: Direct DLL loading with LoadLibrary
   - Linux: Shared library loading with dlopen
   - macOS: Dynamic library support (planned)

## Architecture

### Core Components

```
┌─────────────────────────────────────────────────┐
│            PluginAudioDecoder                   │
│  - Loads plugins via FoobarPluginLoader         │
│  - Manages decoder instances                   │
│  - Handles format conversion                   │
└─────────────────┬───────────────────────────────┘
                  │
┌─────────────────▼───────────────────────────────┐
│            FoobarPluginLoader                   │
│  - Cross-platform dynamic loading              │
│  - Service registry bridging                    │
│  - Plugin lifecycle management                 │
└─────────────────┬───────────────────────────────┘
                  │
┌─────────────────▼───────────────────────────────┐
│           ServiceRegistryBridge                 │
│  - Maps foobar2000 GUIDs to service IDs        │
│  - Manages service instances                    │
│  - Provides query interface                     │
└─────────────────────────────────────────────────┘
```

### Plugin Discovery

1. Plugin files are scanned for foobar2000 export symbols
2. Service factories are extracted and registered
3. Input decoders are mapped to file extensions
4. Services are made available through the bridge

## Usage

### Command Line Interface

```bash
# Basic playback with plugins
./music-player-plugin song.mp3

# Load plugins from custom directory
./music-player-plugin -p /path/to/plugins song.flac

# Set output quality and sample rate
./music-player-plugin -q high -r 96000 song.wav

# Use adaptive resampling
./music-player-plugin -a song.ogg

# List supported formats
./music-player-plugin -l

# Show plugin information
./music-player-plugin -i
```

### Programming Interface

```cpp
#include "audio/plugin_audio_decoder.h"

// Create decoder with plugin support
auto decoder = PluginAudioDecoderFactory::create(
    "./plugins",  // Plugin directory
    44100         // Target sample rate
);

// Open file
if (decoder->open_file("music.flac")) {
    // Get audio info
    auto info = decoder->get_audio_info();
    std::cout << "Format: " << info.format_name << "\n";
    std::cout << "Duration: " << info.duration_seconds << "s\n";

    // Decode audio
    std::vector<float> buffer(4096);
    while (true) {
        int frames = decoder->decode_frames(buffer.data(), 1024);
        if (frames == 0) break;  // End of file

        // Process audio...
    }
}
```

## Plugin Compatibility

### Supported Plugin Types

1. **Input Decoders**
   - Audio format decoders (MP3, FLAC, etc.)
   - Container format support (OGG, M4A)
   - Lossless and lossy codecs

2. **Output Components** (Planned)
   - Audio output drivers
   - DSP effects
   - Visualization components

### Known Working Plugins

The following foobar2000 plugins have been tested:

| Plugin | Format | Status | Notes |
|--------|--------|--------|-------|
| foo_input_mp3 | MP3 | ✅ Working | MP3 decoding |
| foo_input_flac | FLAC | ✅ Working | Lossless decoding |
| foo_input_std | WAV | ✅ Working | Standard formats |
| foo_input_vorbis | OGG | ✅ Working | Vorbis decoding |
| foo_input_mp4 | M4A/AAC | ✅ Working | AAC/M4A support |

### Plugin Requirements

- Plugins must be foobar2000 v0.9 or v1.0 compatible
- Export standard foobar2000 service functions
- Follow foobar2000 SDK conventions
- No UI components (input-only for now)

## Building

### Prerequisites

- CMake 3.20 or later
- C++17 compatible compiler
- Platform-specific audio libraries:
  - Linux: ALSA development packages
  - Windows: Windows SDK
  - macOS: Xcode command line tools

### Build Commands

```bash
# Configure
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --target music-player-plugin

# Run tests
./bin/music-player-plugin -l
```

## Configuration

### Environment Variables

- `FOOBAR_PLUGIN_PATH`: Default plugin directory
- `MP_AUDIO_BACKEND`: Audio backend selection (alsa/pulse/wasapi/coreaudio)

### Configuration File (Future)

A configuration file will be supported in future versions:

```ini
[plugins]
directory = /usr/lib/foobar2000/components
auto_load = true

[audio]
output_rate = 96000
resampler_quality = high
backend = alsa
```

## Troubleshooting

### Common Issues

1. **Plugin fails to load**
   - Check if plugin is compatible with foobar2000 v1.0
   - Verify plugin dependencies are installed
   - Use `LD_LIBRARY_PATH` on Linux for dependency paths

2. **No sound output**
   - Verify audio backend is working
   - Check output sample rate settings
   - Ensure audio device is available

3. **Crash on specific file**
   - File may be corrupted
   - Plugin might have issues with specific encoding
   - Try with different quality settings

### Debug Information

Enable verbose logging:

```bash
# Set debug level (if implemented)
export MP_LOG_LEVEL=debug
./music-player-plugin song.mp3
```

## Development

### Adding New Plugin Support

To add support for a new plugin type:

1. Update `plugin_audio_decoder.cpp` to recognize new formats
2. Add GUID definitions to `foobar2000.cpp`
3. Implement adapter if needed
4. Test with actual plugin file

### Extending the Bridge

The `ServiceRegistryBridge` can be extended to support:

- DSP effects processing
- Output components
- Visualization
- Metadata services

## Roadmap

### Near Future (v0.4)
- [ ] DSP effects support
- [ ] Playlist loading (FPL format)
- [ ] Configuration file support
- [ ] macOS plugin loading

### Future (v0.5+)
- [ ] UI extension support
- [ ] Real-time visualization
- [ ] Plugin auto-update
- [ ] Plugin store integration

## Contributing

Contributions are welcome! Please:

1. Test with real foobar2000 plugins
2. Report compatibility issues
3. Submit pull requests for new features
4. Provide plugin files for testing

## License

This integration maintains the same license as the main project. foobar2000 plugins remain under their respective licenses.

---

*Last Updated: 2025-12-10*
*Version: 0.3.0*