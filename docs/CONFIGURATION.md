# Qoder foobar 配置系统

## 概述

Qoder foobar 使用 JSON 格式的配置文件管理系统，支持：
- 模块化配置结构
- 环境变量覆盖
- 配置验证
- 热重载支持
- 用户自定义主题

## 配置文件位置

配置文件按优先级加载（高优先级覆盖低优先级）：

1. 命令行指定的配置文件 (`-c` 参数)
2. `~/.qoder-foobar/config.json`
3. `/etc/qoder-foobar/config.json`
4. `<安装目录>/share/qoder-foobar/config/config.json`
5. `<源码目录>/config/default_config.json`

## 配置结构

### 音频配置 (audio)

```json
{
    "audio": {
        "output_device": "default",        // 输出设备名称
        "sample_rate": 44100,              // 采样率 (Hz)
        "channels": 2,                     // 声道数
        "bits_per_sample": 32,             // 位深度
        "use_float": true,                 // 是否使用浮点格式
        "buffer_size": 4096,               // 缓冲区大小
        "buffer_count": 4,                 // 缓冲区数量
        "volume": 0.8,                     // 音量 (0.0-1.0)
        "mute": false,                     // 静音开关
        "equalizer_preset": "flat"         // 均衡器预设
    }
}
```

### 插件配置 (plugins)

```json
{
    "plugins": {
        "plugin_directories": [            // 插件搜索目录
            "./plugins",
            "~/.qoder-foobar/plugins",
            "/usr/lib/qoder-foobar/plugins"
        ],
        "auto_load_plugins": true,         // 自动加载插件
        "plugin_scan_interval": 0,         // 插件扫描间隔（秒）
        "plugin_timeout": 5000             // 插件超时（毫秒）
    }
}
```

### 重采样配置 (resampler)

```json
{
    "resampler": {
        "quality": "adaptive",             // 质量模式
        "floating_precision": 32,          // 浮点精度: 32或64
        "enable_adaptive": true,           // 启用自适应模式
        "cpu_threshold": 0.8,              // CPU阈值 (0.1-1.0)
        "use_anti_aliasing": true,         // 使用抗混叠
        "cutoff_ratio": 0.95,              // 截止频率比 (0.1-0.99)
        "filter_taps": 101,                // 滤波器抽头数 (3-999)
        "format_quality": {                // 各格式质量设置
            "mp3": "good",
            "flac": "best",
            "wav": "fast",
            "ogg": "good",
            "aac": "good",
            "m4a": "good",
            "wma": "fast",
            "ape": "high"
        }
    }
}
```

#### 质量模式说明

- **fast**: 线性插值，最快速度，适合实时应用
- **good**: 三次插值，速度与质量平衡
- **high**: 8抽头Sinc插值，高质量处理
- **best**: 16抽头Sinc插值，最高质量，适合母带处理
- **adaptive**: 自适应选择，根据CPU负载自动调整

#### 浮点精度选择

**32位浮点 (默认)**
- 24位尾数精度
- 约150dB动态范围
- 较低的CPU和内存使用
- 适合一般播放场景

**64位浮点**
- 53位尾数精度
- 约320dB动态范围
- 更高的累积精度
- 适合专业音频制作和多级处理

#### 自适应模式

自适应模式会监控CPU使用率：
- CPU使用率 > 阈值：降低质量以提高性能
- CPU使用率 < 阈值/2：提高质量以获得更好效果

### 播放器配置 (player)

```json
{
    "player": {
        "repeat": false,                   // 重复播放
        "shuffle": false,                  // 随机播放
        "crossfade": false,                // 淡入淡出
        "crossfade_duration": 2.0,         // 淡入淡出时长
        "preferred_backend": "auto",       // 首选音频后端
        "show_console_output": true,       // 显示控制台输出
        "show_progress_bar": true,         // 显示进度条
        "key_bindings": {                  // 键盘快捷键
            "play": "space",
            "pause": "p",
            "stop": "s",
            "next": "n",
            "previous": "b",
            "quit": "q"
        },
        "max_history": 1000,               // 最大历史记录数
        "save_history": true,              // 保存历史记录
        "history_file": "~/.qoder-foobar/history.json"
    }
}
```

### 日志配置 (logging)

```json
{
    "logging": {
        "level": "info",                   // 日志级别
        "console_output": true,            // 控制台输出
        "file_output": false,              // 文件输出
        "log_file": "~/.qoder-foobar/qoder.log",
        "enable_rotation": true,           // 启用日志轮转
        "max_file_size": 10485760,         // 最大文件大小
        "max_files": 5,                    // 最大文件数
        "include_timestamp": true,         // 包含时间戳
        "include_thread_id": false,        // 包含线程ID
        "include_function_name": true      // 包含函数名
    }
}
```

### 界面配置 (ui)

```json
{
    "ui": {
        "theme": "default",                // 主题名称
        "language": "en",                  // 界面语言
        "font_family": "default",          // 字体族
        "font_size": 12.0,                 // 字体大小
        "save_window_size": true,          // 保存窗口大小
        "window_width": 1024,              // 窗口宽度
        "window_height": 768,              // 窗口高度
        "start_maximized": false           // 启动时最大化
    }
}
```

## 主题系统

主题文件位于 `config/themes/` 目录，每个主题是一个独立的 JSON 文件。

### 主题结构

```json
{
    "name": "主题名称",
    "version": "1.0.0",
    "description": "主题描述",
    "colors": {
        "background": "#FFFFFF",
        "foreground": "#000000",
        "accent": "#0078D7",
        // ... 更多颜色定义
    },
    "fonts": {
        "default": {
            "family": "Segoe UI",
            "size": 12.0,
            "weight": "normal"
        },
        // ... 更多字体定义
    },
    "dimensions": {
        // 尺寸定义
    },
    "effects": {
        "rounding": 4,
        "shadows": true,
        "animations": true
    }
}
```

## 环境变量

支持使用环境变量覆盖配置项：

```bash
# 设置输出设备
export QODER_AUDIO_OUTPUT_DEVICE=pulse

# 设置采样率
export QODER_AUDIO_SAMPLE_RATE=48000

# 设置音量
export QODER_AUDIO_VOLUME=0.5

# 设置日志级别
export QODER_LOGGING_LEVEL=debug
```

环境变量命名规则：
- 前缀：`QODER_`
- 配置节：大写
- 配置项：大写，用 `_` 分隔
- 嵌套配置：用双下划线 `__` 分隔

## 命令行选项

music-player-config 支持以下命令行选项：

```bash
# 指定配置文件
music-player-config -c /path/to/config.json

# 列出可用设备
music-player-config --list-devices

# 指定输出设备
music-player-config --device pulse

# 设置采样率
music-player-config --rate 48000

# 设置声道数
music-player-config --channels 4

# 设置音量
music-player-config --volume 0.7
```

## 配置验证

配置加载时会自动验证以下内容：

1. **音频配置**
   - 采样率：8000 - 768000 Hz
   - 声道数：1 - 8
   - 位深度：16, 24, 32
   - 音量：0.0 - 1.0

2. **重采样配置**
   - 质量模式：fast, good, high, best, adaptive
   - CPU阈值：0.1 - 1.0

3. **日志配置**
   - 日志级别：trace, debug, info, warn, error, fatal

## API 使用

### C++ API

```cpp
#include "config_manager.h"

// 获取配置管理器实例
ConfigManager& config = ConfigManagerSingleton::get_instance();

// 初始化
config.initialize("/path/to/config.json");

// 获取配置
const auto& audio_config = config.audio();
int sample_rate = audio_config.sample_rate;

// 修改配置
config.audio().volume = 0.9;
config.save_config();

// 监听配置变更
config.add_change_callback([](const AppConfig& new_config) {
    std::cout << "Configuration changed!" << std::endl;
});

// 获取特定配置值
double volume = config.get_config_value<double>("audio.volume", 0.8);
config.set_config_value("player.repeat", true);
```

## 最佳实践

1. **配置备份**
   - 修改配置前先备份
   - 使用版本控制管理自定义配置

2. **性能优化**
   - 避免频繁保存配置
   - 使用缓存减少配置读取

3. **安全考虑**
   - 限制配置文件权限
   - 验证来自不可信来源的配置

4. **插件开发**
   - 插件应通过配置系统获取设置
   - 使用命名空间避免配置冲突

## 采样率支持

所有标准采样率都支持，包括：

**常用速率**:
- 8 kHz, 11.025 kHz, 16 kHz
- 22.05 kHz, 44.1 kHz, 48 kHz
- 88.2 kHz, 96 kHz, 176.4 kHz

**专业速率**:
- 192 kHz, 352.8 kHz, 384 kHz, 705.6 kHz
- 768 kHz

## 性能考虑

### 32位 vs 64位精度对比

| 特性 | 32位 | 64位 |
|------|------|------|
| CPU使用 | 基准 | +30~40% |
| 内存使用 | 基准 | +100% |
| 动态范围 | ~150dB | ~320dB |
| 精度 | 24位 | 53位 |
| 适用场景 | 一般播放 | 专业制作 |

### 推荐配置

**日常聆听**:
```json
{
    "resampler": {
        "quality": "good",
        "floating_precision": 32,
        "enable_adaptive": true
    }
}
```

**音频制作**:
```json
{
    "resampler": {
        "quality": "best",
        "floating_precision": 64,
        "enable_adaptive": false
    }
}
```

**实时处理**:
```json
{
    "resampler": {
        "quality": "fast",
        "floating_precision": 32,
        "enable_adaptive": false
    }
}
```

## 配置验证规则

### 音频配置
- `sample_rate`: 8000 - 768000 Hz
- `channels`: 1 - 8
- `bits_per_sample`: 16, 24, 32
- `volume`: 0.0 - 1.0
- `buffer_size`: 64 - 65536

### 重采样配置
- `quality`: fast, good, high, best, adaptive
- `floating_precision`: 32, 64
- `cpu_threshold`: 0.1 - 1.0
- `cutoff_ratio`: 0.1 - 0.99
- `filter_taps`: 3 - 999

### 日志配置
- `level`: trace, debug, info, warn, error, fatal
- `max_file_size`: 1024 - 1073741824 (1KB - 1GB)
- `max_files`: 1 - 100