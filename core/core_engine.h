#pragma once

#include "service_registry.h"
#include "event_bus.h"
#include "plugin_host.h"
#include "config_manager.h"
#include <memory>
#include <string>

namespace mp {
namespace core {

// Core engine - orchestrates all components
class CoreEngine {
public:
    CoreEngine();
    ~CoreEngine();
    
    // Initialize core engine
    Result initialize();
    
    // Shutdown core engine
    void shutdown();
    
    // Load plugins from directory
    Result load_plugins(const char* plugin_dir);
    
    // Get service registry
    ServiceRegistry* get_service_registry() {
        return service_registry_.get();
    }
    
    // Get event bus
    EventBus* get_event_bus() {
        return event_bus_.get();
    }
    
    // Get plugin host
    PluginHost* get_plugin_host() {
        return plugin_host_.get();
    }
    
    // Get config manager
    ConfigManager* get_config_manager() {
        return config_manager_.get();
    }
    
    // Check if initialized
    bool is_initialized() const {
        return initialized_;
    }
    
private:
    std::unique_ptr<ServiceRegistry> service_registry_;
    std::unique_ptr<EventBus> event_bus_;
    std::unique_ptr<PluginHost> plugin_host_;
    std::unique_ptr<ConfigManager> config_manager_;
    
    bool initialized_;
};

}} // namespace mp::core
