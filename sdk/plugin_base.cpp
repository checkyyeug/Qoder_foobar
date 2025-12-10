#include "qoder_plugin_sdk.h"

namespace qoder {

PluginState IPlugin::get_state() const {
    return state_;
}

void IPlugin::set_state(PluginState state) {
    state_ = state;
}

} // namespace qoder