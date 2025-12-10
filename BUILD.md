# Build Guide

This guide provides detailed instructions for building the Professional Music Player on different platforms.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Quick Build](#quick-build)
- [Platform-Specific Instructions](#platform-specific-instructions)
  - [Linux](#linux)
  - [Windows](#windows)
  - [macOS](#macos)
- [Build Options](#build-options)
- [Testing](#testing)
- [Troubleshooting](#troubleshooting)

## Prerequisites

### Common Requirements

- **CMake 3.20 or higher**
- **C++17 compatible compiler**
  - GCC 7.0+ or Clang 5.0+ on Linux/macOS
  - Visual Studio 2017+ on Windows
- **Git** (for cloning)

### Platform-Specific Requirements

#### Linux
```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake pkg-config

# CentOS/RHEL/Fedora
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake pkgconfig
```

#### macOS
```bash
# Install Xcode command line tools
xcode-select --install

# Or install Homebrew and use it
brew install cmake
```

#### Windows
- Install Visual Studio 2017 or newer
- Ensure C++ development tools are installed
- CMake is included with Visual Studio or can be installed separately

## Quick Build

### For Testing/Development (Stub Mode)

The project can be built without any audio libraries for testing:

```bash
# Clone
git clone <repository-url>
cd Qoder_foobar

# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j4

# Run tests
./build/bin/test_cross_platform
```

This will work on any platform, even without audio libraries installed.

### For Full Features

Install audio libraries first (see platform-specific sections), then build:

```bash
# After installing dependencies
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
```

## Platform-Specific Instructions

### Linux

#### Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential cmake pkg-config \
    libasound2-dev libpulse-dev \
    libflac-dev libmp3lame-dev \
    libvorbis-dev libogg-dev \
    libopenmp-dev
```

**CentOS/RHEL 8+:**
```bash
sudo dnf install -y \
    gcc-c++ cmake pkgconfig \
    alsa-lib-devel pulseaudio-libs-devel \
    flac-devel libmp3lame-devel \
    libvorbis-devel libogg-devel \
    libgomp-devel
```

**Fedora:**
```bash
sudo dnf install -y \
    gcc-c++ cmake pkgconfig \
    alsa-lib-devel pulseaudio-libs-devel \
    flac-devel lame-devel \
    libvorbis-devel libogg-devel \
    libgomp-devel SDL2-devel
```

**Arch Linux:**
```bash
sudo pacman -S --needed \
    base-devel cmake pkgconf \
    alsa-lib pulseaudio \
    flac lame \
    libvorbis libogg \
    openmp sdl2
```

#### Build Commands

```bash
# Standard build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Debug build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)

# With specific options
cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON \
    -DENABLE_OPENMP=ON
cmake --build build -j$(nproc)
```

#### Audio System Setup

```bash
# Check audio devices
aplay -l
cat /proc/asound/cards

# Test audio
speaker-test -c 2 -t wav

# Add user to audio group (if needed)
sudo usermod -a -G audio $USER
# Then log out and log back in
```

### Windows

#### Prerequisites

1. Install Visual Studio 2017 or newer with C++ support
2. Install Git for Windows
3. Install CMake (or use Visual Studio's version)

#### Build with Visual Studio

```cmd
# Clone repository
git clone <repository-url>
cd Qoder_foobar

# Configure for Visual Studio
cmake -G "Visual Studio 17 2022" -A x64 -B build

# Build
cmake --build build --config Release

# Run tests
build\bin\Release\test_cross_platform.exe
```

#### Build with MSBuild

```cmd
# After configuration
cmake -G "Visual Studio 17 2022" -A x64 -B build

# Build all
cmake --build build --config Release -j4

# Build specific target
cmake --build build --config Release --target music-player
```

#### VCPKG Dependencies (Optional)

For additional format support:

```cmd
# Install vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

# Install packages
.\vcpkg install flac:x64-windows
.\vcpkg install libmp3lame:x64-windows

# Configure with vcpkg
cmake -G "Visual Studio 17 2022" -A x64 `
    -B build `
    -DCMAKE_TOOLCHAIN_FILE=path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
```

### macOS

#### Prerequisites

```bash
# Install Xcode command line tools
xcode-select --install

# Or install Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies with Homebrew
brew install cmake pkg-config
```

#### Install Audio Libraries

```bash
# CoreAudio is built into macOS
# Install additional libraries
brew install flac lame libvorbis libogg

# For parallel processing
brew install libomp
```

#### Build Commands

```bash
# Standard build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(sysctl -n hw.logicalcpu)

# With Xcode
cmake -G Xcode -B build
xcodebuild -project build/MusicPlayer.xcodeproj -configuration Release

# For universal binaries (Apple Silicon + Intel)
cmake -B build -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
```

## Build Options

### CMake Options

```bash
# Build type
cmake -DCMAKE_BUILD_TYPE=Release  # or Debug, RelWithDebInfo, MinSizeRel

# Component selection
cmake -DBUILD_TESTS=ON          # Build unit tests
cmake -DBUILD_PLUGINS=ON        # Build plugin ecosystem
cmake -DENABLE_OPENMP=ON        # Enable OpenMP parallelization

# Platform-specific
cmake -DENABLE_FOOBAR_COMPAT=ON  # foobar2000 compatibility
cmake -DENABLE_GPU=ON            # GPU acceleration (future)
cmake -DENABLE_UI=ON             # UI components (future)
```

### Build Targets

```bash
# Build everything
cmake --build build

# Build specific target
cmake --build build --target music-player
cmake --build build --target test_cross_platform

# Install (after build)
cmake --install build  # May require sudo
```

### Parallel Builds

```bash
# Linux/macOS
cmake --build build -j$(nproc)  # Linux
cmake --build build -j$(sysctl -n hw.logicalcpu)  # macOS

# Windows (MSBuild)
cmake --build build --parallel 4
```

## Testing

### Run All Tests

```bash
# Platform detection test
./build/bin/test_cross_platform

# WAV file test
./build/bin/final_wav_player test.wav

# Audio backend test
./build/bin/test_audio_direct
```

### Create Test Files

```bash
# Python script to create test WAV
cat > create_test_wav.py << 'EOF'
import struct
import math

# Create a simple WAV file
sample_rate = 44100
duration = 2.0
frequency = 440.0

with open('test_440hz.wav', 'wb') as f:
    # Header
    f.write(b'RIFF')
    f.write(struct.pack('<I', 36 + int(sample_rate * duration * 4)))
    f.write(b'WAVE')

    # Format chunk
    f.write(b'fmt ')
    f.write(struct.pack('<I', 16))
    f.write(struct.pack('<H', 1))  # PCM
    f.write(struct.pack('<H', 2))  # Stereo
    f.write(struct.pack('<I', sample_rate))
    f.write(struct.pack('<I', sample_rate * 4))
    f.write(struct.pack('<H', 4))
    f.write(struct.pack('<H', 16))

    # Data chunk
    f.write(b'data')
    f.write(struct.pack('<I', int(sample_rate * duration * 4)))

    # Audio data
    for i in range(int(sample_rate * duration)):
        t = i / sample_rate
        value = int(32767 * math.sin(2 * math.pi * frequency * t))
        f.write(struct.pack('<hh', value, value))
EOF

python3 create_test_wav.py
./build/bin/final_wav_player test_440hz.wav
```

### Expected Test Output

```
=== Cross-Platform Music Player ===

Platform: Linux
Architecture: x64
Compiler: GCC
Audio Backend: ALSA

Audio Backend Testing:
Available backends: auto, stub, alsa

Testing backend 'auto': ✓ Created successfully
Testing backend 'alsa': ✓ Created successfully
```

## Troubleshooting

### Common Build Errors

1. **CMake version too old**
   ```
   CMake Error: CMake 3.20 or higher is required
   ```
   Solution: Install newer CMake or use system package manager.

2. **Compiler not found**
   ```
   No CMAKE_CXX_COMPILER could be found
   ```
   Solution: Install build-essential (Linux) or Visual Studio (Windows).

3. **ALSA not found (Linux)**
   ```
   ALSA not found - using stub audio
   ```
   Solution: Install libasound2-dev (see Linux instructions).

4. **Linker errors on Windows**
   ```
   unresolved external symbol __imp_*
   ```
   Solution: Ensure Visual Studio C++ tools are installed.

### Runtime Issues

1. **Permission denied accessing audio**
   ```bash
   # Add user to audio group
   sudo usermod -a -G audio $USER
   # Then re-login
   ```

2. **Audio not playing**
   - Try stub backend: `--backend stub`
   - Check audio device permissions
   - Verify audio system is running

3. **Plugin loading fails**
   - Check LD_LIBRARY_PATH (Linux)
   - Verify DLL search path (Windows)
   - Check DYLD_LIBRARY_PATH (macOS)

### Clean Build

```bash
# Remove all build artifacts
rm -rf build CMakeCache.txt CMakeFiles/

# Or clean specific target
cmake --build build --target clean
```

## Performance Optimization

### Compiler Optimizations

```bash
# Release mode with optimizations
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS_RELEASE="-O3 -DNDEBUG -march=native" \
      -B build
```

### Link-Time Optimization (LTO)

```bash
cmake -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON -B build
```

### Profile-Guided Optimization (PGO)

```bash
# 1. Build with profiling
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DCMAKE_CXX_FLAGS="-fprofile-generate" \
      -B build
cmake --build build

# 2. Run representative workload
./build/bin/music-player test_files/

# 3. Rebuild with profile
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-fprofile-use" \
      -B build_pgo
cmake --build build_pgo
```

## Continuous Integration

### GitHub Actions Example

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]

    runs-on: ${{ matrix.os }}

    steps:
    - uses: actions/checkout@v2

    - name: Configure
      run: cmake -B build -DCMAKE_BUILD_TYPE=Release

    - name: Build
      run: cmake --build build --parallel 4

    - name: Test
      run: ./build/bin/test_cross_platform  # Linux/macOS
      # or build/bin/Release/test_cross_platform.exe  # Windows
```

## Next Steps

After successful build:

1. Read the [Plugin Development Guide](docs/PLUGIN_DEVELOPMENT_GUIDE.md)
2. Check [Audio Pipeline Specification](docs/AUDIO_PIPELINE.md)
3. Explore the [Implementation Progress](IMPLEMENTATION_PROGRESS.md)