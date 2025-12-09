# Plugin Development Guide

## Introduction

This guide provides step-by-step instructions for developing plugins for the Professional Music Player. The plugin system is designed to be simple, stable, and cross-platform compatible.

## Plugin Architecture Overview

### Key Principles

1. **C ABI Compatibility**: Plugins use C-style interfaces to ensure binary compatibility across compilers
2. **Explicit Versioning**: All interfaces include version numbers for compatibility checking
3. **Capability-Based**: Plugins declare their capabilities for runtime feature detection
4. **Service-Oriented**: Plugins access core services through a service registry
5. **Lifecycle Management**: Clear initialization and shutdown sequences

### Plugin Types

| Type | Capability | Purpose | Examples |
|------|-----------|---------|----------|
| **Decoder** | `PluginCapability::Decoder` | Decode audio formats | MP3, FLAC, Opus |
| **Encoder** | `PluginCapability::Encoder` | Encode audio formats | FLAC, Opus |
| **DSP** | `PluginCapability::DSP` | Process audio | EQ, Reverb, Limiter |
| **Output** | `PluginCapability::Output` | Audio output devices | ALSA, PulseAudio |
| **Visualizer** | `PluginCapability::Visualizer` | Visualize audio | Spectrum, Oscilloscope |
| **UI Component** | `PluginCapability::UIComponent` | User interface | Custom panels |

## Getting Started

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.20+
- Music Player SDK headers

### Project Setup

Create a new directory for your plugin:

```bash
mkdir my_plugin
cd my_plugin
```

Create `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyPlugin VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find Music Player SDK
find_path(MP_SDK_INCLUDE_DIR mp_plugin.h
    PATHS /usr/local/include/music-player
          /usr/include/music-player
          ${CMAKE_SOURCE_DIR}/../sdk/headers
)

include_directories(${MP_SDK_INCLUDE_DIR})

# Create plugin shared library
add_library(my_plugin SHARED
    my_plugin.cpp
)

# Set plugin properties
set_target_properties(my_plugin PROPERTIES
    PREFIX ""  # No 'lib' prefix
    SUFFIX ".so"  # Use .so on all platforms
)

install(TARGETS my_plugin
    LIBRARY DESTINATION lib/music-player/plugins
)
```

## Example 1: Simple Decoder Plugin

Let's create a simple decoder plugin for a fictional "SimpleAudio" format.

### Header File (simple_audio_decoder.h)

```cpp
#pragma once

#include "mp_plugin.h"
#include "mp_decoder.h"
#include <memory>
#include <vector>
#include <fstream>

class SimpleAudioDecoder : public mp::IPlugin, public mp::IDecoder {
public:
    SimpleAudioDecoder();
    ~SimpleAudioDecoder() override;
    
    // IPlugin interface
    const mp::PluginInfo& get_plugin_info() const override;
    mp::PluginCapability get_capabilities() const override;
    mp::Result initialize(mp::IServiceRegistry* services) override;
    void shutdown() override;
    void* get_service(mp::ServiceID id) override;
    
    // IDecoder interface
    mp::Result open(const char* path) override;
    mp::Result get_stream_info(mp::AudioStreamInfo* info) override;
    int decode(mp::AudioBuffer* buffer) override;
    mp::Result seek(uint64_t sample_position) override;
    void close() override;
    bool can_decode(const char* path) override;
    
private:
    mp::IServiceRegistry* services_;
    std::unique_ptr<std::ifstream> file_;
    mp::AudioStreamInfo stream_info_;
    bool is_open_;
    uint64_t current_position_;
};
```

### Implementation (simple_audio_decoder.cpp)

```cpp
#include "simple_audio_decoder.h"
#include <cstring>
#include <iostream>

// Plugin metadata
static const mp::PluginInfo PLUGIN_INFO = {
    "Simple Audio Decoder",                    // name
    "Your Name",                                // author
    "Decoder for SimpleAudio format",          // description
    mp::Version(1, 0, 0),                      // version
    mp::API_VERSION,                           // min_api_version
    "com.yourcompany.decoder.simpleaudio"      // uuid
};

SimpleAudioDecoder::SimpleAudioDecoder()
    : services_(nullptr)
    , is_open_(false)
    , current_position_(0) {
    std::memset(&stream_info_, 0, sizeof(stream_info_));
}

SimpleAudioDecoder::~SimpleAudioDecoder() {
    close();
}

const mp::PluginInfo& SimpleAudioDecoder::get_plugin_info() const {
    return PLUGIN_INFO;
}

mp::PluginCapability SimpleAudioDecoder::get_capabilities() const {
    return mp::PluginCapability::Decoder;
}

mp::Result SimpleAudioDecoder::initialize(mp::IServiceRegistry* services) {
    services_ = services;
    std::cout << "Simple Audio Decoder initialized" << std::endl;
    return mp::Result::Success;
}

void SimpleAudioDecoder::shutdown() {
    close();
    services_ = nullptr;
    std::cout << "Simple Audio Decoder shutdown" << std::endl;
}

void* SimpleAudioDecoder::get_service(mp::ServiceID id) {
    if (id == mp::SERVICE_DECODER) {
        return static_cast<mp::IDecoder*>(this);
    }
    return nullptr;
}

bool SimpleAudioDecoder::can_decode(const char* path) {
    // Check file extension
    const char* ext = std::strrchr(path, '.');
    if (!ext) return false;
    return std::strcmp(ext, ".sa") == 0 || std::strcmp(ext, ".simpleaudio") == 0;
}

mp::Result SimpleAudioDecoder::open(const char* path) {
    if (is_open_) {
        close();
    }
    
    // Open file
    file_ = std::make_unique<std::ifstream>(path, std::ios::binary);
    if (!file_->is_open()) {
        return mp::Result::FileNotFound;
    }
    
    // Read header (example format)
    struct Header {
        char magic[4];      // "SA01"
        uint32_t sample_rate;
        uint16_t channels;
        uint16_t bits_per_sample;
        uint64_t total_samples;
    } header;
    
    file_->read(reinterpret_cast<char*>(&header), sizeof(header));
    
    if (std::memcmp(header.magic, "SA01", 4) != 0) {
        file_.reset();
        return mp::Result::InvalidFormat;
    }
    
    // Fill stream info
    stream_info_.sample_rate = header.sample_rate;
    stream_info_.channels = header.channels;
    stream_info_.total_samples = header.total_samples;
    stream_info_.duration_ms = (header.total_samples * 1000) / header.sample_rate;
    
    // Determine format
    if (header.bits_per_sample == 16) {
        stream_info_.format = mp::SampleFormat::Int16;
    } else if (header.bits_per_sample == 24) {
        stream_info_.format = mp::SampleFormat::Int24;
    } else {
        file_.reset();
        return mp::Result::NotSupported;
    }
    
    is_open_ = true;
    current_position_ = 0;
    
    return mp::Result::Success;
}

mp::Result SimpleAudioDecoder::get_stream_info(mp::AudioStreamInfo* info) {
    if (!is_open_) {
        return mp::Result::NotInitialized;
    }
    
    *info = stream_info_;
    return mp::Result::Success;
}

int SimpleAudioDecoder::decode(mp::AudioBuffer* buffer) {
    if (!is_open_ || !file_) {
        return -1;
    }
    
    // Check for EOF
    if (current_position_ >= stream_info_.total_samples) {
        buffer->end_of_stream = true;
        return 0;
    }
    
    // Calculate frames to read
    uint32_t frames_to_read = buffer->capacity;
    uint64_t remaining = stream_info_.total_samples - current_position_;
    if (frames_to_read > remaining) {
        frames_to_read = static_cast<uint32_t>(remaining);
        buffer->end_of_stream = true;
    }
    
    // Read data
    size_t bytes_per_sample = (stream_info_.format == mp::SampleFormat::Int16) ? 2 : 3;
    size_t bytes_to_read = frames_to_read * stream_info_.channels * bytes_per_sample;
    
    file_->read(static_cast<char*>(buffer->data), bytes_to_read);
    size_t bytes_read = file_->gcount();
    
    uint32_t frames_read = bytes_read / (stream_info_.channels * bytes_per_sample);
    
    // Fill buffer metadata
    buffer->sample_rate = stream_info_.sample_rate;
    buffer->channels = stream_info_.channels;
    buffer->format = stream_info_.format;
    buffer->frames = frames_read;
    buffer->timestamp_us = (current_position_ * 1000000) / stream_info_.sample_rate;
    buffer->position_samples = current_position_;
    
    current_position_ += frames_read;
    
    return frames_read;
}

mp::Result SimpleAudioDecoder::seek(uint64_t sample_position) {
    if (!is_open_ || !file_) {
        return mp::Result::NotInitialized;
    }
    
    if (sample_position >= stream_info_.total_samples) {
        return mp::Result::InvalidParameter;
    }
    
    // Calculate file position
    size_t bytes_per_sample = (stream_info_.format == mp::SampleFormat::Int16) ? 2 : 3;
    size_t header_size = 20; // Size of our header
    size_t file_position = header_size + 
        (sample_position * stream_info_.channels * bytes_per_sample);
    
    file_->clear();
    file_->seekg(file_position);
    
    if (file_->fail()) {
        return mp::Result::Error;
    }
    
    current_position_ = sample_position;
    return mp::Result::Success;
}

void SimpleAudioDecoder::close() {
    if (file_) {
        file_->close();
        file_.reset();
    }
    is_open_ = false;
    current_position_ = 0;
    std::memset(&stream_info_, 0, sizeof(stream_info_));
}

// Plugin entry points
MP_DEFINE_PLUGIN(SimpleAudioDecoder)
```

### Building the Plugin

```bash
mkdir build && cd build
cmake ..
make
sudo make install
```

## Example 2: Simple DSP Plugin (Volume Control)

### Implementation (volume_dsp.cpp)

```cpp
#include "mp_plugin.h"
#include "mp_dsp.h"
#include <cmath>
#include <algorithm>

class VolumeDSP : public mp::IPlugin, public mp::IDSPProcessor {
public:
    VolumeDSP() 
        : services_(nullptr)
        , gain_(1.0f)
        , bypass_(false) {
    }
    
    // IPlugin interface
    const mp::PluginInfo& get_plugin_info() const override {
        static const mp::PluginInfo info = {
            "Volume Control DSP",
            "Music Player Team",
            "Simple volume control plugin",
            mp::Version(1, 0, 0),
            mp::API_VERSION,
            "com.musicplayer.dsp.volume"
        };
        return info;
    }
    
    mp::PluginCapability get_capabilities() const override {
        return mp::PluginCapability::DSP;
    }
    
    mp::Result initialize(mp::IServiceRegistry* services) override {
        services_ = services;
        return mp::Result::Success;
    }
    
    void shutdown() override {
        services_ = nullptr;
    }
    
    void* get_service(mp::ServiceID id) override {
        if (id == mp::SERVICE_DSP_PROCESSOR) {
            return static_cast<mp::IDSPProcessor*>(this);
        }
        return nullptr;
    }
    
    // IDSPProcessor interface
    mp::Result process(mp::AudioBuffer* input, mp::AudioBuffer* output) override {
        if (bypass_ || gain_ == 1.0f) {
            // No processing needed
            if (output != input) {
                *output = *input;
            }
            return mp::Result::Success;
        }
        
        // Only process Float32 format
        if (input->format != mp::SampleFormat::Float32) {
            return mp::Result::NotSupported;
        }
        
        float* in_data = static_cast<float*>(input->data);
        float* out_data = (output != input) ? static_cast<float*>(output->data) : in_data;
        
        uint32_t total_samples = input->frames * input->channels;
        
        for (uint32_t i = 0; i < total_samples; i++) {
            float sample = in_data[i] * gain_;
            // Clip to prevent overflow
            sample = std::max(-1.0f, std::min(1.0f, sample));
            out_data[i] = sample;
        }
        
        // Copy metadata
        if (output != input) {
            output->sample_rate = input->sample_rate;
            output->channels = input->channels;
            output->format = input->format;
            output->frames = input->frames;
            output->timestamp_us = input->timestamp_us;
            output->position_samples = input->position_samples;
            output->end_of_stream = input->end_of_stream;
        }
        
        return mp::Result::Success;
    }
    
    uint32_t get_latency_samples() override {
        return 0; // No latency
    }
    
    void reset() override {
        // No state to reset
    }
    
    void set_bypass(bool bypass) override {
        bypass_ = bypass;
    }
    
    // Custom methods for volume control
    void set_volume_db(float db) {
        gain_ = std::pow(10.0f, db / 20.0f);
    }
    
    void set_volume_linear(float linear) {
        gain_ = std::max(0.0f, std::min(2.0f, linear));
    }
    
private:
    mp::IServiceRegistry* services_;
    float gain_;
    bool bypass_;
};

MP_DEFINE_PLUGIN(VolumeDSP)
```

## Best Practices

### 1. Thread Safety

**Critical Rules**:
- Decoders: `decode()` called from I/O thread, `get_info()` from any thread
- DSP: `process()` called from AUDIO THREAD (real-time constraints)
- Never use locks in audio-critical paths

**Example**: Thread-safe parameter updates in DSP

```cpp
class MyDSP : public mp::IDSPProcessor {
private:
    std::atomic<float> gain_;  // Use atomics for simple types
    
public:
    void set_gain(float g) {
        gain_.store(g, std::memory_order_release);
    }
    
    mp::Result process(mp::AudioBuffer* input, mp::AudioBuffer* output) override {
        float current_gain = gain_.load(std::memory_order_acquire);
        // Use current_gain for processing
    }
};
```

### 2. Memory Management

**Avoid allocations in real-time paths**:

```cpp
class MyDecoder : public mp::IDecoder {
private:
    std::vector<uint8_t> decode_buffer_;  // Pre-allocated
    
public:
    mp::Result open(const char* path) override {
        // Allocate during open (not real-time)
        decode_buffer_.resize(MAX_BUFFER_SIZE);
        return mp::Result::Success;
    }
    
    int decode(mp::AudioBuffer* buffer) override {
        // Use pre-allocated buffer, no allocations here
        // ...
    }
};
```

### 3. Error Handling

**Always validate inputs**:

```cpp
int decode(mp::AudioBuffer* buffer) override {
    if (!buffer || !buffer->data) {
        return -1;  // Error
    }
    
    if (buffer->capacity < MIN_FRAMES) {
        return -1;  // Error
    }
    
    // Proceed with decoding
}
```

### 4. Logging

Use service registry to get logging service:

```cpp
mp::Result initialize(mp::IServiceRegistry* services) override {
    services_ = services;
    
    // Get logger service
    auto* logger = static_cast<mp::ILogger*>(
        services->query_service(mp::SERVICE_LOGGER)
    );
    
    if (logger) {
        logger->log(mp::LogLevel::Info, "MyPlugin initialized");
    }
    
    return mp::Result::Success;
}
```

### 5. Configuration

Store plugin settings in global config:

```cpp
mp::Result initialize(mp::IServiceRegistry* services) override {
    services_ = services;
    
    // Get config service
    auto* config = static_cast<mp::core::ConfigManager*>(
        services->query_service(mp::SERVICE_CONFIG_MANAGER)
    );
    
    if (config) {
        // Read settings
        int quality = config->get_int("MyPlugin", "quality", 5);
        bool enabled = config->get_bool("MyPlugin", "enabled", true);
        
        // Apply settings
        set_quality(quality);
        set_enabled(enabled);
    }
    
    return mp::Result::Success;
}
```

## Testing Your Plugin

### Unit Testing

Create test cases for your plugin:

```cpp
#include <gtest/gtest.h>
#include "my_plugin.h"

TEST(MyDecoderTest, OpenValidFile) {
    MyDecoder decoder;
    
    mp::Result result = decoder.open("test_files/valid.sa");
    EXPECT_EQ(result, mp::Result::Success);
    
    mp::AudioStreamInfo info;
    result = decoder.get_stream_info(&info);
    EXPECT_EQ(result, mp::Result::Success);
    EXPECT_EQ(info.sample_rate, 44100);
    
    decoder.close();
}

TEST(MyDecoderTest, DecodeBuffer) {
    MyDecoder decoder;
    decoder.open("test_files/valid.sa");
    
    std::vector<float> buffer_data(2048 * 2);  // 2048 frames, stereo
    mp::AudioBuffer buffer;
    buffer.data = buffer_data.data();
    buffer.capacity = 2048;
    
    int frames = decoder.decode(&buffer);
    EXPECT_GT(frames, 0);
    EXPECT_LE(frames, 2048);
    
    decoder.close();
}
```

### Integration Testing

Test with actual music player:

```bash
# Build and install plugin
make install

# Run music player
music-player

# Check logs for plugin loading
# Play test files
```

## Debugging

### Enable Debug Logging

```cpp
#ifdef DEBUG
    #define PLUGIN_LOG(msg) std::cout << "[MyPlugin] " << msg << std::endl
#else
    #define PLUGIN_LOG(msg)
#endif

int decode(mp::AudioBuffer* buffer) override {
    PLUGIN_LOG("Decoding " << buffer->capacity << " frames");
    // ...
}
```

### Common Issues

| Issue | Symptom | Solution |
|-------|---------|----------|
| **Crashes on load** | Segfault during initialization | Check ABI compatibility, verify SDK version |
| **No audio output** | Plugin loads but no sound | Verify buffer format matches output expectations |
| **Crackling/glitches** | Audio artifacts | Check for buffer underruns, optimize `process()` |
| **Memory leaks** | Memory grows over time | Use valgrind, check cleanup in `shutdown()` |

## Distribution

### Packaging

Create a distributable package:

```bash
# Create package directory
mkdir -p my_plugin-1.0.0
cp build/my_plugin.so my_plugin-1.0.0/
cp README.md my_plugin-1.0.0/
cp LICENSE my_plugin-1.0.0/

# Create tarball
tar czf my_plugin-1.0.0.tar.gz my_plugin-1.0.0/
```

### Installation Instructions

Provide clear install instructions:

```markdown
## Installation

1. Download `my_plugin-1.0.0.tar.gz`
2. Extract: `tar xzf my_plugin-1.0.0.tar.gz`
3. Copy plugin: `sudo cp my_plugin-1.0.0/my_plugin.so /usr/local/lib/music-player/plugins/`
4. Restart music player
```

## Resources

- **SDK Reference**: See `sdk/headers/` for interface definitions
- **Examples**: Check `plugins/` directory for sample plugins
- **Architecture**: Read `docs/AUDIO_PIPELINE.md` for pipeline details
- **Community**: Join discussions at [forum/discord]

## Support

For help with plugin development:
- Check existing plugins for examples
- Read API documentation in header files
- Ask on community forum
- Report bugs on issue tracker
