#include "core_engine.h"
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <cstdlib>

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
        
        // Check if directory exists
        if (access(plugin_dir, F_OK) != 0) {
            plugin_dir = "/usr/local/lib/music-player/plugins";
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
