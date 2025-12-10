# 音频后端自动检测系统

本系统实现了跨平台的音频后端自动检测，无需手动配置即可在 Windows、macOS 和 Linux 上使用最佳音频后端。

## 功能特性

### 🎯 自动检测优先级

**Windows:**
1. WASAPI (Windows Audio Session API) - 首选
2. WinMM (Windows Multimedia) - 备选
3. Stub - 永远可用

**macOS:**
1. CoreAudio - 首选
2. AudioToolbox - 集成
3. Stub - 永远可用

**Linux:**
1. ALSA (Advanced Linux Sound Architecture) - 首选
2. PulseAudio - 次选
3. JACK Audio Connection Kit - 三选
4. Stub - 永远可用

**其他 Unix-like:**
1. PulseAudio - 优先
2. Stub - 永远可用

### 🔄 优雅降级链

- 自动按优先级检测
- 找不到后端自动使用下一个
- 始终有 Stub 作为保底
- 编译时确定，运行时无需检测

## 文件结构

```
Qoder_foobar/
├── cmake/
│   └── AudioBackend.cmake     ← 自动检测模块
├── src/audio/
│   ├── audio_output.h        ← 统一接口
│   ├── audio_output.cpp      ← 工厂实现
│   ├── audio_output_stub.cpp   ← Stub 实现 (永远可用)
│   ├── audio_output_wasapi.cpp ← Windows 实现
│   ├── audio_output_coreaudio.cpp ← macOS 实现
│   ├── audio_output_alsa.cpp     ← Linux ALSA
│   ├── audio_output_pulse.cpp    ← Linux PulseAudio
│   └── main.cpp               ← 测试程序
└── CMakeLists.txt
```

## 使用方法

### 1. 在 CMakeLists.txt 中集成

```cmake
# 包含自动检测模块
include(cmake/AudioBackend.cmake)

# 添加到你的可执行文件
target_sources(your_target PRIVATE ${AUDIO_SOURCES})
target_link_libraries(your_target PRIVATE ${AUDIO_LIBS})
if(AUDIO_INCLUDE_DIRS)
    target_include_directories(your_target PRIVATE ${AUDIO_INCLUDE_DIRS})
endif()
```

### 2. 在代码中使用

```cpp
#include "src/audio/audio_output.h"

// 创建音频输出（自动选择最佳后端）
auto audio = audio::create_audio_output();

// 检查使用的后端
std::cout << "Using backend: " << audio::get_audio_backend_name() << std::endl;

// 使用统一的接口
audio::AudioFormat format;
format.sample_rate = 44100;
format.channels = 2;
format.bits_per_sample = 16;

if (audio->open(format)) {
    audio->write(buffer, frames);
    audio->close();
}
```

## 检测结果示例

### Linux with ALSA
```
Audio Backend Detection:
  → ALSA audio output
Selected audio backend: ALSA
```

### Linux without ALSA but with PulseAudio
```
Audio Backend Detection:
  → PulseAudio audio output
Selected audio Backend: PulseAudio
```

### macOS
```
Audio Backend Detection:
  → CoreAudio audio output
Selected audio backend: CoreAudio
```

### Windows
```
Audio Backend Detection:
  → WASAPI audio output
Selected audio backend: WASAPI
```

### No audio support
```
Audio Backend Detection:
  → Using stub audio backend
Selected audio backend: stub
```

## 构建示例

### 自动构建（推荐）
```bash
# 从源码构建
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 运行测试
./build/test_audio test.wav
```

### 输出示例
```
-- Audio Backend Detection:
--   → ALSA audio output
-- Selected audio backend: ALSA
-- Audio sources: src/audio/audio_output_stub.cpp;src/audio/audio_output_alsa.cpp
-- Audio libraries: /usr/lib/x86_64-linux-gnu/libasound.so.2
-- Audio includes: /usr/include/alsa

Detected audio backend: ALSA
WAV File Information:
  Sample Rate: 44100 Hz
  Channels: 2
  Bits: 16-bit
  Data Size: 352800 bytes

Loaded 176400 audio samples

Testing audio backend...
✓ Audio device opened successfully
  Buffer Size: 1024 frames
  Latency: 100 ms

Processing audio for 5 seconds
.....

✓ Audio device closed
Test completed successfully!

Audio backend 'ALSA' provides real audio output capability
```

## 依赖安装

### Linux
```bash
# 安装 ALSA (推荐)
sudo apt-get update
sudo apt-get install libasound2-dev

# 或安装 PulseAudio
sudo apt-get update
sudo apt-get install libpulse-dev

# 或两者都安装
sudo apt-get update
sudo apt-get install libasound2-dev libpulse-dev
```

### macOS
```bash
# CoreAudio 已内建，无需额外安装
# 但确保有 Xcode 命令行工具
xcode-select --install
```

### Windows
```bash
# Visual Studio 包含所有必需库
# 无需额外安装
```

## 扩展新后端

### 添加新的音频后端

1. 创建实现文件，例如 `src/audio/audio_output_newbackend.cpp`

2. 在 `AudioBackend.cmake` 中添加检测逻辑：

```cmake
elseif(NEW_BACKEND_CONDITION)
    find_package(NEW_BACKEND_LIB)
    if(NEW_BACKEND_LIB_FOUND)
        add_audio_backend("NewBackend" "audio_output_newbackend.cpp" "NEW_BACKEND_LIB" "NEW_BACKEND_INCLUDE")
    endif()
```

### 编译时选择特定后端

```bash
cmake -DAUDIO_BACKEND_WASAPI=ON  # 强制使用 WASAPI
cmake -DAUDIO_BACKEND_ALSA=ON      # 强制使用 ALSA
```

## 性能特性

- **零运行时检测开销**：后端在编译时确定
- **最小二进制大小**：只包含选中的后端
- **最优性能**：使用平台原生 API
- **完全可移植**：所有平台统一接口

## 故障排除

### 检测失败
```bash
# 查看 CMake 输出
cmake .. 2>&1 | grep -A 10 "Audio Backend Detection"

# 检查编译定义
make VERBOSE=1 2>&1 | grep AUDIO_BACKEND
```

### 音频不工作
```bash
# 检查后端类型
./test_audio 2>&1 | grep "Selected audio backend"

# Linux 特定检查
pacmd list-cards
aplay -l
```

### 编译错误
```bash
# 清理并重新配置
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Release
```

## 总结

这个音频后端自动检测系统实现了：
- ✅ **完全自动化**：无需手动配置
- ✅ **跨平台**：支持所有主流操作系统
- ✅ **优雅降级**：永远有 Stub 作为保底
- ✅ **性能优化**：使用原生平台 API
- ✅ **易于扩展**：添加新后端只需几个步骤