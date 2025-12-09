# Audio Pipeline Specification

## Overview

This document specifies the audio pipeline architecture for the Professional Music Player. The pipeline is designed to support high-quality audio playback with low latency, gapless transitions, and flexible DSP processing.

## Architecture Principles

### 1. Thread Model

The audio pipeline operates on **three distinct thread contexts**:

- **Main/UI Thread**: Configuration, playlist management, user interaction
- **I/O Thread**: File reading, network streaming, metadata extraction
- **Audio Thread**: Real-time audio processing (HIGHEST PRIORITY)

**Critical Constraint**: The audio thread must NEVER block. No locks, no allocations, no I/O.

### 2. Data Flow Model

The pipeline uses a **pull-based model** where the audio output callback requests data:

```
[Decoder] → [DSP Chain] → [Format Converter] → [Audio Output]
     ↑           ↑              ↑                    ↑
  I/O Thread  Audio Thread   Audio Thread       Audio Thread
```

### 3. Buffer Management

**Lock-Free Ring Buffers** connect components with different thread contexts:

- **Decode Buffer**: I/O thread → Audio thread
- Size: 2-5 seconds of audio (configurable)
- Implementation: Single-producer, single-consumer lock-free queue
- Format: Decoded PCM in native decoder output format

## Buffer Format Specification

### Primary Audio Buffer Format

All audio data within the pipeline uses a standardized PCM buffer format:

```cpp
struct AudioBuffer {
    // Format information
    uint32_t sample_rate;        // Sample rate in Hz (e.g., 44100, 48000, 192000)
    uint16_t channels;           // Number of channels (1=mono, 2=stereo, 6=5.1, etc.)
    SampleFormat format;         // Sample format (see below)
    ChannelLayout layout;        // Channel layout (standard, custom)
    
    // Buffer data
    void* data;                  // Pointer to sample data
    uint32_t frames;             // Number of frames (samples per channel)
    uint32_t capacity;           // Buffer capacity in frames
    
    // Timing information
    uint64_t timestamp_us;       // Timestamp in microseconds from track start
    uint64_t position_samples;   // Position in samples from track start
    
    // Flags
    bool end_of_stream;          // True if this is the last buffer
    bool discontinuity;          // True if there's a gap before this buffer
};
```

### Sample Formats

The pipeline supports the following sample formats:

| Format | Type | Bits | Range | Usage |
|--------|------|------|-------|-------|
| **Int16** | Signed Integer | 16 | -32768 to 32767 | CD quality, low memory |
| **Int24** | Signed Integer | 24 | -8388608 to 8388607 | High-res (packed 32-bit) |
| **Int32** | Signed Integer | 32 | -2147483648 to 2147483647 | High-res integer |
| **Float32** | Floating Point | 32 | -1.0 to 1.0 | **Preferred for DSP** |
| **Float64** | Floating Point | 64 | -1.0 to 1.0 | Ultra-high precision |

**Standard Format**: Float32 is the standard format for internal processing. All DSP plugins should accept and produce Float32.

### Channel Layouts

Standard channel layouts:

```cpp
enum class ChannelLayout {
    Mono,           // 1 channel: C
    Stereo,         // 2 channels: L, R
    Stereo_LFE,     // 2.1 channels: L, R, LFE
    Surround_5_1,   // 6 channels: L, R, C, LFE, Ls, Rs
    Surround_7_1,   // 8 channels: L, R, C, LFE, Ls, Rs, Lb, Rb
    Custom          // Custom layout (requires channel map)
};
```

### Sample Data Organization

**Interleaved** (default for I/O and output):
```
[L0][R0][L1][R1][L2][R2]...
```

**Planar** (optional for some decoders):
```
[L0][L1][L2]... [R0][R1][R2]...
```

Most decoders output interleaved. DSP plugins may request planar for efficiency.

## Component Specifications

### 1. Decoder Plugin Interface

#### Responsibilities
- Read compressed audio from file/stream
- Decode to PCM
- Provide format information
- Support seeking
- Extract metadata

#### Thread Context
- **create/destroy**: Main thread
- **decode**: I/O thread
- **get_info**: Any thread (must be thread-safe)

#### Key Methods

```cpp
class IDecoder {
    // Initialize decoder with file path or stream
    virtual Result open(const char* path) = 0;
    
    // Get stream information
    virtual Result get_stream_info(AudioStreamInfo* info) = 0;
    
    // Decode next chunk of audio
    // Returns number of frames decoded, 0 on EOF, negative on error
    virtual int decode(AudioBuffer* buffer) = 0;
    
    // Seek to position in samples
    virtual Result seek(uint64_t sample_position) = 0;
    
    // Close and cleanup
    virtual void close() = 0;
};
```

#### Performance Requirements
- **Decode latency**: < 10ms per call
- **Seek latency**: < 50ms for local files, < 500ms for streams
- **Memory**: No allocations during decode() if possible
- **Thread safety**: open/close/seek may be called from different threads

#### Buffer Size Recommendations
- Decode 1024-8192 frames per call (prefer 2048)
- Smaller = lower latency, higher CPU overhead
- Larger = more efficient, higher latency

### 2. DSP Plugin Interface

#### Responsibilities
- Process audio buffers in-place or produce output
- Report processing latency
- Support bypass mode
- Handle sample rate changes gracefully

#### Thread Context
- **process**: AUDIO THREAD (real-time constraints)
- **configure**: Main thread
- **reset**: Audio thread (during stream changes)

#### Key Methods

```cpp
class IDSPProcessor {
    // Initialize DSP with configuration
    virtual Result initialize(const DSPConfig* config) = 0;
    
    // Process audio buffer
    // May modify buffer in-place or write to output buffer
    virtual Result process(AudioBuffer* input, AudioBuffer* output) = 0;
    
    // Get processing latency in samples
    virtual uint32_t get_latency_samples() = 0;
    
    // Reset internal state (e.g., on track change)
    virtual void reset() = 0;
    
    // Enable/disable processing
    virtual void set_bypass(bool bypass) = 0;
};
```

#### Performance Requirements
- **CRITICAL**: process() must complete in < 5ms for 2048 frames @ 44.1kHz
- **No allocations** during process()
- **No locks** during process()
- **No I/O** during process()
- Fail gracefully if input format changes

#### DSP Chain Order

Standard chain order (user-configurable):

1. **ReplayGain** - Volume normalization
2. **EQ** - Equalization
3. **Compressor/Limiter** - Dynamic range control
4. **Crossfeed** - Stereo enhancement for headphones
5. **Sample Rate Converter** - Resample to output rate
6. **Channel Mixer** - Channel count conversion

### 3. Format Converter

#### Responsibilities
- Convert between sample formats
- Convert between sample rates
- Convert between channel layouts
- Apply dithering when reducing bit depth

#### Thread Context
- **Audio thread** (must be real-time safe)

#### Sample Rate Conversion

**Algorithms**:
- **Linear interpolation**: Fast, low quality (for emergency fallback)
- **Windowed sinc**: High quality, medium CPU (recommended)
- **libsamplerate**: Highest quality, higher CPU (optional)

**Latency**: Report converter latency to pipeline for A/V sync

#### Bit Depth Conversion

When converting to lower bit depth:
- **Dithering**: Apply TPDF dither to prevent quantization noise
- **Noise shaping**: Optional, reduces audible dither noise

Example: Float32 → Int16
```
1. Multiply by 32767.0
2. Add dither noise (TPDF: random[-1,1])
3. Clamp to [-32768, 32767]
4. Round to integer
```

### 4. Audio Output Interface

#### Responsibilities
- Enumerate available audio devices
- Open device with specified format
- Provide low-latency callback-based playback
- Handle device disconnection gracefully

#### Thread Context
- **Callback**: AUDIO THREAD (highest priority)
- **Control methods**: Main thread

#### Key Methods

```cpp
class IAudioOutput {
    // Enumerate devices
    virtual Result enumerate_devices(DeviceList* devices) = 0;
    
    // Open device with format
    virtual Result open(const char* device_id, const AudioFormat* format,
                       AudioCallback callback, void* user_data) = 0;
    
    // Start playback
    virtual Result start() = 0;
    
    // Stop playback
    virtual Result stop() = 0;
    
    // Close device
    virtual Result close() = 0;
    
    // Get current buffer status
    virtual Result get_buffer_status(BufferStatus* status) = 0;
};
```

#### Callback Signature

```cpp
typedef void (*AudioCallback)(
    void* user_data,          // User data pointer
    AudioBuffer* buffer,      // Buffer to fill
    uint32_t requested_frames // Number of frames requested
);
```

#### Platform-Specific Backends

| Platform | API | Mode | Latency | Priority |
|----------|-----|------|---------|----------|
| **Linux** | ALSA | mmap | 10-20ms | Primary |
| **Linux** | PulseAudio | callback | 20-40ms | Fallback |
| **Linux** | PipeWire | callback | 10-20ms | Future |
| **Windows** | WASAPI Exclusive | event | 3-10ms | Primary |
| **Windows** | WASAPI Shared | callback | 10-20ms | Fallback |
| **macOS** | CoreAudio | callback | 5-10ms | Primary |

#### Buffer Size Configuration

**Target latency**: 20-50ms (user-configurable)

Example for 44.1kHz:
- 10ms = 441 frames
- 20ms = 882 frames (common)
- 50ms = 2205 frames

Use power-of-2 sizes when possible: 512, 1024, 2048

## Gapless Playback Design

### Requirements

1. No audible gap between tracks
2. Respect encoder delay and padding metadata
3. Handle sample rate changes gracefully
4. Pre-buffer next track while current playing

### Implementation Strategy

#### Dual Decoder Architecture

```
Current Track Decoder (Decoder A)  →  [Buffer A]  ↘
                                                    → [Mixer] → Output
Next Track Decoder (Decoder B)     →  [Buffer B]  ↗
```

**State Machine**:

1. **Single track playing**: Decoder A active, Decoder B idle
2. **Near end of track** (< 5 sec remaining): Start Decoder B, begin buffering
3. **Track boundary**: Switch from Buffer A to Buffer B atomically
4. **Track changed**: Decoder A becomes idle, Decoder B becomes active

#### Track Boundary Detection

Use **sample-accurate position** from decoder:
```cpp
if (current_position + buffer_frames >= total_samples - encoder_padding) {
    // This is the last buffer, prepare for transition
    trigger_next_track_buffering();
}
```

#### Handling Sample Rate Changes

If next track has different sample rate:

**Option 1 - Seamless (no gap, requires resampling)**:
- Resample next track to current output rate
- Crossfade over 10-50ms

**Option 2 - Brief silence (tiny gap, no resampling)**:
- Stop output
- Reconfigure output device
- Resume with new rate
- Gap duration: ~50-200ms (device dependent)

**User choice**: Configuration option for seamless vs. bit-perfect

#### Metadata Handling

Support for gapless metadata:

- **MP3**: LAME encoder delay/padding in LAME header or iTunes metadata
- **FLAC**: FLAC__STREAM_METADATA_APPLICATION with padding info
- **AAC**: iTunSMPB atom in MP4 container
- **Vorbis**: Custom comments

Parse and apply:
```
actual_start_sample = declared_start + encoder_delay
actual_end_sample = declared_end - encoder_padding
```

## Error Handling

### Decoder Errors

| Error | Recovery Strategy |
|-------|-------------------|
| File not found | Skip to next track, show notification |
| Corrupt data | Attempt to continue, skip if unrecoverable |
| Unsupported format | Skip track, log error |
| Seek failure | Reset decoder, try from beginning |

### Output Errors

| Error | Recovery Strategy |
|-------|-------------------|
| Buffer underrun | Insert silence, log warning |
| Device disconnected | Pause playback, attempt fallback device |
| Format not supported | Try format conversion, fallback to compatible format |

### DSP Errors

| Error | Recovery Strategy |
|-------|-------------------|
| Processing too slow | Skip DSP, log warning, continue |
| Unexpected format | Bypass DSP, log error |
| Internal error | Bypass DSP, notify user |

## Performance Targets

### Latency Budget

Total latency from file to speaker:

| Component | Latency | Notes |
|-----------|---------|-------|
| Decode | 0-5ms | Buffered on I/O thread |
| DSP Chain | 5-10ms | Depends on plugins |
| Format Conversion | 0-2ms | If needed |
| Output Buffer | 10-50ms | Configurable |
| **Total** | **15-67ms** | Acceptable for music |

### CPU Usage

**Target**: < 5% on modern CPU (4-core, 2GHz+) during playback

Measurement:
- Decode: < 1% CPU
- DSP: < 3% CPU (depends on plugins)
- Output: < 1% CPU
- Other: < 1% CPU

### Memory Usage

**Audio buffers**:
- Decode buffer: 5 sec @ 192kHz stereo Float32 = 7.3 MB
- Output buffer: 50ms @ 192kHz stereo Float32 = 75 KB
- DSP internal: Varies, typically < 1 MB per plugin

**Total audio memory**: < 20 MB typical, < 50 MB worst case

## Testing Requirements

### Functional Tests

1. **Format Support**: Verify all supported formats decode correctly
2. **Gapless**: Test with gapless test tracks (available from hydrogenaud.io)
3. **Seeking**: Verify sample-accurate seeking
4. **DSP Chain**: Test all DSP combinations
5. **Device Changes**: Test hot-plug/unplug scenarios

### Performance Tests

1. **Latency**: Measure end-to-end latency with loopback
2. **CPU**: Profile under various loads
3. **Buffer Underruns**: Test with disk I/O stress
4. **Memory**: Check for leaks over 24-hour playback

### Quality Tests

1. **Bit-Perfect**: Verify output matches input for lossless formats (use null sum test)
2. **Dither**: Verify proper dithering on bit depth reduction
3. **Resampling**: Verify quality with frequency sweeps and SNR measurements
4. **Clipping**: Verify no clipping on full-scale signals

## Implementation Checklist

### Phase 1 - Basic Playback (Complete)
- [x] Audio buffer format definition
- [x] Decoder interface specification
- [ ] Basic WAV/FLAC decoder implementation
- [ ] Lock-free ring buffer
- [ ] Audio output interface
- [ ] ALSA backend implementation

### Phase 2 - DSP Framework (Pending)
- [ ] DSP plugin interface
- [ ] DSP chain processor
- [ ] Volume control DSP
- [ ] ReplayGain scanner and applier
- [ ] Sample rate converter
- [ ] Format converter with dithering

### Phase 3 - Gapless (Pending)
- [ ] Dual decoder state machine
- [ ] Metadata parser for encoder delay/padding
- [ ] Seamless sample rate switching
- [ ] Track boundary detection
- [ ] Pre-buffering logic

### Phase 4 - Advanced (Future)
- [ ] Network streaming support
- [ ] Audio device hot-plug handling
- [ ] Multi-channel (5.1/7.1) support
- [ ] DSD playback (DoP)
- [ ] VST plugin hosting

## References

- **LAME Encoder Delay**: http://lame.sourceforge.net/tech-FAQ.txt
- **FLAC Format**: https://xiph.org/flac/format.html
- **ReplayGain Spec**: https://wiki.hydrogenaud.io/index.php?title=ReplayGain_2.0_specification
- **Lock-Free Queues**: Herb Sutter's "Writing Lock-Free Code" series
- **Audio Programming**: "Designing Audio Effect Plugins in C++" by Will Pirkle
