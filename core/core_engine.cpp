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
    
    // Create plugin host
    plugin_host_ = std::make_unique<PluginHost>(service_registry_.get());
    
    // Register core services
    service_registry_->register_service(SERVICE_EVENT_BUS, event_bus_.get());
    service_registry_->register_service(SERVICE_PLUGIN_HOST, plugin_host_.get());
    service_registry_->register_service(SERVICE_CONFIG_MANAGER, config_manager_.get());
    
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
    
    // Cleanup
    plugin_host_.reset();
    event_bus_.reset();
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
