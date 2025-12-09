# Quick Start Guide

## Building the Project

```bash
cd /data/checky/foobar/opencode_foobar
mkdir -p build && cd build
/usr/bin/cmake ..
make -j$(nproc)
```

## Running the Application

```bash
# From project root
./build/bin/music-player

# Or specify custom plugin directory
./build/bin/music-player /path/to/plugins
```

## Project Layout

```
opencode_foobar/
├── build/                    # Build output
│   ├── bin/music-player     # Main executable (57 KB)
│   └── lib/
│       ├── libcore_engine.a           # Core library (82 KB)
│       ├── libplatform_abstraction.a  # Platform layer (13 KB)
│       └── wav_decoder.so             # WAV plugin (29 KB)
│
├── core/                     # Core engine (9 files)
│   ├── core_engine.{h,cpp}
│   ├── event_bus.{h,cpp}
│   ├── plugin_host.{h,cpp}
│   └── service_registry.{h,cpp}
│
├── sdk/headers/              # Public API (5 headers)
│   ├── mp_types.h
│   ├── mp_plugin.h
│   ├── mp_event.h
│   ├── mp_decoder.h
│   └── mp_audio_output.h
│
├── platform/linux/           # Linux platform implementation
│   └── audio_output_alsa.cpp
│
├── plugins/decoders/         # Decoder plugins
│   └── wav_decoder.cpp
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
