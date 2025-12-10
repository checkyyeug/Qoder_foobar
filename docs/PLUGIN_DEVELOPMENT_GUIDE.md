# Qoder foobar 插件开发指南

## 目录
1. [概述](#概述)
2. [开发环境设置](#开发环境设置)
3. [插件类型](#插件类型)
4. [开发步骤](#开发步骤)
5. [示例：音频解码器插件](#示例音频解码器插件)
6. [示例：DSP效果器插件](#示例dsp效果器插件)
7. [调试和测试](#调试和测试)
8. [部署和分发](#部署和分发)
9. [最佳实践](#最佳实践)

## 概述

Qoder foobar 插件系统允许开发者扩展播放器的功能。插件是动态加载的模块，可以实现音频解码、DSP效果、可视化等功能。

### 特性
- **热插拔**：运行时加载/卸载插件
- **跨平台**：Windows/Linux/macOS支持
- **类型安全**：强类型的C++接口
- **版本兼容**：API版本检查机制
- **高性能**：零拷贝音频缓冲区

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

### 3. 调试技巧
- 使用`set_state(PluginState::Error)`标记错误
- 通过`get_last_error()`获取错误信息
- 使用日志库记录调试信息

## 部署和分发

### 1. 编译发布版本
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### 2. 插件安装
将生成的.so/.dll文件复制到Qoder foobar的plugins目录。

## 最佳实践

### 1. 性能优化
- 使用SIMD指令优化音频处理
- 避免不必要的内存分配
- 批量处理音频数据

### 2. 错误处理
- 总是检查返回值
- 提供详细的错误信息
- 在错误时清理资源

### 3. 资源管理
- 使用RAII管理资源
- 及时释放文件句柄
- 避免内存泄漏

---

*更多问题请访问项目Wiki或提交Issue。*
