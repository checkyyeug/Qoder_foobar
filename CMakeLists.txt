cmake_minimum_required(VERSION 3.20)
project(MusicPlayer VERSION 0.1.0 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build configuration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# Options
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_PLUGINS "Build bundled plugins" ON)
option(ENABLE_FOOBAR_COMPAT "Enable foobar2000 plugin compatibility" ON)
option(ENABLE_GPU "Enable GPU acceleration" ON)

# Platform detection
if(WIN32)
    set(PLATFORM_NAME "windows")
elseif(APPLE)
    set(PLATFORM_NAME "macos")
elseif(UNIX)
    set(PLATFORM_NAME "linux")
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/sdk/headers
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/platform
)

# Add subdirectories
add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(sdk)

if(BUILD_PLUGINS)
    add_subdirectory(plugins)
endif()

if(ENABLE_GPU)
    add_subdirectory(gpu)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Main executable
add_executable(music-player
    src/main.cpp
)

target_link_libraries(music-player
    PRIVATE
        core_engine
        platform_abstraction
        Threads::Threads
)

# Test WASAPI executable
add_executable(test-wasapi
    src/test_wasapi.cpp
)

target_link_libraries(test-wasapi
    PRIVATE
        platform_abstraction
        Threads::Threads
)

# Install targets
install(TARGETS music-player
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_BINARY_DIR}/lib/
    DESTINATION lib/music-player/plugins
    FILES_MATCHING PATTERN "*.so" PATTERN "*.dylib" PATTERN "*.dll"
)
