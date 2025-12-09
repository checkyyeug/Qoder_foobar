# Quick Start Guide

## Quick Build

```bash
# Clone and build
git clone <repository-url>
cd Qoder_foobar
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## Running the Application

### Basic Commands

```bash
# Test audio output
./bin/music-player --test

# List available audio devices
./bin/music-player --list-devices

# Play audio file (uses plugin system)
./bin/music-player your-music-file.wav

# Show version information
./bin/music-player --version

# Display help
./bin/music-player --help
```

### Advanced Usage

```bash
# With custom plugin directory
./bin/music-player --list-plugins /path/to/plugins

# Specify audio device
./bin/music-player --device-name " speakers" audio.wav
```

## Project Architecture

```
Qoder_foobar/
├── build/                          # Build output
│   ├── bin/
│   │   └── music-player           # Main executable (v0.2.0)
│   └── lib/
│       ├── libcore_engine.a       # Core engine (microkernel)
│       ├── libplatform_abstraction.a # Platform layer
│       ├── libfoobar_compat.a     # Compatibility layer
│       └── plugins/                # Plugin DLLs
│           ├── wav_decoder.dll
│           ├── flac_decoder.dll
│           ├── mp3_decoder.dll
│           ├── volume_control_dsp.dll
│           └── equalizer_dsp.dll
│
├── src/                           # Main application
│   └── main.cpp                   # Proper architecture usage
│
├── core/                          # Microkernel components
│   ├── core_engine.{h,cpp}       # Central orchestrator
│   ├── service_registry.{h,cpp}   # Dependency injection
│   ├── event_bus.{h,cpp}         # Async messaging
│   ├── plugin_host.{h,cpp}        # Plugin lifecycle
│   ├── playback_engine.{h,cpp}    # Audio pipeline
│   ├── config_manager.{h,cpp}     # Configuration system
│   ├── playlist_manager.{h,cpp}   # Playlist management
│   └── visualization_engine.{h,cpp} # Audio visualization
│
├── sdk/headers/                   # Public API
│   ├── mp_types.h                 # Core data types
│   ├── mp_plugin.h               # Plugin interface
│   ├── mp_decoder.h              # Decoder interface
│   ├── mp_audio_output.h         # Audio output
│   └── mp_event.h                # Event system
│
├── platform/                      # Platform abstraction
│   ├── audio_output_factory.cpp   # Factory pattern
│   ├── windows/
│   │   └── audio_output_wasapi.cpp # Windows WASAPI
│   └── linux/
│       └── audio_output_alsa.cpp    # Linux ALSA
│
├── plugins/                       # Plugin ecosystem
│   ├── decoders/                  # Audio format decoders
│   │   ├── wav_decoder.cpp         # WAV format
│   │   ├── flac_decoder.cpp        # FLAC format
│   │   └── mp3_decoder.cpp         # MP3 format
│   └── dsp/                       # Audio processing
│       ├── volume_control_dsp.cpp   # Volume control
│       └── equalizer_dsp.cpp       # Equalizer
│
├── compat/                        # foobar2000 compatibility
│   ├── foobar_compat_manager.cpp  # Compatibility manager
│   ├── adapters/                  # API adapters
│   ├── sdk_implementations/       # foobar SDK implementation
│   └── migration/                 # Data migration tools
│
└── docs/                          # Documentation
    ├── AUDIO_PIPELINE.md          # Audio subsystem design
    └── PLUGIN_DEVELOPMENT_GUIDE.md # Plugin development
```

## Testing the Architecture

```bash
# Verify core engine initialization
./bin/music-player --test
# Should show:
# ✓ Core engine initialized successfully
# ✓ Plugin system ready
# ✓ Service registry active
# ✓ Event bus running
```

## Plugin Development

**Create a new decoder**:

```bash
# 1. Create plugin directory
mkdir plugins/my_decoder

# 2. Implement IDecoder interface
# See docs/PLUGIN_DEVELOPMENT_GUIDE.md

# 3. Build and test
cmake --build .
./bin/music-player --list-plugins
```
│
└── src/
    └── main.cpp              # Application entry point
```

## File Count Summary

- **Total Source Files**: 27
  - C++ Implementation: 11 (.cpp files)
  - C++ Headers: 8 (.h files)
  - CMake Files: 7
  - Documentation: 3 (.md files)

- **Build Artifacts**:
  - Executable: 1 (music-player)
  - Static Libraries: 2
  - Shared Libraries: 1 (WAV plugin)

## Binary Sizes

- Main executable: 57 KB
- Core engine library: 82 KB  
- Platform abstraction: 13 KB
- WAV decoder plugin: 29 KB
- **Total**: ~181 KB

## Creating New Plugins

### 1. Create Plugin Source

```cpp
#include "mp_plugin.h"
#include "mp_decoder.h"

class MyDecoderPlugin : public mp::IPlugin {
public:
    const mp::PluginInfo& get_plugin_info() const override {
        static mp::PluginInfo info = {
            "My Decoder",
            "Author Name",
            "Description",
            mp::Version(1, 0, 0),
            mp::Version(0, 1, 0),
            "com.example.decoder.myformat"
        };
        return info;
    }
    
    mp::PluginCapability get_capabilities() const override {
        return mp::PluginCapability::Decoder;
    }
    
    mp::Result initialize(mp::IServiceRegistry* services) override {
        return mp::Result::Success;
    }
    
    void shutdown() override {}
};

MP_DEFINE_PLUGIN(MyDecoderPlugin)
```

### 2. Add to CMakeLists.txt

```cmake
add_library(my_decoder SHARED my_decoder.cpp)
target_link_libraries(my_decoder PRIVATE sdk_headers)
set_target_properties(my_decoder PROPERTIES PREFIX "")
```

### 3. Build and Test

```bash
cd build
make
# Plugin will be in build/lib/my_decoder.so
```

## Development Workflow

1. **Modify Source**: Edit files in core/, sdk/, platform/, or plugins/
2. **Rebuild**: `cd build && make -j$(nproc)`
3. **Test**: `./bin/music-player`
4. **Debug**: `gdb ./bin/music-player`

## Key Interfaces

### IPlugin (Base Plugin Interface)
- `get_plugin_info()` - Metadata
- `get_capabilities()` - Feature flags
- `initialize()` - Setup with service registry
- `shutdown()` - Cleanup
- `get_service()` - Expose services

### IDecoder (Audio Decoder Interface)
- `probe_file()` - Format detection
- `open_stream()` - Open audio file
- `get_stream_info()` - Get audio properties
- `decode_block()` - Decode audio data
- `seek()` - Random access
- `get_metadata()` - Extract tags
- `close_stream()` - Cleanup

### IEventBus (Event System)
- `subscribe()` - Listen for events
- `unsubscribe()` - Stop listening
- `publish()` - Async event dispatch
- `publish_sync()` - Sync event dispatch

### IServiceRegistry
- `register_service()` - Publish service
- `unregister_service()` - Remove service
- `query_service()` - Find service

## Testing the Event System

The main application demonstrates event bus usage:

```cpp
auto* event_bus = engine.get_event_bus();
auto handle = event_bus->subscribe(mp::EVENT_PLAYBACK_STARTED, 
    [](const mp::Event& evt) {
        std::cout << "Event received!" << std::endl;
    }
);

mp::Event test_event(mp::EVENT_PLAYBACK_STARTED);
event_bus->publish(test_event);

event_bus->unsubscribe(handle);
```

## Next Development Steps

According to the design document priority:

1. **Configuration Manager** - JSON-based settings persistence
2. **Audio Output** - Complete ALSA implementation for actual playback
3. **FLAC Decoder** - High-priority open-source format
4. **Basic UI** - Simple playback controls
5. **Playlist System** - Basic playlist management

## Troubleshooting

**Plugin not loading?**
- Check plugin is in `build/lib/` directory
- Verify plugin exports `create_plugin` and `destroy_plugin`
- Check plugin UUID is unique

**Build errors?**
- Ensure CMake 3.20+ installed: `/usr/bin/cmake --version`
- Check compiler: `g++ --version` (need GCC 7+ or Clang 5+)
- Verify SDK headers included: `#include "mp_plugin.h"`

**Runtime errors?**
- Check plugin version compatibility
- Verify service registry has required services
- Review error messages for specific failure points

## Resources

- Design Document: `.qoder/quests/music-player-development.md`
- Implementation Summary: `IMPLEMENTATION_SUMMARY.md`
- Project README: `README.md`
- SDK Headers: `sdk/headers/*.h`
