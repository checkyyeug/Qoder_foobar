/**
 * @file audio_output.h
 * @brief Unified audio output interface
 * @date 2025-12-10
 */

#pragma once

#include <cstdint>
#include <memory>

namespace audio {

/**
 * @brief Audio format specification
 */
struct AudioFormat {
    uint32_t sample_rate;    // Sample rate in Hz
    uint16_t channels;       // Number of channels (1=mono, 2=stereo)
    uint16_t bits_per_sample; // Bits per sample (16, 24, 32)

    AudioFormat() : sample_rate(44100), channels(2), bits_per_sample(16) {}
};

/**
 * @brief Audio output interface
 *
 * All audio backends must implement this interface
 */
class IAudioOutput {
public:
    virtual ~IAudioOutput() = default;

    /**
     * @brief Open audio device with specified format
     * @param format Audio format
     * @return true on success
     */
    virtual bool open(const AudioFormat& format) = 0;

    /**
     * @brief Close audio device
     */
    virtual void close() = 0;

    /**
     * @brief Write audio data to device
     * @param buffer Audio data (interleaved float samples)
     * @param frames Number of frames to write
     * @return Number of frames actually written
     */
    virtual int write(const float* buffer, int frames) = 0;

    /**
     * @brief Get output latency in milliseconds
     * @return Latency in ms
     */
    virtual int get_latency() const = 0;

    /**
     * @brief Get buffer size in frames
     * @return Buffer size
     */
    virtual int get_buffer_size() const = 0;

    /**
     * @brief Check if device is ready
     * @return true if ready
     */
    virtual bool is_ready() const = 0;
};

/**
 * @brief Factory function to create audio output
 * @return Unique pointer to audio output instance
 */
std::unique_ptr<IAudioOutput> create_audio_output();

/**
 * @brief Get the name of the audio backend being used
 * @return Backend name (e.g., "ALSA", "WASAPI", "CoreAudio", "stub")
 */
const char* get_audio_backend_name();

} // namespace audio