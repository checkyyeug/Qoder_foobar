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

# Set compile options (platform-specific)
if(MSVC)
    target_compile_options(plugin_wav_decoder PRIVATE /W4)
else()
    target_compile_options(plugin_wav_decoder PRIVATE
        -Wall -Wextra
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3>
    )
endif()

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

# Set compile options (platform-specific)
if(MSVC)
    target_compile_options(plugin_flac_decoder PRIVATE /W4)
else()
    target_compile_options(plugin_flac_decoder PRIVATE
        -Wall -Wextra
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3>
    )
endif()

set_target_properties(plugin_flac_decoder PROPERTIES
    PREFIX ""
    OUTPUT_NAME "flac_decoder"
)

# MP3 Decoder Plugin (using minimp3 header-only library)
add_library(plugin_mp3_decoder SHARED
    mp3_decoder.cpp
)

target_include_directories(plugin_mp3_decoder
    PRIVATE
        ${CMAKE_SOURCE_DIR}/sdk/headers
        ${CMAKE_SOURCE_DIR}/sdk/external/minimp3
)

target_link_libraries(plugin_mp3_decoder
    PRIVATE
        sdk_headers
)

# Set compile options (platform-specific)
if(MSVC)
    target_compile_options(plugin_mp3_decoder PRIVATE /W4)
else()
    target_compile_options(plugin_mp3_decoder PRIVATE
        -Wall -Wextra
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3>
    )
endif()

set_target_properties(plugin_mp3_decoder PROPERTIES
    PREFIX ""
    OUTPUT_NAME "mp3_decoder"
)

message(STATUS "Building MP3 decoder plugin with minimp3 (header-only library)")
