#include "audio_output_factory.h"

namespace mp {
namespace platform {

// Forward declarations of platform-specific factory functions
#ifdef _WIN32
extern "C" IAudioOutput* create_wasapi_output();
#elif defined(__APPLE__)
extern "C" IAudioOutput* create_coreaudio_output();
#elif defined(__linux__)
extern "C" IAudioOutput* create_alsa_output();
#endif

IAudioOutput* create_platform_audio_output() {
#ifdef _WIN32
    return create_wasapi_output();
#elif defined(__APPLE__)
    return create_coreaudio_output();
#elif defined(__linux__)
    return create_alsa_output();
#else
    #error "Unsupported platform"
    return nullptr;
#endif
}

}} // namespace mp::platform
