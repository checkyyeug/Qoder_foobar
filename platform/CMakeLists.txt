# Platform abstraction library
add_library(platform_abstraction STATIC
    linux/audio_output_alsa.cpp
)

# Find ALSA library
find_package(ALSA)
if(NOT ALSA_FOUND)
    message(WARNING "ALSA not found, audio output will be stub only")
    target_compile_definitions(platform_abstraction PRIVATE NO_ALSA)
endif()

target_include_directories(platform_abstraction
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/sdk/headers
)

if(ALSA_FOUND)
    target_link_libraries(platform_abstraction
        PUBLIC
            sdk_headers
            ${ALSA_LIBRARIES}
    )
    target_include_directories(platform_abstraction PRIVATE ${ALSA_INCLUDE_DIRS})
else()
    target_link_libraries(platform_abstraction
        PUBLIC
            sdk_headers
    )
endif()

# Set compile options
target_compile_options(platform_abstraction PRIVATE
    -Wall -Wextra
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
)
