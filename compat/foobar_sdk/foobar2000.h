/**
 * @file foobar2000.h
 * @brief foobar2000 SDK compatibility definitions
 * @date 2025-12-10
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

// Platform-specific definitions
#ifdef _WIN32
    #define FOOBAR2000_EXPORT __declspec(dllexport)
    #define FOOBAR2000_CALL __stdcall
    #define FOOBAR2000_GUID WINAPI
#else
    #define FOOBAR2000_EXPORT __attribute__((visibility("default")))
    #define FOOBAR2000_CALL
    #define FOOBAR2000_GUID
#endif

// Forward declarations
class service_base;
class input_impl;
class output_impl;

namespace foobar2000 {

// GUID structure (Windows-style)
struct GUID {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t  Data4[8];

    bool operator==(const GUID& other) const {
        return Data1 == other.Data1 && Data2 == other.Data2 &&
               Data3 == other.Data3 &&
               memcmp(Data4, other.Data4, 8) == 0;
    }
};

// Service interfaces
class service_base {
public:
    virtual ~service_base() = default;

    // Service reference counting
    virtual void service_add_ref() = 0;
    virtual bool service_release() = 0;

    // Service queries
    virtual bool service_query(const GUID& guid, void** out) = 0;

    // Service information
    virtual const char* service_get_name() = 0;
    virtual const GUID* service_get_class_guid() = 0;

protected:
    service_base() = default;
};

// Smart pointer for service objects
template<typename T>
class service_ptr_t {
private:
    T* ptr_;

public:
    service_ptr_t() : ptr_(nullptr) {}
    service_ptr_t(T* p) : ptr_(p) {
        if (ptr_) ptr_->service_add_ref();
    }
    service_ptr_t(const service_ptr_t& other) : ptr_(other.ptr_) {
        if (ptr_) ptr_->service_add_ref();
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

    T* get() const { return ptr_; }
    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }
    bool is_valid() const { return ptr_ != nullptr; }

    void release() {
        if (ptr_) {
            ptr_->service_release();
            ptr_ = nullptr;
        }
    }
};

// Input decoder interface
class input_decoder : public service_base {
public:
    virtual ~input_decoder() = default;

    // Decoder information
    virtual bool can_decode(const char* path) = 0;
    virtual int open(const char* path) = 0;
    virtual int decode(void* buffer, int bytes) = 0;
    virtual void seek(int64_t position) = 0;
    virtual int64_t get_length() = 0;
    virtual int get_sample_rate() = 0;
    virtual int get_channels() = 0;
    virtual int get_bits_per_sample() = 0;
};

// Output interface
class output : public service_base {
public:
    virtual ~output() = default;

    // Output control
    virtual bool open(const char* device_id) = 0;
    virtual int write(const void* buffer, int bytes) = 0;
    virtual int get_latency() = 0;
    virtual void flush() = 0;
    virtual void close() = 0;
};

// Abort callback interface
class abort_callback {
public:
    virtual ~abort_callback() = default;

    virtual void abort() = 0;
    virtual bool is_aborting() = 0 = 0;
};

// Audio chunk (multi-value metadata support)
class audio_chunk {
public:
    struct sample_spec {
        int sample_rate;
        int channels;
        int bits_per_sample;
        uint32_t channel_mask;
    };

    audio_chunk() = default;
    ~audio_chunk() = default;

    virtual void set_data(const float* data, int samples, const sample_spec& spec) = 0;
    virtual const float* get_data() const = 0;
    virtual int get_sample_count() const = 0;
    virtual const sample_spec& get_spec() const = 0;
    virtual void copy(const audio_chunk& other) = 0;
};

// File information
class file_info {
public:
    virtual ~file_info() = default;

    // Basic info
    virtual void reset() = 0;
    virtual bool is_valid() const = 0;

    // Metadata
    virtual void set_meta(const char* key, const char* value) = 0;
    virtual const char* get_meta(const char* key) const = 0;
    virtual void remove_meta(const char* key) = 0;

    // Length
    virtual void set_length(int64_t length) = 0;
    virtual int64_t get_length() const = 0;

    // Properties
    virtual void set_property(const char* key, const char* value) = 0;
    virtual const char* get_property(const char* key) const = 0;
};

// Metadb handle
class metadb_handle {
public:
    virtual ~metadb_handle() = default;

    virtual bool is_valid() const = 0;
    virtual void get_info(file_info& info) const = 0;
    virtual void set_info(const file_info& info) = 0;
};

// Plugin entry point
typedef struct {
    const char* version;
    uint32_t api_version;
    service_ptr_t<service_base>* services;
    uint32_t service_count;
} initquit_t;

// Plugin interface
class input_factory {
public:
    virtual ~input_factory() = 0;

    virtual service_ptr_t<input_decoder> create() = 0;
    virtual bool is_our_path(const char* path) = 0;
    virtual const char* get_name() = 0;
    virtual const char* get_extension_list() = 0;
};

// Common GUIDs
namespace service_ids {
    extern const GUID input_decoder_v2;
    extern const GUID metadb_v2;
    extern const GUID output_v2;
    extern const guid_playback_control_v2;
}

namespace input_decoders {
    extern const GUID mpg;
    extern const GUID pcm;
    extern const GUID flac;
    extern const GUID wav;
}

} // namespace foobar2000

// Plugin entry points
extern "C" {
    typedef FOOBAR2000_EXPORT int (FOOBAR2000_CALL* init_stage_t)(const foobar2000::initquit_t*);
    typedef FOOBAR2000_EXPORT void (FOOBAR2000_CALL* quit_stage_t)();
    typedef FOOBAR2000_EXPORT bool (FOOBAR2000_CALL* service_query_t)(const GUID&, void**);
}