#include "core_engine.h"
#include <iostream>

namespace mp {
namespace core {

CoreEngine::CoreEngine()
    : initialized_(false) {
}

CoreEngine::~CoreEngine() {
    shutdown();
}

Result CoreEngine::initialize() {
    if (initialized_) {
        return Result::AlreadyInitialized;
    }
    
    std::cout << "Initializing Music Player Core Engine..." << std::endl;
    
    // Create service registry
    service_registry_ = std::make_unique<ServiceRegistry>();
    
    // Create event bus
    event_bus_ = std::make_unique<EventBus>();
    
    // Create config manager
    config_manager_ = std::make_unique<ConfigManager>();
    std::string config_path = "music-player.json"; // Default config path
    config_manager_->initialize(config_path);
    config_manager_->set_auto_save(true);
    
    // Create playlist manager
    playlist_manager_ = std::make_unique<PlaylistManager>();
    std::string config_dir = "."; // Use current directory for now
    playlist_manager_->initialize(config_dir.c_str());
    
    // Create visualization engine
    visualization_engine_ = std::make_unique<VisualizationEngine>();
    VisualizationConfig viz_config{};
    viz_config.waveform_width = 800;
    viz_config.waveform_time_span = 5.0f;
    viz_config.fft_size = 2048;
    viz_config.spectrum_bars = 30;
    viz_config.spectrum_min_freq = 20.0f;
    viz_config.spectrum_max_freq = 20000.0f;
    viz_config.spectrum_smoothing = 0.75f;
    viz_config.vu_peak_decay_rate = 10.0f;
    viz_config.vu_rms_window_ms = 100.0f;
    viz_config.update_rate_hz = 60;
    visualization_engine_->initialize(viz_config);
    
    // Create plugin host
    plugin_host_ = std::make_unique<PluginHost>(service_registry_.get());
    
    // Register core services
    service_registry_->register_service(SERVICE_EVENT_BUS, event_bus_.get());
    service_registry_->register_service(SERVICE_PLUGIN_HOST, plugin_host_.get());
    service_registry_->register_service(SERVICE_CONFIG_MANAGER, config_manager_.get());
    service_registry_->register_service(SERVICE_PLAYLIST_MANAGER, playlist_manager_.get());
    service_registry_->register_service(SERVICE_VISUALIZATION, visualization_engine_.get());
    
    // Start event bus
    event_bus_->start();
    
    initialized_ = true;
    
    std::cout << "Core Engine initialized successfully" << std::endl;
    
    return Result::Success;
}

void CoreEngine::shutdown() {
    if (!initialized_) {
        return;
    }
    
    std::cout << "Shutting down Music Player Core Engine..." << std::endl;
    
    // Shutdown plugins first
    if (plugin_host_) {
        plugin_host_->shutdown_plugins();
    }
    
    // Stop event bus
    if (event_bus_) {
        event_bus_->stop();
    }
    
    // Shutdown config manager (saves if auto-save enabled)
    if (config_manager_) {
        config_manager_->shutdown();
    }
    
    // Shutdown playlist manager (saves all playlists)
    if (playlist_manager_) {
        playlist_manager_->shutdown();
    }
    
    // Shutdown visualization engine
    if (visualization_engine_) {
        visualization_engine_->shutdown();
    }
    
    // Cleanup
    plugin_host_.reset();
    event_bus_.reset();
    visualization_engine_.reset();
    playlist_manager_.reset();
    config_manager_.reset();
    service_registry_.reset();
    
    initialized_ = false;
    
    std::cout << "Core Engine shutdown complete" << std::endl;
}

Result CoreEngine::load_plugins(const char* plugin_dir) {
    if (!initialized_) {
        return Result::NotInitialized;
    }
    
    std::cout << "Loading plugins from: " << plugin_dir << std::endl;
    
    Result result = plugin_host_->scan_directory(plugin_dir);
    if (result != Result::Success) {
        std::cerr << "Failed to scan plugin directory" << std::endl;
        return result;
    }
    
    result = plugin_host_->initialize_plugins();
    if (result != Result::Success) {
        std::cerr << "Failed to initialize plugins" << std::endl;
        return result;
    }
    
    return Result::Success;
}

}} // namespace mp::core
