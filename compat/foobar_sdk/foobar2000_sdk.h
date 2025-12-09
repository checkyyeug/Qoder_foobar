#pragma once

// Foobar2000 SDK Interface Stubs
// This file contains minimal interface definitions for foobar2000 plugin compatibility
// These are clean-room implementations based on publicly documented interfaces

#include <cstdint>
#include <cstddef>

namespace foobar2000_sdk {

// Forward declarations
class service_base;
class abort_callback;

// Reference counting base class (mimics foobar2000's service_base)
class service_base {
public:
    virtual ~service_base() = default;
    
    // Reference counting interface
    virtual int service_add_ref() = 0;
    virtual int service_release() = 0;
    
protected:
    service_base() = default;
    service_base(const service_base&) = delete;
    service_base& operator=(const service_base&) = delete;
};

// Smart pointer for service objects (mimics service_ptr_t)
template<typename T>
class service_ptr_t {
public:
    service_ptr_t() : ptr_(nullptr) {}
    
    explicit service_ptr_t(T* p) : ptr_(p) {
        if (ptr_) ptr_->service_add_ref();
    }
    
    service_ptr_t(const service_ptr_t& other) : ptr_(other.ptr_) {
        if (ptr_) ptr_->service_add_ref();
    }
    
    service_ptr_t(service_ptr_t&& other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }
    
    ~service_ptr_t() {
        if (ptr_) ptr_->service_release();
    }
    
    service_ptr_t& operator=(const service_ptr_t& other) {
        if (this != &other) {
            if (ptr_) ptr_->service_release();
            ptr_ = other.ptr_;
            if (ptr_) ptr_->service_add_ref();
        }
        return *this;
    }
    
    service_ptr_t& operator=(service_ptr_t&& other) noexcept {
        if (this != &other) {
            if (ptr_) ptr_->service_release();
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }
    
    T* operator->() const { return ptr_; }
    T& operator*() const { return *ptr_; }
    T* get_ptr() const { return ptr_; }
    
    bool is_valid() const { return ptr_ != nullptr; }
    bool is_empty() const { return ptr_ == nullptr; }
    
    void release() {
        if (ptr_) {
            ptr_->service_release();
            ptr_ = nullptr;
        }
    }
    
private:
    T* ptr_;
};

// Abort callback interface (for cancellable operations)
class abort_callback {
public:
    virtual ~abort_callback() = default;
    
    // Check if operation should be aborted
    virtual bool is_aborting() const = 0;
    
    // Sleep with abort check
    virtual void sleep(double seconds) = 0;
};

// No-op abort callback
class abort_callback_dummy : public abort_callback {
public:
    bool is_aborting() const override { return false; }
    void sleep(double) override {}
};

// Audio sample type
typedef float audio_sample;

// Audio chunk structure (mimics foobar2000's audio_chunk)
class audio_chunk {
public:
    audio_chunk() 
        : sample_rate_(0)
        , channels_(0)
        , channel_config_(0)
        , sample_count_(0)
        , data_(nullptr)
        , data_capacity_(0) {}
    
    virtual ~audio_chunk() {
        if (data_) {
            delete[] data_;
        }
    }
    
    // Sample rate in Hz
    uint32_t get_sample_rate() const { return sample_rate_; }
    void set_sample_rate(uint32_t rate) { sample_rate_ = rate; }
    
    // Number of channels
    uint32_t get_channels() const { return channels_; }
    void set_channels(uint32_t ch) { channels_ = ch; }
    
    // Channel configuration bitmask
    uint32_t get_channel_config() const { return channel_config_; }
    void set_channel_config(uint32_t config) { channel_config_ = config; }
    
    // Number of samples per channel
    size_t get_sample_count() const { return sample_count_; }
    void set_sample_count(size_t count) { sample_count_ = count; }
    
    // Get audio data pointer
    const audio_sample* get_data() const { return data_; }
    audio_sample* get_data() { return data_; }
    
    // Set data size and allocate if needed
    void set_data_size(size_t samples_per_channel) {
        sample_count_ = samples_per_channel;
        size_t total_samples = samples_per_channel * channels_;
        
        if (total_samples > data_capacity_) {
            if (data_) delete[] data_;
            data_ = new audio_sample[total_samples];
            data_capacity_ = total_samples;
        }
    }
    
    // Get duration in seconds
    double get_duration() const {
        if (sample_rate_ == 0) return 0.0;
        return static_cast<double>(sample_count_) / static_cast<double>(sample_rate_);
    }
    
    // Reset chunk
    void reset() {
        sample_count_ = 0;
    }
    
private:
    uint32_t sample_rate_;
    uint32_t channels_;
    uint32_t channel_config_;
    size_t sample_count_;
    audio_sample* data_;
    size_t data_capacity_;
};

// File information structure
struct file_info {
    // Metadata tag
    struct tag {
        const char* name;
        const char* value;
    };
    
    // Technical info
    double length;          // Duration in seconds
    uint32_t sample_rate;
    uint32_t channels;
    uint32_t bitrate;       // In kbps
    const char* codec;
    
    // Metadata tags
    tag* tags;
    size_t tag_count;
    
    file_info() 
        : length(0.0)
        , sample_rate(0)
        , channels(0)
        , bitrate(0)
        , codec(nullptr)
        , tags(nullptr)
        , tag_count(0) {}
    
    ~file_info() {
        if (tags) delete[] tags;
    }
};

} // namespace foobar2000_sdk
