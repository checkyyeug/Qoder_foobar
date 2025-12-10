# 🔄 码率转换插件分析报告

## 📊 当前状态: **架构就绪，实现待完成 (30%)**

---

## 🎯 码率转换需求

### 为什么需要码率转换？

1. **设备兼容性**
   - 不同音频设备支持不同的采样率
   - 扬声器: 通常 44.1kHz 或 48kHz
   - 录音设备: 8kHz 到 192kHz
   - 专业设备: 96kHz 或 192kHz

2. **性能优化**
   - 高采样率消耗更多 CPU 和内存
   - 转换为合适的采样率可提高性能

3. **音质控制**
   - 上采样: 提高音质（需插值）
   - 下采样: 降低音质（需抗混叠）

4. **格式转换**
   - 不同音频格式的标准采样率不同
   - MP3: 8kHz - 48kHz
   - CD: 44.1kHz
   - DVD: 48kHz
   - 蓝光: 96kHz/192kHz

---

## 📋 当前后端采样率支持

### ALSA (Linux)
```cpp
// 当前 ALSA 实现支持
- 设置目标采样率: snd_pcm_hw_params_set_rate_near()
- 自动选择最接近的设备采样率
- 不支持运行时转换
```

### WASAPI (Windows)
```cpp
// WASAPI 实现支持
- 使用客户端提供的采样率
- Windows 音频引擎自动处理转换
- 用户无法控制转换过程
```

### CoreAudio (macOS)
```cpp
// CoreAudio 实现支持
- 音频队列自动处理采样率
- 用户无感知转换
- 无手动控制接口
```

---

## 🔧 码率转换插件设计

### 1. 插件接口定义
```cpp
// src/audio/sample_rate_converter.h
class ISampleRateConverter {
public:
    virtual ~ISampleRateConverter() = default;

    // 初始化转换器
    virtual bool initialize(
        int input_rate,
        int output_rate,
        int channels
    ) = 0;

    // 执行转换
    virtual int convert(
        const float* input,
        int input_frames,
        float* output,
        int max_output_frames
    ) = 0;

    // 获取延迟
    virtual int get_latency() const = 0;

    // 重置状态
    virtual void reset() = 0;

    // 转换器信息
    virtual const char* get_name() const = 0;
    virtual const char* get_description() const = 0;
};
```

### 2. 转换器工厂
```cpp
class SampleRateConverterFactory {
public:
    // 创建转换器
    static std::unique_ptr<ISampleRateConverter> create(
        const std::string& type = "linear"
    );

    // 列出可用转换器
    static std::vector<std::string> list_available();

    // 检查转换器可用性
    static bool is_available(const std::string& type);
};
```

---

## 🛠️ 实现方案

### 方案 1: 线性插值 (简单实现)
```cpp
class LinearSampleRateConverter : public ISampleRateConverter {
private:
    double ratio_;
    double position_;
    int channels_;

public:
    bool initialize(int input_rate, int output_rate, int channels) override {
        ratio_ = static_cast<double>(input_rate) / output_rate;
        channels_ = channels;
        position_ = 0.0;
        return true;
    }

    int convert(const float* input, int input_frames,
                float* output, int max_output_frames) override {
        int output_frames = 0;

        for (; output_frames < max_output_frames; ++output_frames) {
            int input_index = static_cast<int>(position_);
            double fraction = position_ - input_index;

            if (input_index >= input_frames - 1) break;

            // 线性插值
            for (int ch = 0; ch < channels_; ++ch) {
                float sample1 = input[input_index * channels_ + ch];
                float sample2 = input[(input_index + 1) * channels_ + ch];
                output[output_frames * channels_ + ch] =
                    sample1 + (sample2 - sample1) * fraction;
            }

            position_ += ratio_;
        }

        return output_frames;
    }
};
```

### 方案 2: 多项式插值 (高质量)
```cpp
class CubicSampleRateConverter : public ISampleRateConverter {
private:
    std::vector<float> buffer_;
    double ratio_;
    double position_;
    int channels_;

    // 三次插值
    float cubic_interpolate(float y0, float y1, float y2, float y3, double mu) {
        double mu2 = mu * mu;
        double mu3 = mu2 * mu;

        double a0 = y3 - y2 - y0 + y1;
        double a1 = y0 - y1;
        double a2 = y2 - y0;
        double a3 = y1 - y0;

        return a0 * mu3 + a1 * mu2 + a2 * mu + a3;
    }

public:
    bool initialize(int input_rate, int output_rate, int channels) override {
        ratio_ = static_cast<double>(input_rate) / output_rate;
        channels_ = channels;
        position_ = 0.0;
        buffer_.resize(4 * channels_);
        return true;
    }

    int convert(const float* input, int input_frames,
                float* output, int max_output_frames) override {
        // 实现三次插值转换
        // 详细实现略...
        return output_frames;
    }
};
```

### 方案 3: 使用外部库 (专业级)

#### libsamplerate (高质量)
```cpp
class LibsamplerateConverter : public ISampleRateConverter {
private:
    SRC_STATE* src_state_;
    SRC_DATA src_data_;
    int channels_;
    int converter_type_;

public:
    bool initialize(int input_rate, int output_rate, int channels) override {
        channels_ = channels;

        int error;
        src_state_ = src_new(SRC_SINC_FASTEST, channels, &error);

        if (!src_state_) {
            return false;
        }

        src_data_.src_ratio = static_cast<double>(output_rate) / input_rate;
        src_data_.end_of_input = 0;

        return true;
    }

    int convert(const float* input, int input_frames,
                float* output, int max_output_frames) override {
        src_data_.data_in = const_cast<float*>(input);
        src_data_.input_frames = input_frames;
        src_data_.data_out = output;
        src_data_.output_frames = max_output_frames;
        src_data_.end_of_input = (input_frames == 0) ? 1 : 0;

        int error = src_process(src_state_, &src_data_);

        if (error) {
            return 0;
        }

        return static_cast<int>(src_data_.output_frames_gen);
    }
};
```

---

## 🔌 插件集成方案

### 1. 音频管道中的位置
```cpp
// src/audio/audio_pipeline.cpp
class AudioPipeline {
private:
    std::unique_ptr<ISampleRateConverter> resampler_;
    std::unique_ptr<IAudioOutput> output_;
    AudioFormat input_format_;
    AudioFormat output_format_;

public:
    bool set_sample_rate(int target_rate) {
        if (input_format_.sample_rate != target_rate) {
            // 创建转换器
            resampler_ = SampleRateConverterFactory::create("linear");

            if (!resampler_->initialize(
                input_format_.sample_rate,
                target_rate,
                input_format_.channels
            )) {
                return false;
            }

            output_format_.sample_rate = target_rate;

            // 重新打开音频设备
            output_->close();
            if (!output_->open(output_format_)) {
                return false;
            }
        }

        return true;
    }

    int write(const float* buffer, int frames) {
        if (resampler_) {
            // 需要转换
            std::vector<float> temp_buffer(frames * 2);
            int converted = resampler_->convert(
                buffer, frames,
                temp_buffer.data(), temp_buffer.size() / 2
            );
            return output_->write(temp_buffer.data(), converted);
        } else {
            // 无需转换
            return output_->write(buffer, frames);
        }
    }
};
```

### 2. 配置管理
```cpp
// src/audio/audio_config.h
struct AudioConfig {
    int preferred_sample_rate = 44100;
    std::string resampler_type = "linear";
    bool auto_resample = true;

    // 从配置文件加载
    void load() {
        // 从 config.json 或配置文件读取
    }

    // 保存到配置文件
    void save() {
        // 保存当前配置
    }
};
```

---

## 📊 性能对比

| 转换方法 | CPU 使用 | 音质 | 延迟 | 复杂度 |
|----------|----------|------|------|--------|
| 线性插值 | 低 | 中 | 低 | ★★☆☆☆ |
| 三次插值 | 中 | 高 | 中 | ★★★☆☆ |
| libsamplerate | 中-高 | 很高 | 中 | ★★★★☆ |
| FFT 重采样 | 高 | 极高 | 高 | ★★★★★ |

---

## 🎯 使用场景

### 1. 高保真播放
```cpp
// 目标: 最高音质
auto converter = SampleRateConverterFactory::create("libsamplerate");
converter->initialize(input_rate, 96000, channels);  // 升频到 96kHz
```

### 2. 性能优化
```cpp
// 目标: 低 CPU 使用
auto converter = SampleRateConverterFactory::create("linear");
converter->initialize(96000, 44100, channels);  // 降频到 44.1kHz
```

### 3. 设备适配
```cpp
// 自动选择设备最佳采样率
int device_rate = audio_device->get_optimal_sample_rate();
set_sample_rate(device_rate);
```

---

## 🚀 实现计划

### 第一阶段: 基础实现 (1 周)
- [ ] 定义 ISampleRateConverter 接口
- [ ] 实现线性插值转换器
- [ ] 实现转换器工厂
- [ ] 集成到音频管道

### 第二阶段: 质量提升 (1 周)
- [ ] 实现三次插值转换器
- [ ] 优化性能和内存使用
- [ ] 添加配置管理
- [ ] 实现自动采样率检测

### 第三阶段: 专业功能 (2 周)
- [ ] 集成 libsamplerate
- [ ] 实现多线程转换
- [ ] 添加转换质量预设
- [ ] 实现实时转换监控

---

## 📝 结论

码率转换是音频播放器的重要功能，当前项目：

### ✅ 已完成
- 架构设计完整
- 接口定义清晰
- 集成方案明确

### ⚠️ 待完成
- 实际转换器实现
- 与播放器集成
- 配置和优化

### 🎯 实现价值
1. **提高兼容性**: 支持更多音频设备
2. **优化性能**: 选择合适的采样率
3. **改善音质**: 高质量重采样
4. **专业特性**: 满足专业用户需求

码率转换插件将使 Qoder foobar 从基础播放器升级为**专业级音频播放器**！