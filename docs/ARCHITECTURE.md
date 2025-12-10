# Qoder foobar v2.0 系统架构文档

## 概述

Qoder foobar v2.0 是一个现代化的、模块化的音乐播放器系统，采用了分层架构设计，支持高性能音频处理、插件扩展和跨平台部署。

### 设计原则

1. **模块化**：每个功能模块独立，便于维护和扩展
2. **可配置性**：通过JSON配置文件和环境变量灵活控制系统行为
3. **高性能**：优化的音频处理管道，支持32/64位浮点精度
4. **可扩展性**：插件系统支持动态加载和热插拔
5. **兼容性**：支持原生SDK和foobar2000插件兼容层

## 系统架构

### 整体架构图

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              Qoder foobar v2.0                              │
├─────────────────────────────────────────────────────────────────────────────┤
│                           应用层 (Application Layer)                          │
│  ┌─────────────────────┬─────────────────────┬─────────────────────────────┐    │
│  │  命令行工具           │   图形用户界面      │      测试工具              │    │
│  │  CLI Interface      │   GUI Application     │      Test Suites           │    │
│  └─────────────────────┴─────────────────────┴─────────────────────────────┘    │
├─────────────────────────────────────────────────────────────────────────────┤
│                           服务层 (Service Layer)                             │
│  ┌─────────────┬──────────┬─────────────┬──────────┬─────────────┬─────────────┐  │
│  │   播放器    │ 播放列表  │  元数据管理  │  可视化   │  配置管理  │  日志系统  │  │
│  │  Engine   │  Manager │  Manager   │  Engine   │  Manager    │  System    │  │
│  └─────────────┴──────────┴─────────────┴──────────┴─────────────┴─────────────┘  │
├─────────────────────────────────────────────────────────────────────────────┤
│                           插件层 (Plugin Layer)                              │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                      Plugin Manager                           │   │
│  │  ┌─────────────────┬──────────────┬─────────────────────┐       │   │
│  │  │   Native Plugin   │ Compatibility │   Foobar2000      │       │   │
│  │  │      SDK         │     Layer    │   Adapter         │       │   │
│  │  └─────────────────┴──────────────┴─────────────────────┘       │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────────────────────┤
│                           音频处理层 (Audio Layer)                           │
│  ┌─────────────┬─────────────────────┬─────────────┬─────────────────────┐  │
│  │   解码器    │    DSP处理     │  重采样器   │    输出系统     │  │
│  │  Plugins    │   Plugins     │  System     │    Plugins     │  │
│  └─────────────┴─────────────────────┴─────────────┴─────────────────────┘  │
├─────────────────────────────────────────────────────────────────────────────┤
│                           平台抽象层 (Platform Layer)                        │
│  ┌─────────────────────┬──────────────┬─────────────────────────────┐   │
│  │      Windows      │     macOS     │         Linux            │   │
│  │    (WASAPI)       │  (CoreAudio) │ (ALSA/PulseAudio)        │   │
│  └─────────────────────┴──────────────┴─────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 核心组件详解

### 1. 配置系统 (Configuration System)

配置系统是整个系统的基础，提供统一的配置管理能力：

```cpp
// 配置系统架构
ConfigManager
├── 配置加载器 (JSON Parser)
├── 配置验证器 (Validator)
├── 环境变量支持 (Environment Variables)
└── 配置变更通知 (Change Notification)

// 配置数据流
JSON文件 → ConfigManager → 各模块
    ↓              ↓              ↓
验证 → 加载 → 应用 → 通知
```

**主要特性**：
- JSON格式配置文件
- 环境变量覆盖
- 分层配置结构
- 配置验证
- 热重载支持
- 变更通知机制

### 2. 插件系统 (Plugin System)

插件系统是可扩展性的核心，支持两种插件架构：

#### 2.1 Native Plugin SDK
```cpp
// 插件接口层次
IPlugin (基础接口)
├── IDecoder (解码器)
├── IDSPProcessor (DSP处理器)
├── IAudioOutput (音频输出)
└── IVisualization (可视化)
```

#### 2.2 foobar2000 兼容层
```cpp
// 兼容层架构
foobar2000 Plugin → Adapter Wrapper → Native Interface
       ↓                    ↓                  ↓
  Services/Components → Service Mapping → Standard Methods
```

**插件加载流程**：
1. 插件发现和扫描
2. 依赖解析
3. 动态库加载
4. 接口适配
5. 生命周期管理

### 3. 音频处理管道 (Audio Processing Pipeline)

音频处理管道是系统的核心，处理从解码到输出的整个流程：

```
音频文件 → 解码器 → 原始PCM → DSP处理器 → 重采样器 → 音频输出 → 硬件
    ↓          ↓        ↓          ↓         ↓          ↓
  File → Format → Audio → Effects → Rate Convert → Backend → Device
```

#### 3.1 重采样系统

重采样系统支持多种精度和算法：

```cpp
// 重采样架构
ISampleRateConverter
├── LinearSampleRateConverter      (32位)
├── CubicSampleRateConverter       (32位)
├── SincSampleRateConverter        (32位)
├── LinearSampleRateConverter64     (64位)
├── CubicSampleRateConverter64      (64位)
├── SincSampleRateConverter64       (64位)
└── AdaptiveSampleRateConverter     (自适应)
```

**算法特性对比**：
- **精度**：32位 vs 64位浮点
- **质量**：速度 vs 质量权衡
- **自适应**：根据CPU负载自动选择

### 4. 跨平台抽象

平台抽象层隐藏了不同操作系统的差异：

```cpp
// 平台特定实现
IAudioOutput
├── WindowsAudioOutput (WASAPI)
├── MacOSAudioOutput   (CoreAudio)
├── LinuxAudioOutput   (ALSA/Pulse)
└── StubAudioOutput     (无音频)
```

## 数据流

### 音频数据流

```
原始音频文件 → 解码
├── WAV: 原始PCM数据
├── MP3: 压缩音频数据
└── FLAC: 无损压缩数据

↓ 解码过程

原始PCM (16/24/32-bit) → 格式转换 → 32/64位浮点

↓ DSP处理 (可选)

音效处理 → 均衡器 → 动态范围压缩

↓ 重采样 (如需要)

输入采样率 → 目标采样率 → 插值处理

↓ 输出

音频缓冲区 → 平台API → 音频设备
```

### 配置数据流

```
系统启动
    ↓
加载默认配置 (default_config.json)
    ↓
加载用户配置 (~/.qoder-foobar/config.json)
    ↓
应用环境变量覆盖 (QODER_*)
    ↓
验证配置有效性
    ↓
分发到各模块
```

## 关键接口

### 1. 插件接口 (IPlugin)

```cpp
class IPlugin {
public:
    virtual ~IPlugin() = default;

    // 生命周期
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    // 状态管理
    virtual PluginState get_state() const = 0;
    virtual void set_state(PluginState state) = 0;

    // 信息
    virtual PluginInfo get_info() const = 0;
    virtual std::string get_last_error() const = 0;
};
```

### 2. 音频解码器接口 (IAudioDecoder)

```cpp
class IAudioDecoder : public IPlugin {
public:
    // 解码能力
    virtual bool can_decode(const std::string& file_path) = 0;
    virtual std::vector<std::string> get_supported_extensions() = 0;

    // 解码操作
    virtual bool open(const std::string& file_path) = 0;
    virtual int decode(AudioBuffer& buffer, int max_frames) = 0;
    virtual void close() = 0;

    // 格式信息
    virtual AudioFormat get_format() const = 0;
};
```

### 3. 配置管理器接口

```cpp
class ConfigManager {
public:
    // 初始化
    bool initialize(const std::string& config_file = "");

    // 配置操作
    bool load_config();
    bool save_config();
    bool reload_config();

    // 访问配置
    const AppConfig& get_config() const;
    AudioConfig& audio();
    ResamplerConfig& resampler();

    // 通知机制
    void add_change_callback(std::function<void(const AppConfig&)> callback);

    // 高级功能
    template<typename T>
    void set_config_value(const std::string& path, const T& value);

    template<typename T>
    T get_config_value(const std::string& path, const T& default_value = T{}) const;
};
```

## 线程模型

### 主要线程

1. **主线程**：UI和用户交互
2. **音频线程**：音频处理和输出
3. **解码线程**：音频文件解码
4. **插件管理线程**：插件加载/卸载

### 线程间通信

```cpp
// 事件总线系统
EventBus
├── 主线程事件
├── 音频线程事件
├── 插件线程事件
└── 配置变更事件

// 共享数据保护
std::mutex audio_mutex;
std::condition_variable audio_cv;
std::atomic<bool> is_playing;
```

## 内存管理

### RAII原则

所有资源使用RAII管理：

```cpp
class AudioPlayer {
private:
    std::unique_ptr<IAudioOutput> output_;
    std::unique_ptr<PluginManager> plugin_mgr_;

public:
    ~AudioPlayer() {
        // 自动清理资源
    }
};
```

### 智能指针使用

- `std::unique_ptr`：独占所有权
- `std::shared_ptr`：共享所有权（音频缓冲区）
- `std::weak_ptr`：弱引用（避免循环引用）

### 内存池

音频处理使用内存池减少分配开销：

```cpp
class AudioBufferPool {
private:
    std::vector<std::unique_ptr<AudioBuffer>> available_;
    std::mutex mutex_;

public:
    std::unique_ptr<AudioBuffer> acquire(int size);
    void release(std::unique_ptr<AudioBuffer> buffer);
};
```

## 错误处理

### Result 类型

```cpp
template<typename T>
class Result {
private:
    T value_;
    std::string error_;
    bool has_value_;

public:
    static Result<T> success(T value);
    static Result<T> error(const std::string& error);

    bool is_success() const;
    const T& value() const;
    const std::string& error() const;
};
```

### 异常策略

- **音频线程**：不抛出异常，返回错误状态
- **UI线程**：可以抛出异常，由UI层处理
- **插件层**：异常隔离，不影响主程序

## 性能优化

### 1. SIMD优化

音频处理使用SIMD指令：

```cpp
#ifdef __SSE2__
void process_audio_sse(float* data, int frames);
#endif
```

### 2. 零拷贝

音频数据传递使用指针或引用：

```cpp
// 避免：
std::vector<float> buffer = input_buffer;
output_buffer = buffer;  // 拷贝

// 推荐：
output_buffer = input_buffer;  // 移动语义或引用
```

### 3. 批处理

音频数据批量处理减少函数调用开销：

```cpp
// 每次处理更多帧
const int BATCH_SIZE = 4096;
```

## 扩展点

### 1. 新插件类型

```cpp
// 添加新的插件类型
enum class PluginType {
    Decoder = 1,
    DSP = 2,
    Output = 3,
    Visualization = 4,
    Metadata = 5  // 新增
};
```

### 2. 新音频后端

```cpp
// 添加新的平台支持
#ifdef HAVE_JACK_AUDIO
    #include "platform/jack_audio_output.h"
#endif
```

### 3. 新配置模块

```cpp
// 添加新的配置节
struct NetworkConfig {
    bool enable_streaming = false;
    std::string server_url;
    int buffer_size = 32768;
};
```

## 测试策略

### 单元测试

```cpp
// 每个组件都有对应的测试
TEST(AudioDecoderTest, CanDecodeValidFile) {
    MyDecoder decoder;
    ASSERT_TRUE(decoder.can_decode("test.wav"));
}
```

### 集成测试

```cpp
// 端到端测试
TEST(AudioPipelineTest, PlayCompleteFile) {
    AudioPlayer player;
    EXPECT_TRUE(player.load("test.wav"));
    EXPECT_TRUE(player.play());
}
```

### 性能测试

```cpp
// 基准测试
TEST(ResamplerBenchmark, Measure64bitPerformance) {
    // 测试64位重采样性能
}
```

## 部署架构

### 开发环境

```
开发机器 → Git Repository → CI/CD Pipeline → 构建产物
                ↓                      ↓
        本地测试 → 自动化测试 → 发布包
```

### 生产部署

```
安装包 → 系统安装 → 配置目录 → 运行实例
    ↓          ↓            ↓           ↓
  二进制文件 → 配置文件 → 用户数据 → 进程
```

## 总结

Qoder foobar v2.0 的架构设计充分考虑了：

1. **可扩展性**：模块化设计便于添加新功能
2. **可维护性**：清晰的分层架构和接口定义
3. **性能**：优化的音频处理管道
4. **灵活性**：丰富的配置选项和插件支持
5. **兼容性**：跨平台支持和向后兼容

这种架构设计使得系统既能满足当前需求，也为未来的功能扩展奠定了坚实的基础。