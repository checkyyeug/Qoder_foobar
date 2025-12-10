# 🌐 跨平台构建指南

## 概述

Qoder foobar 音乐播放器支持跨平台构建，自动检测并使用最佳音频后端：

- **Linux**: ALSA → PulseAudio → JACK → Stub
- **Windows**: WASAPI → Stub
- **macOS**: CoreAudio → Stub

---

## 🐧 Linux 构建

### 系统要求
- Ubuntu 18.04+ / Debian 10+ / CentOS 7+ / Fedora 30+
- CMake 3.20+
- GCC 9+ or Clang 10+

### 安装依赖

#### Ubuntu/Debian
```bash
# 基础构建工具
sudo apt-get update
sudo apt-get install -y cmake build-essential pkg-config

# 音频库 (二选一)
# 选项1: ALSA (推荐)
sudo apt-get install -y libasound2-dev
# 选项2: PulseAudio
sudo apt-get install -y libpulse-dev
```

#### CentOS/RHEL/Fedora
```bash
# 基础构建工具
sudo yum groupinstall -y "Development Tools"
sudo yum install -y cmake

# 音频库
# CentOS/RHEL
sudo yum install -y alsa-lib-devel pulseaudio-libs-devel

# Fedora
sudo dnf install -y alsa-lib-devel pulseaudio-libs-devel
```

#### Arch Linux
```bash
sudo pacman -S --needed cmake base-devel
sudo pacman -S alsa-lib pulseaudio
```

### 构建步骤
```bash
git clone https://github.com/checkyyeug/Qoder_foobar.git
cd Qoder_foobar

cmake -B build
cmake --build build

# 运行
./build/bin/music-player
```

---

## 🪟 Windows 构建

### 系统要求
- Windows 10 或更高版本
- Visual Studio 2019 或更高版本 (Community Edition 可用)
- CMake 3.20+

### 使用 Visual Studio 构建

1. **安装 Visual Studio**
   - 下载 [Visual Studio Community](https://visualstudio.microsoft.com/vs/community/)
   - 安装时选择 "Desktop development with C++"

2. **安装 CMake**
   - 下载 [CMake](https://cmake.org/download/)
   - 或使用 Visual Studio Installer 中的 CMake 组件

3. **构建步骤**
   ```cmd
   git clone https://github.com/checkyyeug/Qoder_foobar.git
   cd Qoder_foobar

   mkdir build
   cd build

   # 使用 CMake 生成 Visual Studio 项目
   cmake .. -G "Visual Studio 16 2019" -A x64

   # 使用 Visual Studio 或命令行构建
   cmake --build . --config Release
   ```

4. **运行**
   ```cmd
   cd Release
   music-player.exe
   ```

### 使用 MSYS2/MinGW

1. **安装 MSYS2**
   - 下载 [MSYS2](https://www.msys2.org/)
   - 运行安装程序

2. **安装工具链**
   ```bash
   pacman -Syu
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake
   ```

3. **构建**
   ```bash
   git clone https://github.com/checkyyeug/Qoder_foobar.git
   cd Qoder_foobar

   cmake -B build -G "MSYS Makefiles"
   cmake --build build
   ```

---

## 🍎 macOS 构建

### 系统要求
- macOS 10.15 Catalina 或更高版本
- Xcode 12 或更高版本
- CMake 3.20+

### 使用 Xcode 构建

1. **安装 Xcode**
   - 从 App Store 安装 Xcode
   - 安装 Command Line Tools:
     ```bash
     xcode-select --install
     ```

2. **安装 Homebrew** (可选)
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

3. **构建步骤**
   ```bash
   git clone https://github.com/checkyyeug/Qoder_foobar.git
   cd Qoder_foobar

   mkdir build
   cd build

   # 配置
   cmake ..

   # 构建
   cmake --build .

   # 运行
   ./music-player
   ```

### 使用 Makefile 构建

```bash
# 简单构建
cmake -B build
cd build
make

# 或者使用 Ninja (更快)
cmake -B build -G Ninja
cd build
ninja
```

---

## 🔧 构建选项

### 强制使用特定音频后端

```bash
cmake -B build -DAUDIO_BACKEND_WASAPI=ON   # Windows
cmake -B build -DAUDIO_BACKEND_ALSA=ON     # Linux
cmake -B build -DAUDIO_BACKEND_PULSE=ON    # Linux
cmake -B build -DAUDIO_BACKEND_STUB=ON     # 所有平台 (无音频)
```

### 构建类型

```bash
# Debug 构建 (包含调试信息)
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Release 构建 (优化版本)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# MinSizeRel 构建 (最小体积)
cmake -B build -DCMAKE_BUILD_TYPE=MinSizeRel
```

---

## 📝 构建输出示例

### Linux 成功输出
```
-- Audio Backend Detection:
--   → ALSA audio output
-- Selected audio backend: ALSA
-- Building with audio backend: ALSA
[100%] Built target music-player
```

### Windows 成功输出
```
-- Audio Backend Detection:
--   → WASAPI audio output
-- Selected audio backend: WASAPI
-- Building with audio backend: WASAPI
[100%] Built target music-player.exe
```

### macOS 成功输出
```
-- Audio Backend Detection:
--   → CoreAudio audio output
-- Selected audio backend: CoreAudio
-- Building with audio backend: CoreAudio
[100%] Built target music-player
```

---

## 🚨 故障排除

### Linux: 找不到 ALSA
```bash
# Ubuntu/Debian
sudo apt-get install libasound2-dev

# CentOS/RHEL/Fedora
sudo yum install alsa-lib-devel
```

### Windows: 找不到库
- 确保安装了 Windows SDK
- 检查 Visual Studio 是否正确安装

### macOS: 找不到 CoreAudio
- CoreAudio 是系统框架，应该总是可用
- 确保 Xcode Command Line Tools 已安装

### 通用问题
```bash
# 清理构建
rm -rf build

# 重新配置
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 详细输出
cmake -B build -DCMAKE_VERBOSE_MAKEFILE=ON
```

---

## 🎯 测试音频输出

构建成功后，可以测试音频是否正常工作：

```bash
# Linux
./build/bin/music-player test_file.wav

# Windows
.\build\Release\music-player.exe test_file.wav

# macOS
./build/music-player test_file.wav
```

输入 `help` 查看可用命令，输入 `play` 开始播放。

---

## 📊 支持的平台矩阵

| 平台 | 编译器 | 音频后端 | 状态 |
|------|--------|----------|------|
| Linux | GCC/Clang | ALSA/PulseAudio/JACK | ✅ 完全支持 |
| Windows | MSVC/MinGW | WASAPI | ✅ 完全支持 |
| macOS | Clang | CoreAudio | ✅ 完全支持 |

---

## 🏆 总结

Qoder foobar 的跨平台构建系统设计简单易用：

1. **零配置**: 自动检测平台和音频后端
2. **统一接口**: 所有平台使用相同的 API
3. **优雅降级**: 无音频库时使用 Stub 后端
4. **易于维护**: 模块化设计，易于扩展

在支持的平台上，只需简单的几步即可构建出功能完整的音乐播放器！