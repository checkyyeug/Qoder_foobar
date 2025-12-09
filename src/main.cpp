#include "core_engine.h"
#include "audio_output_factory.h"
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cmath>
#include <filesystem>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Global running flag for signal handling
static std::atomic<bool> g_running(true);

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nReceived shutdown signal, exiting..." << std::endl;
        g_running = false;
    }
}

int main(int argc, char** argv) {
    std::cout << "========================================" << std::endl;
    std::cout << "   Professional Music Player v0.1.0" << std::endl;
    std::cout << "   Cross-Platform Audio Player" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // Install signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    // Create core engine
    mp::core::CoreEngine engine;
    
    // Initialize core engine
    mp::Result result = engine.initialize();
    if (result != mp::Result::Success) {
        std::cerr << "Failed to initialize core engine" << std::endl;
        return 1;
    }
    
    // Determine plugin directory
    const char* plugin_dir = nullptr;
    if (argc > 1) {
        plugin_dir = argv[1];
    } else {
        // Try build directory first, then installation directory
        plugin_dir = "build/lib";
        
        // Check if directory exists using C++17 filesystem
        namespace fs = std::filesystem;
        if (!fs::exists(plugin_dir) || !fs::is_directory(plugin_dir)) {
#ifdef _WIN32
            plugin_dir = "C:/Program Files/Music Player/plugins";
#else
            plugin_dir = "/usr/local/lib/music-player/plugins";
#endif
        }
    }
    
    // Load plugins
    std::cout << std::endl;
    result = engine.load_plugins(plugin_dir);
    if (result != mp::Result::Success) {
        std::cout << "Note: No plugins loaded (this is normal for initial build)" << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Core engine running. Press Ctrl+C to exit." << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // Test event bus
    auto* event_bus = engine.get_event_bus();
    if (event_bus) {
        // Subscribe to test event
        auto handle = event_bus->subscribe(mp::EVENT_PLAYBACK_STARTED, 
            [](const mp::Event& evt) {
                std::cout << "Event received: Playback Started (ID: " << evt.id << ")" << std::endl;
            }
        );
        
        // Publish test event
        mp::Event test_event(mp::EVENT_PLAYBACK_STARTED);
        event_bus->publish(test_event);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        event_bus->unsubscribe(handle);
    }
    
    // Test configuration manager
    auto* config = engine.get_config_manager();
    if (config) {
        std::cout << "Testing Configuration Manager..." << std::endl;
        
        // Set some default configuration values
        config->set_string("audio", "output_device", "default");
        config->set_int("audio", "sample_rate", 44100);
        config->set_int("audio", "buffer_size", 1024);
        config->set_bool("audio", "exclusive_mode", false);
        
        config->set_string("playback", "repeat_mode", "off");
        config->set_bool("playback", "gapless", true);
        config->set_float("playback", "volume", 0.8);
        
        config->set_string("library", "music_folder", "/home/user/Music");
        config->set_bool("library", "auto_scan", true);
        
        // Read back values
        std::string device = config->get_string("audio", "output_device");
        int sample_rate = config->get_int("audio", "sample_rate");
        bool gapless = config->get_bool("playback", "gapless");
        
        std::cout << "  Audio output device: " << device << std::endl;
        std::cout << "  Sample rate: " << sample_rate << " Hz" << std::endl;
        std::cout << "  Gapless playback: " << (gapless ? "enabled" : "disabled") << std::endl;
        std::cout << "  Configuration will be auto-saved on exit" << std::endl;
        std::cout << std::endl;
    }
    
    // Test platform audio output
    std::cout << "Testing Platform Audio Output..." << std::endl;
    mp::IAudioOutput* audio_output = mp::platform::create_platform_audio_output();
    if (audio_output) {
        // Enumerate devices
        const mp::AudioDeviceInfo* devices = nullptr;
        size_t device_count = 0;
        mp::Result result = audio_output->enumerate_devices(&devices, &device_count);
        
        if (result == mp::Result::Success && device_count > 0) {
            std::cout << "  Found " << device_count << " audio device(s):" << std::endl;
            for (size_t i = 0; i < device_count; ++i) {
                std::cout << "    - " << devices[i].name 
                          << " (" << devices[i].max_channels << " channels, "
                          << devices[i].default_sample_rate << " Hz)" << std::endl;
            }
            
            // Test audio playback with a simple sine wave (440 Hz A note)
            std::cout << "  Testing audio playback (2 second 440 Hz tone)..." << std::endl;
            
            struct AudioContext {
                float phase = 0.0f;
                float frequency = 440.0f; // A note
                float sample_rate = 44100.0f;
            };
            
            AudioContext audio_ctx;
            
            auto audio_callback = [](void* buffer, size_t frames, void* user_data) {
                AudioContext* ctx = static_cast<AudioContext*>(user_data);
                float* output = static_cast<float*>(buffer);
                
                float phase_increment = 2.0f * static_cast<float>(M_PI) * ctx->frequency / ctx->sample_rate;
                
                for (size_t i = 0; i < frames; ++i) {
                    float sample = 0.3f * std::sin(ctx->phase); // 30% volume
                    output[i * 2] = sample;     // Left channel
                    output[i * 2 + 1] = sample; // Right channel
                    
                    ctx->phase += phase_increment;
                    if (ctx->phase > 2.0f * static_cast<float>(M_PI)) {
                        ctx->phase -= 2.0f * static_cast<float>(M_PI);
                    }
                }
            };
            
            mp::AudioOutputConfig audio_config = {};
            audio_config.device_id = nullptr; // Use default device
            audio_config.sample_rate = 44100;
            audio_config.channels = 2;
            audio_config.format = mp::SampleFormat::Float32;
            audio_config.buffer_frames = 1024;
            audio_config.callback = audio_callback;
            audio_config.user_data = &audio_ctx;
            
            audio_ctx.sample_rate = static_cast<float>(audio_config.sample_rate);
            
            result = audio_output->open(audio_config);
            if (result == mp::Result::Success) {
                std::cout << "  Audio device opened successfully" << std::endl;
                std::cout << "  Latency: " << audio_output->get_latency() << " ms" << std::endl;
                
                result = audio_output->start();
                if (result == mp::Result::Success) {
                    std::cout << "  Playback started..." << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                    
                    audio_output->stop();
                    std::cout << "  Playback stopped" << std::endl;
                } else {
                    std::cout << "  Failed to start playback" << std::endl;
                }
                
                audio_output->close();
            } else {
                std::cout << "  Failed to open audio device" << std::endl;
            }
        } else {
            std::cout << "  No audio devices found or enumeration failed" << std::endl;
        }
        
        delete audio_output;
        std::cout << std::endl;
    }
    
    // Main loop
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Shutting down..." << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Shutdown is automatic via CoreEngine destructor
    
    std::cout << "Goodbye!" << std::endl;
    return 0;
}
