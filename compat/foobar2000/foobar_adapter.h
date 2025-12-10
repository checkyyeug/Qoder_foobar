/**
 * @file foobar_adapter.h
 * @brief foobar2000插件兼容适配器
 * @date 2025-12-10
 */

#pragma once

#include "../../sdk/qoder_plugin_sdk.h"
#include "../foobar_sdk/foobar2000.h"
#include <memory>
#include <vector>
#include <unordered_map>

namespace qoder::compat {

/**
 * @brief foobar2000解码器到Qoder接口的适配器
 */
class FoobarDecoderAdapter : public IAudioDecoder {
private:
    foobar2000::service_ptr_t<foobar2000::input_decoder> foobar_decoder_;
    std::string file_path_;
    AudioFormat format_;
    int64_t total_samples_;

public:
    explicit FoobarDecoderAdapter(foobar2000::input_decoder* decoder);
    ~FoobarDecoderAdapter() override = default;

    // IPlugin接口
    bool initialize() override;
    void finalize() override;
    PluginInfo get_info() const override;
    PluginState get_state() const override;
    void set_state(PluginState state) override;
    std::string get_last_error() const override;

    // IAudioDecoder接口
    bool can_decode(const std::string& file_path) override;
    std::vector<std::string> get_supported_extensions() override;
    bool open(const std::string& file_path) override;
    int decode(AudioBuffer& buffer, int max_frames) override;
    bool seek(int64_t sample_pos) override;
    void close() override;
    AudioFormat get_format() const override;
    int64_t get_length() const override;
    double get_duration() const override;
    std::vector<MetadataItem> get_metadata() override;
    std::string get_metadata_value(const std::string& key) override;
    int64_t get_position() const override;
    bool is_eof() const override;
};

/**
 * @brief foobar2000插件包装器
 */
class FoobarPluginWrapper {
private:
    std::string plugin_path_;
    void* library_handle_;
    std::vector<std::unique_ptr<IPluginFactory>> adapter_factories_;

public:
    FoobarPluginWrapper();
    ~FoobarPluginWrapper();

    // 加载foobar2000插件
    bool load_plugin(const std::string& path);

    // 获取适配后的解码器
    std::vector<std::unique_ptr<IAudioDecoder>> get_decoders();

    // 获取插件信息
    std::vector<PluginInfo> get_plugin_info() const;

private:
    // 枚举插件提供的服务
    void enumerate_services();

    // 创建解码器适配器
    void create_decoder_adapter(const foobar2000::GUID& guid,
                                const std::string& name);
};

/**
 * @brief foobar解码器适配器工厂
 */
class FoobarDecoderFactory : public ITypedPluginFactory<IAudioDecoder> {
private:
    std::function<std::unique_ptr<IAudioDecoder>()> creator_;

public:
    explicit FoobarDecoderFactory(std::function<std::unique_ptr<IAudioDecoder>()> creator)
        : creator_(std::move(creator)) {}

    std::unique_ptr<IAudioDecoder> create_typed() override {
        return creator_();
    }

    PluginInfo get_info() const override;
};

} // namespace qoder::compat