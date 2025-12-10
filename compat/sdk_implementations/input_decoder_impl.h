/**
 * @file input_decoder_impl.h
 * @brief Input decoder implementation
 * @date 2025-12-10
 */

#pragma once

#include "service_base_impl.h"
#include "../foobar_sdk/foobar2000.h"
#include <string>

namespace foobar2000 {

/**
 * @brief Concrete input decoder implementation
 */
class InputDecoderImpl : public service_base_impl {
private:
    GUID class_guid_;
    std::string format_name_;
    std::vector<std::string> extensions_;

public:
    InputDecoderImpl(const char* format_name, const char* extension, const GUID& guid);
    virtual ~InputDecoderImpl() = default;

    // service_base implementation
    const GUID* service_get_class_guid() override {
        return &class_guid_;
    }

    // input_decoder interface
    bool can_decode(const char* path) override;
    int open(const char* path) override;
    int decode(void* buffer, int bytes) override;
    void seek(int64_t position) override;
    int64_t get_length() override;
    int get_sample_rate() override;
    int get_channels() override;
    int get_bits_per_sample() override;

    // Helper methods
    void add_extension(const char* ext);
    const std::string& get_format_name() const { return format_name_; }
};

} // namespace foobar2000