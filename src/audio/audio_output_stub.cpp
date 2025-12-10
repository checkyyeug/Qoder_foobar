/**
 * @file audio_output_stub.cpp
 * @brief Stub audio output implementation (always available)
 * @date 2025-12-10
 */

#include "audio_output.h"
#include <cstring>

namespace audio {

/**
 * @brief Stub audio output implementation
 *
 * This implementation does nothing but provides a valid audio output
 * that will always compile and run without any dependencies.
 */
class AudioOutputStub final : public IAudioOutput {
private:
    AudioFormat format_;
    bool is_open_;
    int latency_;
    int buffer_size_;

public:
    AudioOutputStub()
        : is_open_(false)
        , latency_(0)
        , buffer_size_(1024) {
    }

    ~AudioOutputStub() override {
        close();
    }

    bool open(const AudioFormat& format) override {
        format_ = format;
        is_open_ = true;

        // Calculate some reasonable defaults
        latency_ = 100; // 100ms latency
        buffer_size_ = format.sample_rate / 10; // 100ms buffer

        return true;
    }

    void close() override {
        is_open_ = false;
    }

    int write(const float* buffer, int frames) override {
        if (!is_open_) return 0;

        // Silently discard the audio data
        // In real implementation, this would send to audio device

        // Return the number of frames "processed"
        return frames;
    }

    int get_latency() const override {
        return latency_;
    }

    int get_buffer_size() const override {
        return buffer_size_;
    }

    bool is_ready() const override {
        return is_open_;
    }
};

// Factory function for stub backend (only used when no other backend is available)
std::unique_ptr<IAudioOutput> create_stub_audio_output() {
    return std::make_unique<AudioOutputStub>();
}

} // namespace audio