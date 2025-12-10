/**
 * @file plugin_audio_decoder.cpp
 * @brief PluginAudioDecoder implementation
 * @date 2025-12-10
 */

#include "plugin_audio_decoder.h"
#include "cubic_resampler.h"
#include "../../compat/foobar_compat_manager.h"
#include "../../compat/sdk_implementations/input_decoder_impl.h"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iostream>

// External service query function
extern "C" bool FOOBAR2000_GUID service_query(const foobar2000::GUID&, void**);

// Static factory members
std::unique_ptr<PluginAudioDecoder> PluginAudioDecoderFactory::shared_decoder_;

PluginAudioDecoder::PluginAudioDecoder(mp::compat::FoobarPluginLoader* plugin_loader,
                                     int target_rate)
    : plugin_loader_(plugin_loader)
    , target_sample_rate_(target_rate) {

    audio_info_ = AudioInfo();

    // Create service bridge if needed
    if (!plugin_loader_) {
        auto compat_manager = std::make_unique<mp::compat::FoobarCompatManager>();
        plugin_loader_ = new mp::compat::FoobarPluginLoader(compat_manager.get());
        service_bridge_ = std::make_unique<mp::compat::ServiceRegistryBridgeImpl>(
            /*service_registry=*/nullptr); // We'll need to integrate with actual registry
    }
}

PluginAudioDecoder::~PluginAudioDecoder() {
    close_file();

    // Clean up if we own the loader
    if (service_bridge_ && plugin_loader_) {
        delete plugin_loader_;
    }
}

bool PluginAudioDecoder::initialize(const char* plugin_directory) {
    // Initialize plugin system
    if (!initialize_plugin_system()) {
        std::cerr << "Failed to initialize plugin system" << std::endl;
        return false;
    }

    // Load plugins from directory if provided
    if (plugin_directory) {
        int loaded = load_plugins_from_directory(plugin_directory);
        std::cout << "Loaded " << loaded << " plugins from " << plugin_directory << std::endl;
    }

    // Register known decoders
    register_known_decoders();

    return true;
}

bool PluginAudioDecoder::initialize_plugin_system() {
    // Initialize foobar2000 compatibility
    mp::compat::CompatConfig config;
    config.enable_plugin_compat = true;
    config.adapter_logging_level = 2; // Warnings

    // Note: This would need the actual FoobarCompatManager initialization
    // For now, we'll assume it's initialized externally

    return true;
}

void PluginAudioDecoder::register_known_decoders() {
    // Register built-in decoders
    using namespace foobar2000;

    // WAV decoder
    auto wav_decoder = std::make_unique<InputDecoderImpl>("WAV Decoder", "wav", input_decoders::wav);
    register_service(input_decoders::wav, std::move(wav_decoder));

    // FLAC decoder
    auto flac_decoder = std::make_unique<InputDecoderImpl>("FLAC Decoder", "flac", input_decoders::flac);
    register_service(input_decoders::flac, std::move(flac_decoder));

    // MP3 decoder
    auto mp3_decoder = std::make_unique<InputDecoderImpl>("MP3 Decoder", "mp3", input_decoders::mpg);
    register_service(input_decoders::mpg, std::move(mp3_decoder));

    // PCM decoder
    auto pcm_decoder = std::make_unique<InputDecoderImpl>("PCM Decoder", "pcm", input_decoders::pcm);
    register_service(input_decoders::pcm, std::move(pcm_decoder));
}

bool PluginAudioDecoder::open_file(const char* path) {
    if (!path) return false;

    // Close any existing file
    close_file();

    // Find appropriate decoder
    current_decoder_ = find_decoder_for_file(path);
    if (!current_decoder_.is_valid()) {
        std::cerr << "No decoder found for file: " << path << std::endl;
        return false;
    }

    // Try to open the file
    int result = current_decoder_->open(path);
    if (result != 0) {
        std::cerr << "Failed to open file: " << path << " (error: " << result << ")" << std::endl;
        current_decoder_.release();
        return false;
    }

    // Store file path
    current_file_path_ = path;

    // Get audio information
    audio_info_.sample_rate = current_decoder_->get_sample_rate();
    audio_info_.channels = current_decoder_->get_channels();
    audio_info_.bits_per_sample = current_decoder_->get_bits_per_sample();
    audio_info_.total_samples = current_decoder_->get_length();
    audio_info_.duration_seconds = static_cast<double>(audio_info_.total_samples) / audio_info_.sample_rate;
    audio_info_.format_name = get_decoder_name();

    // Setup resampler if needed
    if (target_sample_rate_ > 0 && target_sample_rate_ != audio_info_.sample_rate) {
        resampler_ = audio::CubicSampleRateConverterFactory::create();
        if (!resampler_->initialize(audio_info_.sample_rate, target_sample_rate_, audio_info_.channels)) {
            std::cerr << "Failed to initialize resampler" << std::endl;
            resampler_.reset();
        }
    }

    // Extract metadata (simplified)
    metadata_.clear();
    // In a real implementation, we would extract metadata from the file

    return true;
}

void PluginAudioDecoder::close_file() {
    if (current_decoder_.is_valid()) {
        current_decoder_.release();
    }

    current_file_path_.clear();
    audio_info_ = AudioInfo();
    metadata_.clear();
    resampler_.reset();
    conversion_buffer_.clear();
}

int PluginAudioDecoder::decode_frames(float* output, int max_frames) {
    if (!current_decoder_.is_valid() || !output || max_frames <= 0) {
        return 0;
    }

    int frames_decoded = 0;

    // If using resampler
    if (resampler_) {
        // Calculate how many input frames we need
        int ratio = static_cast<int>(target_sample_rate_) / audio_info_.sample_rate;
        int input_frames_needed = max_frames * audio_info_.sample_rate / target_sample_rate_ + 1;

        // Ensure conversion buffer is large enough
        conversion_buffer_.resize(input_frames_needed * audio_info_.channels);

        // Decode from plugin
        int input_frames = current_decoder_->decode(
            conversion_buffer_.data(),
            input_frames_needed * sizeof(float));

        if (input_frames > 0) {
            // Resample
            frames_decoded = resampler_->convert(
                conversion_buffer_.data(), input_frames,
                output, max_frames);
        }
    } else {
        // Decode directly
        frames_decoded = current_decoder_->decode(
            output, max_frames * sizeof(float));
    }

    return frames_decoded;
}

bool PluginAudioDecoder::seek(int64_t position) {
    if (!current_decoder_.is_valid()) {
        return false;
    }

    current_decoder_->seek(position);
    return true;
}

std::string PluginAudioDecoder::get_metadata(const char* key) const {
    if (!key) return "";

    auto it = std::find_if(metadata_.begin(), metadata_.end(),
        [key](const Metadata& m) { return m.key == key; });

    return (it != metadata_.end()) ? it->value : "";
}

bool PluginAudioDecoder::can_decode(const char* path) const {
    if (!path) return false;

    auto decoder = find_decoder_for_file(path);
    return decoder.is_valid();
}

std::vector<std::string> PluginAudioDecoder::get_supported_extensions() const {
    std::vector<std::string> extensions;

    // Common audio formats
    extensions.push_back("wav");
    extensions.push_back("flac");
    extensions.push_back("mp3");
    extensions.push_back("ogg");
    extensions.push_back("m4a");
    extensions.push_back("aac");
    extensions.push_back("wma");
    extensions.push_back("ape");
    extensions.push_back("m4a");
    extensions.push_back("mp4");

    return extensions;
}

void PluginAudioDecoder::set_target_sample_rate(int rate) {
    if (rate != target_sample_rate_) {
        target_sample_rate_ = rate;

        // Reinitialize resampler if file is open
        if (current_decoder_.is_valid() && rate > 0 && rate != audio_info_.sample_rate) {
            resampler_ = audio::CubicSampleRateConverterFactory::create();
            if (!resampler_->initialize(audio_info_.sample_rate, rate, audio_info_.channels)) {
                resampler_.reset();
            }
        }
    }
}

std::string PluginAudioDecoder::get_decoder_name() const {
    if (!current_decoder_.is_valid()) {
        return "";
    }

    // In a real implementation, we would get the name from the decoder
    // For now, return based on file extension
    if (current_file_path_.empty()) {
        return "Unknown";
    }

    std::filesystem::path path(current_file_path_);
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".mp3") return "MP3 Decoder";
    if (ext == ".flac") return "FLAC Decoder";
    if (ext == ".wav") return "WAV Decoder";
    if (ext == ".ogg") return "OGG Vorbis Decoder";
    if (ext == ".m4a" || ext == ".aac") return "AAC Decoder";

    return "Generic Decoder";
}

int PluginAudioDecoder::load_plugins_from_directory(const char* directory) {
    if (!directory || !plugin_loader_) {
        return 0;
    }

    // Load plugins using the plugin loader
    auto result = plugin_loader_->load_plugins_from_directory(directory);
    if (result != mp::Result::Success) {
        std::cerr << "Failed to load plugins from " << directory << std::endl;
        return 0;
    }

    return static_cast<int>(plugin_loader_->get_module_count());
}

std::pair<size_t, size_t> PluginAudioDecoder::get_plugin_stats() const {
    size_t plugin_count = 0;
    size_t decoder_count = 0;

    if (plugin_loader_) {
        plugin_count = plugin_loader_->get_module_count();
        const auto& services = plugin_loader_->get_services();

        // Count input decoder services
        for (const auto& service : services) {
            if (service.available && service.name.find("input") != std::string::npos) {
                decoder_count++;
            }
        }
    }

    return {plugin_count, decoder_count};
}

foobar2000::service_ptr_t<foobar2000::input_decoder>
PluginAudioDecoder::find_decoder_for_file(const char* path) {
    using namespace foobar2000;

    if (!path) return service_ptr_t<input_decoder>();

    // Get file extension
    std::filesystem::path file_path(path);
    std::string ext = file_path.extension().string();
    if (ext.empty()) return service_ptr_t<input_decoder>();

    // Remove dot and convert to lowercase
    if (ext[0] == '.') ext = ext.substr(1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    void* service_ptr = nullptr;

    // Check against known decoders
    if (ext == "wav" || ext == "wave") {
        if (service_query(input_decoders::wav, &service_ptr)) {
            return service_ptr_t<input_decoder>(static_cast<input_decoder*>(service_ptr));
        }
    }

    if (ext == "flac") {
        if (service_query(input_decoders::flac, &service_ptr)) {
            return service_ptr_t<input_decoder>(static_cast<input_decoder*>(service_ptr));
        }
    }

    if (ext == "mp3" || ext == "mpg" || ext == "mpeg") {
        if (service_query(input_decoders::mpg, &service_ptr)) {
            return service_ptr_t<input_decoder>(static_cast<input_decoder*>(service_ptr));
        }
    }

    // Try generic decoders
    if (service_query(input_decoders::pcm, &service_ptr)) {
        return service_ptr_t<input_decoder>(static_cast<input_decoder*>(service_ptr));
    }

    return service_ptr_t<input_decoder>();
}

// Factory implementation
std::unique_ptr<PluginAudioDecoder> PluginAudioDecoderFactory::create(
    const char* plugin_directory, int target_rate) {

    auto decoder = std::make_unique<PluginAudioDecoder>(nullptr, target_rate);

    if (!decoder->initialize(plugin_directory)) {
        return nullptr;
    }

    return decoder;
}

PluginAudioDecoder& PluginAudioDecoderFactory::get_shared() {
    if (!shared_decoder_) {
        shared_decoder_ = create();
    }
    return *shared_decoder_;
}

bool PluginAudioDecoderFactory::is_initialized() {
    return shared_decoder_ != nullptr;
}