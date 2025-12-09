#pragma once

#include "mp_audio_output.h"

namespace mp {
namespace platform {

// Factory function to create platform-specific audio output
// Returns the appropriate audio output implementation for the current platform:
// - Windows: WASAPI
// - macOS: CoreAudio
// - Linux: ALSA
IAudioOutput* create_platform_audio_output();

}} // namespace mp::platform
