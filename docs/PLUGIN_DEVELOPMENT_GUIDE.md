# Qoder foobar v2.0 插件开发指南

## 目录
1. [概述](#概述)
2. [开发环境设置](#开发环境设置)
3. [插件架构](#插件架构)
4. [插件类型](#插件类型)
5. [开发步骤](#开发步骤)
6. [示例：音频解码器插件](#示例音频解码器插件)
7. [示例：DSP效果器插件](#示例dsp效果器插件)
8. [配置集成](#配置集成)
9. [调试和测试](#调试和测试)
10. [部署和分发](#部署和分发)
11. [最佳实践](#最佳实践)

## 概述

Qoder foobar v2.0 插件系统允许开发者扩展播放器的功能。它支持两种插件模式：
- **Native SDK**：现代化的C++17接口
- **foobar2000兼容层**：可直接加载现有foobar2000插件

### 特性
- **热插拔**：运行时加载/卸载插件
- **跨平台**：Windows/Linux/macOS支持
- **类型安全**：强类型的C++接口
- **版本兼容**：API版本检查机制
- **高性能**：零拷贝音频缓冲区
- **配置支持**：插件可定义自己的配置项
- **64位浮点支持**：专业音频处理精度

## 插件架构

```
┌─────────────────────────────────────────┐
│              Qoder foobar Core           │
├─────────────────────────────────────────┤
│           Plugin Manager                 │
├─────────────────────────────────────────┤
│  ┌─────────────┬─────────────────────┐   │
│  │ Native SDK   │ foobar2000 Adapter   │   │
│  │              │ Compatibility Layer │   │
│  └─────────────┴─────────────────────┘   │
└─────────────────────────────────────────┘
         │                │
    ┌────▼─────┐    ┌───▼─────┐
    │ Native   │    │ foobar  │
    │ Plugins  │    │ Plugins │
    └──────────┘    └─────────┘
```

## 开发环境设置

### 必需工具
- C++17兼容的编译器（GCC 7+, Clang 5+, MSVC 2017+）
- CMake 3.20+
- Make或Ninja

### 项目结构
```
my_plugin/
├── CMakeLists.txt          # 构建配置
├── my_plugin.cpp           # 插件实现
└── README.md               # 插件说明
```

### CMakeLists.txt模板
```cmake
cmake_minimum_required(VERSION 3.20)
project(my_plugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# 构建插件
add_library(my_plugin MODULE
    my_plugin.cpp
)

target_include_directories(my_plugin PRIVATE
    ${CMAKE_SOURCE_DIR}/sdk
)

target_compile_definitions(my_plugin PRIVATE
    QODER_PLUGIN_BUILD=1
)

# 输出设置
set_target_properties(my_plugin PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)

# 平台特定设置
if(WIN32)
    target_link_libraries(my_plugin PRIVATE)
else()
    target_link_libraries(my_plugin PRIVATE ${CMAKE_DL_LIBS})
endif()
```

## 插件类型

### 1. 音频解码器 (IAudioDecoder)
解码各种音频格式的插件。

### 2. DSP效果器 (IDSPProcessor)
处理音频信号的插件。

### 3. 音频输出 (IAudioOutput)
音频设备输出插件。

## 开发步骤

### 步骤1：创建插件类
继承相应的插件类型接口。

### 步骤2：实现插件接口
根据插件类型实现相应的方法。

### 步骤3：创建插件工厂
```cpp
class MyDecoderFactory : public ITypedPluginFactory<IAudioDecoder> {
public:
    std::unique_ptr<IAudioDecoder> create_typed() override {
        return std::make_unique<MyDecoder>();
    }

    PluginInfo get_info() const override {
        // 返回插件信息
    }
};
```

### 步骤4：导出插件
```cpp
QODER_EXPORT_AUDIO_PLUGIN(MyDecoder)
```

## 示例：音频解码器插件

```cpp
#include "qoder_plugin_sdk.h"
#include <fstream>

using namespace qoder;

class MyFormatDecoder : public IAudioDecoder {
private:
    std::ifstream file_;
    AudioFormat format_;
    bool is_open_;

public:
    bool initialize() override {
        set_state(PluginState::Initialized);
        return true;
    }

    PluginInfo get_info() const override {
        PluginInfo info;
        info.name = "My Format Decoder";
        info.version = "1.0.0";
        info.author = "Your Name";
        info.description = "Custom audio format decoder";
        info.type = PluginType::AudioDecoder;
        info.api_version = QODER_PLUGIN_API_VERSION;
        info.supported_formats = {"myfmt"};
        return info;
    }

    bool can_decode(const std::string& file_path) override {
        return file_path.ends_with(".myfmt");
    }

    std::vector<std::string> get_supported_extensions() override {
        return {"myfmt"};
    }

    bool open(const std::string& file_path) override {
        file_.open(file_path, std::ios::binary);
        if (!file_.is_open()) {
            return false;
        }

        format_.sample_rate = 44100;
        format_.channels = 2;
        format_.bits_per_sample = 16;
        format_.is_float = false;

        is_open_ = true;
        set_state(PluginState::Active);
        return true;
    }

    int decode(AudioBuffer& buffer, int max_frames) override {
        // 解码音频数据
        return frames_decoded;
    }

    void close() override {
        file_.close();
        is_open_ = false;
    }

    AudioFormat get_format() const override {
        return format_;
    }

    // ... 其他必需方法
};

// 工厂
class MyDecoderFactory : public ITypedPluginFactory<IAudioDecoder> {
public:
    std::unique_ptr<IAudioDecoder> create_typed() override {
        return std::make_unique<MyFormatDecoder>();
    }
};

// 导出
QODER_EXPORT_AUDIO_PLUGIN(MyFormatDecoder)
```

## 调试和测试

### 1. 编译插件
```bash
mkdir build && cd build
cmake ..
make
```

### 2. 测试插件
使用测试程序加载插件。

## 配置集成

插件可以定义自己的配置项，这些配置会被集成到主配置系统中。

### 插件配置示例

```cpp
class MyPlugin : public IAudioDecoder {
private:
    // 配置项
    int quality_mode_ = 2;
    bool enable_enhancement_ = true;
    double threshold_ = 0.5;

public:
    // 获取插件配置
    nlohmann::json get_config_schema() const override {
        return {
            {"quality_mode", {
                {"type", "integer"},
                {"min", 1},
                {"max", 5},
                {"default", 2},
                {"description", "Processing quality (1-5)"}
            }},
            {"enable_enhancement", {
                {"type", "boolean"},
                {"default", true},
                {"description", "Enable enhancement processing"}
            }},
            {"threshold", {
                {"type", "number"},
                {"min", 0.0},
                {"max", 1.0},
                {"default", 0.5},
                {"description", "Detection threshold"}
            }}
        };
    }

    // 应用配置
    void apply_config(const nlohmann::json& config) override {
        if (config.contains("quality_mode")) {
            quality_mode_ = config["quality_mode"];
        }
        if (config.contains("enable_enhancement")) {
            enable_enhancement_ = config["enable_enhancement"];
        }
        if (config.contains("threshold")) {
            threshold_ = config["threshold"];
        }
    }
};
```

### 配置文件中的插件设置

用户配置文件会自动包含插件配置：

```json
{
    "plugins": {
        "my_plugin": {
            "quality_mode": 3,
            "enable_enhancement": true,
            "threshold": 0.7
        }
    }
}
```

### 环境变量支持

插件配置也支持环境变量覆盖：

```bash
export QODER_PLUGIN_MY_PLUGIN_QUALITY_MODE=4
export QODER_PLUGIN_MY_PLUGIN_ENABLE_ENHANCEMENT=false
```

## 调试和测试

### 1. 编译测试

```bash
# 使用项目构建系统
cmake --build build --target my-plugin

# 测试插件加载
./build/bin/test-plugin
```

### 2. 调试技巧

#### 使用日志
```cpp
#include "core/logger.h"

void MyPlugin::process() {
    LOG_DEBUG("Processing frame: " << frame_count_);
    LOG_INFO("Plugin initialized successfully");
    LOG_ERROR("Failed to open file: " << file_path);
}
```

#### 错误状态管理
```cpp
bool MyPlugin::open(const std::string& file) {
    if (!file_exists(file)) {
        set_last_error("File not found: " + file);
        set_state(PluginState::Error);
        return false;
    }

    // 成功处理
    set_state(PluginState::Active);
    return true;
}
```

### 3. 单元测试

```cpp
#include <gtest/gtest.h>
#include "my_plugin.h"

TEST(MyPluginTest, CanDecodeValidFile) {
    MyPlugin plugin;
    ASSERT_TRUE(plugin.initialize());
    ASSERT_TRUE(plugin.open("test.wav"));

    AudioBuffer buffer;
    int frames = plugin.decode(buffer, 1024);
    EXPECT_GT(frames, 0);
}
```

## 部署和分发

### 1. 插件目录结构

```
my_plugin/
├── CMakeLists.txt          # 构建配置
├── my_plugin.cpp           # 插件实现
├── my_plugin.h             # 头文件（可选）
├── README.md               # 插件说明
├── LICENSE                 # 许可证
└── config/                 # 默认配置（可选）
    └── default.json
```

### 2. 构建脚本

```bash
#!/bin/bash
# build.sh
set -e

PLUGIN_NAME="my_plugin"
PLUGIN_VERSION="1.0.0"

# 创建构建目录
mkdir -p build
cd build

# 配置CMake
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DPLUGIN_NAME=${PLUGIN_NAME} \
    -DPLUGIN_VERSION=${PLUGIN_VERSION}

# 构建
cmake --build . -j$(nproc)

# 创建发布包
mkdir -p ../dist/${PLUGIN_NAME}
cp lib${PLUGIN_NAME}.* ../dist/${PLUGIN_NAME}/
cp ../README.md ../dist/${PLUGIN_NAME}/
cp ../LICENSE ../dist/${PLUGIN_NAME}/

echo "Plugin built successfully in dist/${PLUGIN_NAME}/"
```

### 3. 安装位置

插件会被自动加载以下目录：

```bash
# 用户插件目录
~/.qoder-foobar/plugins/

# 系统插件目录
/usr/lib/qoder-foobar/plugins/
/usr/local/lib/qoder-foobar/plugins/

# 开发目录
./plugins/
```

## 最佳实践

### 1. 性能优化

#### 音频处理优化
```cpp
// 使用SIMD（如果可用）
#ifdef __SSE2__
#include <emmintrin.h>

void process_audio_sse(float* data, int frames) {
    int aligned_frames = frames & ~3;

    // SIMD处理
    for (int i = 0; i < aligned_frames; i += 4) {
        __m128 input = _mm_load_ps(&data[i]);
        __m128 processed = _mm_mul_ps(input, _mm_set1_ps(0.5f));
        _mm_store_ps(&data[i], processed);
    }

    // 处理剩余样本
    for (int i = aligned_frames; i < frames; ++i) {
        data[i] *= 0.5f;
    }
}
#endif
```

#### 内存管理
```cpp
class MyPlugin : public IDSPProcessor {
private:
    std::vector<float> buffer_;  // 预分配缓冲区

public:
    bool configure(const AudioFormat& format) {
        // 预分配足够大的缓冲区
        buffer_.reserve(format.sample_rate);
        return true;
    }

    int process(float* input, float* output, int frames) {
        // 使用预分配的缓冲区
        std::copy(input, input + frames, buffer_.data());

        // 处理音频
        apply_processing(buffer_.data(), frames);

        // 输出结果
        std::copy(buffer_.data(), buffer_.data() + frames, output);
        return frames;
    }
};
```

### 2. 错误处理

#### RAII资源管理
```cpp
class FileHandler {
    std::FILE* file_ = nullptr;

public:
    FileHandler(const std::string& path) {
        file_ = fopen(path.c_str(), "rb");
        if (!file_) {
            throw std::runtime_error("Failed to open file: " + path);
        }
    }

    ~FileHandler() {
        if (file_) {
            fclose(file_);
        }
    }

    FILE* get() const { return file_; }
};
```

#### 异常安全
```cpp
bool MyPlugin::process() {
    try {
        FileHandler file(file_path_);
        // 使用文件...
        return true;
    } catch (const std::exception& e) {
        set_last_error(e.what());
        set_state(PluginState::Error);
        return false;
    }
}
```

### 3. 跨平台兼容性

#### 平台特定代码
```cpp
#ifdef _WIN32
    // Windows特定代码
    #include <windows.h>
#elif defined(__APPLE__)
    // macOS特定代码
    #include <CoreFoundation/CoreFoundation.h>
#else
    // Linux/Unix特定代码
    #include <unistd.h>
#endif
```

#### Unicode文件路径
```cpp
#ifdef _WIN32
    std::wstring utf8_to_wide(const std::string& utf8) {
        int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
        std::wstring result(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &result[0], size);
        return result;
    }
#endif

bool open_file(const std::string& path) {
#ifdef _WIN32
    std::wstring wpath = utf8_to_wide(path);
    FILE* file = _wfopen(wpath.c_str(), L"rb");
#else
    FILE* file = fopen(path.c_str(), "rb");
#endif

    if (!file) return false;
    fclose(file);
    return true;
}
```

### 4. 版本兼容性

```cpp
#define MY_PLUGIN_VERSION_MAJOR 1
#define MY_PLUGIN_VERSION_MINOR 2
#define MY_PLUGIN_VERSION_PATCH 0

// 版本检查
bool is_version_compatible(uint32_t host_version) {
    uint32_t plugin_version = QODER_MAKE_VERSION(
        MY_PLUGIN_VERSION_MAJOR,
        MY_PLUGIN_VERSION_MINOR,
        MY_PLUGIN_VERSION_PATCH
    );

    // 检查主版本兼容性
    return QODER_VERSION_MAJOR(host_version) == MY_PLUGIN_VERSION_MAJOR &&
           QODER_VERSION_MINOR(host_version) >= MY_PLUGIN_VERSION_MINOR;
}
```

## FAQ

### Q: 如何调试插件加载问题？
A: 检查以下内容：
- 插件是否导出了正确的函数
- 依赖库是否可用
- 插件是否与主机架构匹配

### Q: 插件可以使用第三方库吗？
A: 可以，但需要注意：
- 静态链接以避免依赖问题
- 动态链接需要在CMakeLists.txt中正确配置
- 遵守库的许可证要求

### Q: 如何发布插件？
A: 建议提供：
- 编译好的二进制文件
- 源代码
- 构建脚本
- 详细文档
- 示例配置

---

*更多问题请访问项目Wiki或提交Issue。*
