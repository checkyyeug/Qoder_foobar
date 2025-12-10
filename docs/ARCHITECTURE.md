# Qoder foobar v2.0 系统架构

## 项目概述

Qoder foobar v2.0 是一个模块化、跨平台的专业音乐播放器，支持：
- 32/64位浮点音频处理
- 自适应采样率转换（8kHz - 768kHz）
- 插件热插拔系统
- JSON配置管理
- 多平台音频后端（ALSA/Pulse/WASAPI/CoreAudio）

## 整体架构

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

## 核心组件

### 1. 配置系统 (config/)
```cpp
// 主要接口
class ConfigManager {
    bool initialize(const std::string& config_file = "");
    const AppConfig& get_config() const;
    template<typename T> void set_config_value(const std::string& path, const T& value);
    void add_change_callback(std::function<void(const AppConfig&)> callback);
};
```
- **配置文件层次**：命令行 > ~/.qoder-foobar/ > /etc/ > 安装目录 > 源码目录
- **环境变量覆盖**：QODER_AUDIO_*, QODER_RESAMPLER_*, QODER_LOGGING_*
- **实时更新**：配置变更自动通知到各模块
- **验证机制**：自动验证配置值的有效范围

### 2. 插件系统 (sdk/, compat/, plugins/)
```cpp
// 核心插件接口
class IPlugin {
    virtual bool initialize() = 0;
    virtual PluginInfo get_info() const = 0;
    virtual PluginState get_state() const = 0;
};

class IAudioDecoder : public IPlugin {
    virtual bool can_decode(const std::string& file_path) = 0;
    virtual int decode(AudioBuffer& buffer, int max_frames) = 0;
    virtual AudioFormat get_format() const = 0;
};
```
- **双SDK架构**：Native SDK + foobar2000兼容层
- **插件类型**：解码器、DSP处理器、音频输出、可视化
- **热插拔**：运行时动态加载/卸载
- **依赖管理**：自动解析插件依赖关系
- **沙箱隔离**：插件异常不影响主程序

### 3. 音频处理管道 (src/audio/)
```
音频文件 → 解码器插件 → PCM数据 → DSP插件 → 重采样器 → 音频输出插件 → 硬件
```

#### 3.1 重采样系统
```cpp
// 精度选择
class ConfiguredSampleRateConverter {
    bool configure(int input_rate, int output_rate, int channels);
    void set_precision(int bits); // 32或64位
};

// 算法实现
- LinearSampleRateConverter     // 线性插值（最快）
- CubicSampleRateConverter      // 三次插值（平衡）
- SincSampleRateConverter       // Sinc插值（高质量）
- AdaptiveSampleRateConverter  // 自适应选择
```

#### 3.2 64位浮点支持
```cpp
class ISampleRateConverter64 {
    virtual bool configure(int input_rate, int output_rate, int channels) = 0;
    virtual int process(const double* input, double* output, int input_frames) = 0;
};
```
- **动态范围**：32位(~150dB) vs 64位(~320dB)
- **性能开销**：64位CPU使用率增加30-40%
- **适用场景**：专业音频制作使用64位，一般播放32位足够

### 4. 跨平台音频抽象 (src/audio/, platform/)
```cpp
class AudioOutputBase : public IAudioOutput {
    bool set_volume(double volume) override;
    bool set_mute(bool mute) override;
protected:
    virtual bool do_initialize() = 0;
    virtual int do_write(const float* data, int frames) = 0;
};
```

#### 平台实现
- **Linux**:
  - ALSA直接输出（低延迟）
  - PulseAudio输出（混音支持）
  - 自动检测，优先PulseAudio
- **Windows**: WASAPI（独占/共享模式）
- **macOS**: CoreAudio（低延迟音频单元）

### 5. 核心引擎 (core/)
```cpp
class CoreEngine {
    bool load_file(const std::string& path);
    bool play();
    bool pause();
    bool stop();

    // 插件管理
    PluginManager& plugin_manager();

    // 配置访问
    ConfigManager& config_manager();
};
```

### 6. 事件系统
```cpp
class EventBus {
    template<typename EventType>
    void publish(const EventType& event);

    template<typename EventType>
    void subscribe(std::function<void(const EventType&)> handler);
};
```
- **线程安全**：使用无锁队列实现
- **事件类型**：播放事件、错误事件、配置变更事件

## 关键特性

- **模块化设计**：每个功能独立，便于维护和扩展
- **高性能**：零拷贝音频缓冲区，SIMD优化
- **可扩展**：插件系统支持动态加载
- **跨平台**：支持主流操作系统
- **精度可选**：32/64位浮点处理
- **自适应**：根据CPU负载自动调整重采样质量

## 数据流

### 音频数据流
```
1. 音频文件 → 解码器插件 → 原始PCM
2. PCM → 位深度转换 → 32/64位浮点
3. 浮点音频 → DSP插件链 → 处理后音频
4. 处理后音频 → 重采样器 → 目标采样率
5. 最终音频 → 音频输出插件 → 硬件设备
```

### 配置数据流
```
1. 加载默认配置 (config/default_config.json)
2. 加载系统配置 (/etc/qoder-foobar/config.json)
3. 加载用户配置 (~/.qoder-foobar/config.json)
4. 应用环境变量覆盖 (QODER_*)
5. 验证配置有效性
6. 分发到各模块并注册变更监听
```

## 线程模型

```
主线程 (UI/控制)
├── 音频线程 (实时音频处理)
│   ├── 解码线程池
│   ├── DSP处理线程
│   └── 输出线程
├── 插件管理线程
│   ├── 插件扫描
│   ├── 加载/卸载
│   └── 依赖解析
└── 文件I/O线程
    ├── 配置文件读写
    ├── 日志写入
    └── 元数据缓存
```

## 内存管理

### RAII和智能指针
```cpp
class AudioPlayer {
private:
    std::unique_ptr<IAudioOutput> output_;
    std::shared_ptr<AudioBuffer> buffer_;
    std::weak_ptr<PluginManager> plugin_mgr_;
};
```

### 音频缓冲区管理
```cpp
class AudioBufferPool {
    std::unique_ptr<AudioBuffer> acquire(int size);
    void release(std::unique_ptr<AudioBuffer> buffer);
private:
    std::queue<std::unique_ptr<AudioBuffer>> available_;
    std::mutex mutex_;
};
```

## 错误处理

### Result类型
```cpp
template<typename T>
class Result {
    static Result<T> success(T value);
    static Result<T> error(const std::string& error);
    bool is_success() const;
    const T& value() const;
    const std::string& error() const;
};
```

### 异常策略
- **音频线程**：不抛出异常，返回错误状态
- **UI线程**：使用try-catch处理用户操作
- **插件沙箱**：隔离插件异常，记录日志

## 性能优化

### 1. SIMD优化
```cpp
#ifdef __SSE2__
void process_audio_sse(float* data, int frames) {
    // 使用SSE2指令集并行处理
}
#endif
```

### 2. 零拷贝设计
- 音频缓冲区使用指针传递
- 避免不必要的内存拷贝
- 移动语义优化大对象传递

### 3. 内存预分配
- 音频缓冲区池化
- 预分配FFT工作空间
- 避免实时内存分配

## 扩展点

### 1. 新插件类型
```cpp
enum class PluginType {
    AudioDecoder = 1,
    DSPProcessor = 2,
    AudioOutput = 3,
    Visualization = 4,
    MetadataReader = 5,    // 新增
    PlaylistManager = 6    // 新增
};
```

### 2. 新音频后端
```cpp
// 添加新的平台支持
#ifdef HAVE_JACK_AUDIO
    #include "platform/jack_audio_output.h"
    REGISTER_AUDIO_OUTPUT(jack, JackAudioOutput);
#endif

#ifdef HAVE_PIPEWIRE
    #include "platform/pipewire_audio_output.h"
    REGISTER_AUDIO_OUTPUT(pipewire, PipeWireAudioOutput);
#endif
```

### 3. 新配置模块
```cpp
struct NetworkConfig {
    bool enable_streaming = false;
    std::string server_url;
    int buffer_size = 32768;
    int reconnect_timeout = 5000;
};
```

### 4. 新DSP效果
```cpp
class ReverbProcessor : public IDSPProcessor {
    bool configure(const AudioFormat& format) override;
    int process(float* input, float* output, int frames) override;

    void set_room_size(double size);
    void set_damping(double damping);
    void set_wet_level(double level);
};
```

## 构建系统

### CMake模块化设计
```
CMakeLists.txt (根)
├── config/CMakeLists.txt
├── core/CMakeLists.txt
├── src/CMakeLists.txt
├── sdk/CMakeLists.txt
└── plugins/CMakeLists.txt
```

### 依赖管理
- **必需依赖**：CMake 3.20+, C++17, nlohmann/json
- **可选依赖**：ALSA, PulseAudio, FFmpeg
- **自动检测**：cmake/AudioBackend.cmake