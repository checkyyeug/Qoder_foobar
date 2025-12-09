# WAV Decoder Plugin
add_library(plugin_wav_decoder SHARED
    wav_decoder.cpp
)

target_include_directories(plugin_wav_decoder
    PRIVATE
        ${CMAKE_SOURCE_DIR}/sdk/headers
)

target_link_libraries(plugin_wav_decoder
    PRIVATE
        sdk_headers
)

# Set compile options
target_compile_options(plugin_wav_decoder PRIVATE
    -Wall -Wextra
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
)

# Don't add 'lib' prefix to plugin
set_target_properties(plugin_wav_decoder PROPERTIES
    PREFIX ""
    OUTPUT_NAME "wav_decoder"
)

# FLAC Decoder Plugin
# Try to find FLAC library
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(FLAC flac)
endif()

add_library(plugin_flac_decoder SHARED
    flac_decoder.cpp
)

target_include_directories(plugin_flac_decoder
    PRIVATE
        ${CMAKE_SOURCE_DIR}/sdk/headers
)

if(FLAC_FOUND)
    target_link_libraries(plugin_flac_decoder
        PRIVATE
            sdk_headers
            ${FLAC_LIBRARIES}
    )
    target_include_directories(plugin_flac_decoder PRIVATE ${FLAC_INCLUDE_DIRS})
    target_compile_definitions(plugin_flac_decoder PRIVATE)
    message(STATUS "FLAC library found, building FLAC decoder plugin")
else()
    target_link_libraries(plugin_flac_decoder
        PRIVATE
            sdk_headers
    )
    target_compile_definitions(plugin_flac_decoder PRIVATE NO_FLAC)
    message(WARNING "FLAC library not found, FLAC decoder will be stub only")
endif()

target_compile_options(plugin_flac_decoder PRIVATE
    -Wall -Wextra
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
)

set_target_properties(plugin_flac_decoder PROPERTIES
    PREFIX ""
    OUTPUT_NAME "flac_decoder"
)
