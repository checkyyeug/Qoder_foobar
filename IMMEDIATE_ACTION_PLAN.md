# 即时行动计划：foobar2000 兼容性紧急修复

**创建日期**: 2025-12-09
**优先级**: URGENT
**当前兼容性评级**: 15/100（严重不兼容）

## 执行摘要

基于深度技术分析，项目存在根本性架构不兼容问题。本计划专注于最关键的 5 个修复项目，预计可在 4-6 周内显著提升兼容性至 45-55/100（部分可用）。

## 现状：关键故障分析

### 当前代码库的兼容性缺陷

```
┌────────────────────────────────────────────────────────────┐
│ 已发现的严重问题（必须立即修复）                          │
├────────────────────────────────────────────────────────────┤
│ ❌ 零个适配器实现（仅 32 个存根和接口）                   │
│ ❌ SDK 接口是空的头文件（无任何实现）                     │
│ ❌ 元数据系统：只有简单的键值对，无多值字段               │
│ ❌ 无脚本引擎（用户无法使用自定义格式）                   │
│ ❌ 播放引擎：单线程，无 DSP 链                            │
│ ❌ 配置系统：JSON 与 foobar2000 的全局变量不兼容          │
│ ❌ 无法加载任何 foobar2000 DLL（调用约定不匹配）         │
└────────────────────────────────────────────────────────────┘
```

## 五大关键修复项目

### 🔴 **修复 #1：实现基础 SDK 接口**（缺失率 100%）

**状态**：未开始  
**影响**：阻塞 - 无法加载任何插件  
**预计时间**：1-2 周  
**复杂度**：中等

#### 需要实现的接口

```cpp
// ============================================
// compat/sdk_implementations/service_base.cpp
// ============================================
#include "foobar_sdk/foobar2000_sdk.h"

// service_base 实现（所有服务的基类）
service_base::~service_base() = default;

void service_base::service_add_ref() {
    ++refcount_;
}

void service_base::service_release() {
    if (--refcount_ == 0) {
        delete this;
    }
}

// ============================================
// compat/sdk_implementations/abort_callback.cpp
// ============================================
class abort_callback_impl : public abort_callback {
private:
    std::atomic<bool> aborted_{false};
    mutable std::mutex mutex_;
    std::vector<std::function<bool()>> callbacks_;

public:
    bool is_aborting() const override {
        if (aborted_.load()) return true;
        
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& cb : callbacks_) {
            if (cb()) {
                aborted_ = true;
                return true;
            }
        }
        return false;
    }
    
    void set() { aborted_ = true; }
};

// ============================================
// compat/sdk_implementations/audio_chunk.cpp
// ============================================
class audio_chunk_impl : public audio_chunk {
    pfc::array_t<audio_sample> data_;
    uint32_t sample_rate_ = 0;
    uint32_t channels_ = 0;
    uint64_t sample_count_ = 0;
    
public:
    audio_sample* get_data() override { return data_.get_ptr(); }
    const audio_sample* get_data() const override { return data_.get_ptr(); }
    uint32_t get_sample_count() const override { return sample_count_; }
    // ... 更多方法 ...
};
```

#### 任务清单
- [ ] **周一**：创建 `compat/sdk_implementations/` 目录结构
- [ ] **周一**：实现 `service_base`、`abort_callback`、`audio_chunk`
- [ ] **周二**：实现 `file_info`（关键：多值字段支持）
- [ ] **周三**：实现 `audio_sample`、内存管理工具
- [ ] **周四**：实现字符串处理（`pfc::string` 等）
- [ ] **周五**：接口测试和验证
- [ ] **第二周**：实现 `metadb_handle`、`metadb` 存根

#### 关键代码示例

```cpp
// compat/sdk_implementations/file_info_impl.h
#pragma once
#include "foobar_sdk/foobar2000_sdk.h"
#include <unordered_map>
#include <vector>

namespace foobar2000_sdk {

class file_info_impl : public file_info {
    // 关键：支持多值字段，如 tagger::["artist1", "artist2"]
    struct field_value {
        std::vector<std::string> values;
    };
    
    std::unordered_map<std::string, field_value> meta_fields_;
    audio_info_impl audio_info_;
    file_stats stats_;
    
public:
    // file_info 接口实现
    const char* meta_get(const char* p_name, size_t p_index) const override;
    size_t meta_get_count(const char* p_name) const override;
    bool meta_set(const char* p_name, const char* p_value) override;
    bool meta_remove(const char* p_name) override;
    bool meta_add(const char* p_name, const char* p_value) override;
    
    // 音频信息
    void set_audio_info(const audio_info& info) override { audio_info_ = info; }
    const audio_info& get_audio_info() const override { return audio_info_; }
    
    // 文件统计
    void set_file_stats(const file_stats& stats) override { stats_ = stats; }
    const file_stats& get_file_stats() const override { return stats_; }
};

} // namespace foobar2000_sdk
```

---

### 🔴 **修复 #2：创建可加载 foobar2000 DLL 的插件加载器**（缺失率 100%）

**状态**：未开始  
**影响**：阻塞 - 插件无法加载  
**预计时间**：2-3 周  
**复杂度**：高（涉及复杂的接口菱形继承问题）

#### 架构

```cpp
// ============================================
// compat/plugin_loader/plugin_loader.h
// ============================================
class FoobarPluginLoader {
public:
    struct LoadedModule {
        void* handle = nullptr;
        std::vector<service_factory_base*> service_factories;
        std::string path;
        bool initialized = false;
    };
    
    Result load_plugin(const char* dll_path);
    void unload_plugin(const char* dll_path);
    
private:
    // 解析 foobar2000 的导出函数
    Result parse_service_entries(LoadedModule& module);
    
    // 对于每个服务工厂，注册到服务注册表
    Result register_services(LoadedModule& module);
    
    std::vector<LoadedModule> modules_;
    std::mutex mutex_;
};

// ============================================
// compat/plugin_loader/service_factory_adapter.h
// ============================================
// 桥接 foobar2000 服务工厂到我们的插件系统
class ServiceFactoryAdapter : public service_factory_base {
    service_factory_base* foobar_factory;
    
public:
    service_factory_adapter(service_factory_base* foobar_factory)
        : foobar_factory(foobar_factory) {}
    
    service_ptr create_service() override {
        // 从 foobar2000 插件创建服务实例
        return foobar_factory->create_service();
    }
    
    const GUID& get_guid() const override {
        return foobar_factory->get_guid();
    }
};
```

#### 加载流程

```text
1. LoadLibrary("foo_input_flac.dll")
   ↓
2. GetProcAddress("_foobar2000_client_entry")
   ↓
3. 获取服务工厂列表
   ↓
4. 对每个服务工厂：
   - 包装为本机 ServiceFactoryAdapter
   - 注册到 ServiceRegistry
   ↓
5. 当请求服务时：
   - 查询适配器
   - 委托给 foobar2000 服务工厂
   - 返回包装后的实例
```

#### 任务清单
- [ ] **Week 1**:
  - [ ] 实现 Windows DLL 加载（LoadLibrary, GetProcAddress）
  - [ ] 解析 foobar2000 服务条目格式
  - [ ] 创建 ServiceFactoryAdapter
  - [ ] 基础错误处理（DLL 缺失、导出符号缺失）
  
- [ ] **Week 2**:
  - [ ] 测试用 foo_input_std.dll（foobar2000 内置组件）
  - [ ] 验证服务创建流程
  - [ ] 实现卸载机制（引用计数归零时 FreeLibrary）
  - [ ] 集成到现有的 PluginHost
  
- [ ] **Week 3**:
  - [ ] 测试第三方插件（foo_input_flac, foo_dsp_eq 等）
  - [ ] 调试调用约定问题（__thiscall vs __stdcall）
  - [ ] 修复 vtable 布局不兼容

#### 已知技术挑战

1. **调用约定不匹配**
   - foobar2000：通常使用 `__thiscall`（成员函数）
   - 需要验证 vtable 布局完全一致
   - 可能需要 thunk 或 trampoline 代码

2. **接口菱形继承**
   - foobar2000 服务通常从多个接口继承
   - 需要确保 dynamic_cast 在 DLL 边界工作
   - 可能需要 RTTI 兼容性层

3. **全局状态冲突**
   - foobar2000 插件可能使用全局/静态变量
   - 可能与播放器核心冲突
   - 需要在隔离的环境中加载？

---

### 🔴 **修复 #3：元数据系统紧急增强**（不兼容率 100%）

**状态**：部分实现  
**影响**：严重 - 元数据无法正确读取或显示  
**预计时间**：2-3 周  
**复杂度**：中等

#### 问题分析

```cpp
// 当前项目的元数据（仅简单键值对）
struct MetadataTag {
    const char* key;      // "artist"
    const char* value;    // "The Beatles"
};

// foobar2000 的元数据（多值字段）
class file_info {
    // 可以包含多个值：
    // tagger::["artist1", "artist2", "artist3"]
    // genre::["rock", "classic rock"]
    // 
    // 特殊字段：
    // %play_count% <内置统计>
    // %last_played% <内置统计>
    // %rating% <用户评分>
};
```

#### 分阶段增强

**阶段 1：多值字段支持（Week 1）**

```cpp
// core/metadata/enhanced_metadata.h
class EnhancedMetadata {
public:
    struct Field {
        std::string name;
        std::vector<std::string> values;  // 多值！
        bool is_multi_value = false;
    };
    
    // 设置字段值（单值或多值）
    void set_field(const std::string& name, const std::string& value);
    void set_field(const std::string& name, const std::vector<std::string>& values);
    
    // 获取字段值
    std::vector<std::string> get_field(const std::string& name) const;
    std::string get_field_first(const std::string& name) const;
    
    // 检查字段是否存在且有值
    bool has_field(const std::string& name) const;
    
private:
    std::unordered_map<std::string, Field> fields_;
};

// 使用示例
EnhancedMetadata meta;
meta.set_field("artist", std::vector<std::string>{"The Beatles", "Paul McCartney"});
meta.set_field("genre", std::vector<std::string>{"rock", "classic rock"});

auto artists = meta.get_field("artist");  // 返回 ["The Beatles", "Paul McCartney"]
```

**阶段 2：统计信息（Play Count, Last Played）（Week 2）**

```cpp
// core/metadata/track_statistics.h
struct TrackStatistics {
    uint32_t play_count = 0;
    uint64_t first_played_timestamp = 0;  // Unix timestamp
    uint64_t last_played_timestamp = 0;
    uint64_t added_timestamp = 0;
    
    float rating = 0.0f;  // 0-5 星
    uint32_t skip_count = 0;
    
    // 持久化到 metadb
    void save(const std::string& track_identifier);
    void load(const std::string& track_identifier);
};

// 集成到现有 TrackInfo
struct EnhancedTrackInfo {
    std::string file_path;
    EnhancedMetadata metadata;
    TrackStatistics stats;
    // ... 其他字段
};
```

**阶段 3：查询引擎（Week 3）**

```cpp
// compat/metadata/query_engine.h
class MetadataQueryEngine {
public:
    // 简单的查询评估
    bool evaluate(const std::string& query, const EnhancedTrackInfo& track);
    
private:
    // 基础解析器（支持简单查询）
    bool parse_and_evaluate(const std::string& query, 
                           const EnhancedTrackInfo& track);
    
    // 支持的操作符
    enum Operator { IS, IS_NOT, GREATER, LESS, AFTER, BEFORE, HAS };
};
```

#### 与 foobar2000 格式的兼容

```cpp
// compat/metadata/metadb_adapter.h
class MetadbAdapter {
public:
    // 从 EnhancedMetadata 创建 file_info（供 foobar2000 插件使用）
    static std::unique_ptr<file_info_impl> to_file_info(
        const EnhancedTrackInfo& track);
    
    // 从 foobar2000 file_info 更新 EnhancedMetadata
    static void from_file_info(EnhancedTrackInfo& track, 
                               const file_info& info);
};
```

#### 任务清单
- [ ] **Week 1**：EnhancedMetadata 类和多值字段
- [ ] **Week 1**：更新 FLAC 解码器以填充多值字段
- [ ] **Week 2**：TrackStatistics 实现
- [ ] **Week 2**：持久化层（SQLite）
- [ ] **Week 3**：基础查询引擎（IS, AND, OR）
- [ ] **Week 3**：集成测试

---

### 🟡 **修复 #4：Titleformat 脚本引擎 - 核心实现**（缺失率 100%）

**状态**：未开始  
**影响**：严重 - UI 自定义完全失效  
**预计时间**：3-4 周  
**复杂度**：高

#### 最小可行实现（支持 80% 的常用功能）

支持的操作符和函数：
```
基本字段: %artist%, %title%, %album%, %tracknumber%
条件: $if(cond, true, false)
条件简写: [text] (仅当字段存在时显示)
函数: $len(), $lower(), $upper(), $pad()
括号: (expression)
```

#### 架构

```cpp
// compat/scripting/titleformat_compiler.h
class TitleformatCompiler {
public:
    // 编译脚本为中间表示
    Result compile(const std::string& script);
    
    // 执行已编译的脚本
    std::string execute(const EnhancedTrackInfo& track);
    
private:
    enum class OpCode {
        PUSH_FIELD,      // 压入字段值（从 track.metadata）
        PUSH_LITERAL,    // 压入字面值
        CONDITIONAL,     // 条件 ?: 操作符
        FUNCTION_CALL,   // 函数调用
        CONCAT,          // 隐式连接
    };
    
    struct Instruction {
        OpCode opcode;
        std::string operand;  // 字段名、字面值或函数名
        std::vector<Instruction> children;  // 用于嵌套表达式
    };
    
    std::vector<Instruction> program_;
    
    // 词法分析
    Result tokenize(const std::string& script, std::vector<Token>& tokens);
    
    // 语法分析
    Result parse(const std::vector<Token>& tokens);
};

// compat/scripting/functions.h
class BuiltinFunctions {
public:
    static std::string if_func(const std::vector<std::string>& args);
    static std::string len_func(const std::vector<std::string>& args);
    static std::string lower_func(const std::vector<std::string>& args);
    static std::string upper_func(const std::vector<std::string>& args);
    static std::string pad_func(const std::vector<std::string>& args);
};
```

#### 简化执行示例

```cpp
// 脚本：%artist% - %title% '['%bitrate% kbps']'
// 编译为：
// PUSH_FIELD "artist"
// PUSH_LITERAL " - "
// PUSH_FIELD "title"
// CONCAT (3 values)
// PUSH_LITERAL " ["
// PUSH_FIELD "bitrate"
// PUSH_LITERAL " kbps]"
// CONCAT (3 values)
// CONDITIONAL (if bitrate exists, include bracket part)
// CONCAT (final string)

std::string TitleformatCompiler::execute(const EnhancedTrackInfo& track) {
    std::stack<std::string> stack;
    
    for (const auto& instr : program_) {
        switch (instr.opcode) {
            case OpCode::PUSH_FIELD: {
                auto values = track.metadata.get_field(instr.operand);
                stack.push(values.empty() ? "" : values[0]);
                break;
            }
            case OpCode::PUSH_LITERAL:
                stack.push(instr.operand);
                break;
            case OpCode::CONCAT: {
                std::string result;
                // 从栈中弹出多个值并连接
                int count = std::stoi(instr.operand);
                std::vector<std::string> parts;
                for (int i = 0; i < count; ++i) {
                    parts.push_back(stack.top());
                    stack.pop();
                }
                // 逆序连接（栈是后进先出）
                for (auto it = parts.rbegin(); it != parts.rend(); ++it) {
                    result += *it;
                }
                stack.push(result);
                break;
            }
            case OpCode::CONDITIONAL: {
                std::string false_part = stack.top(); stack.pop();
                std::string true_part = stack.top(); stack.pop();
                std::string cond = stack.top(); stack.pop();
                stack.push(cond.empty() ? false_part : true_part);
                break;
            }
        }
    }
    
    return stack.empty() ? "" : stack.top();
}
```

#### 任务清单
- [ ] **Week 1**：词法分析器（识别 %field%, $func(), []）
- [ ] **Week 2**：语法树构建和基础执行（字段替换）
- [ ] **Week 3**：条件语句（$if, []）和内置函数
- [ ] **Week 4**：UI 集成和测试套件

---

### 🟡 **修复 #5：音频管道 - DSP 和 ReplayGain 基础**（缺失率 80%）

**状态**：部分 DSP 有存根  
**影响**：中等 - 音频质量受影响  
**预计时间**：2-3 周  
**复杂度**：中等到高

#### 当前状态

```cpp
// plugins/dsp/volume_control_dsp.cpp 是空的存根！
// 需要实现实际的音量控制
```

#### DSP 链架构

```cpp
// compat/audio/dsp_chain_manager.h
class DSPChainManager {
public:
    // 添加 DSP 处理器到链
    void add_dsp(std::unique_ptr<dsp> processor);
    
    // 处理音频块
    void process(audio_chunk& chunk, abort_callback& abort);
    
    // 配置管理
    void load_config(const std::string& config);
    std::string save_config() const;
    
private:
    std::vector<std::unique_ptr<dsp>> dsp_chain_;
    std::mutex mutex_;
};

// compat/audio/replaygain_processor.h
class ReplayGainProcessor {
public:
    struct Settings {
        enum Mode { DISABLED, TRACK, ALBUM, SMART };
        Mode mode = TRACK;
        float preamp_db = 0.0f;  // 前置放大（dB）
        bool prevent_clipping = true;
    };
    
    // 从 metadb 读取 RG 信息并应用增益
    void apply_gain(audio_chunk& chunk, const EnhancedTrackInfo& track);
    
private:
    Settings settings_;
    
    float calculate_gain(const TrackStatistics& stats) const;
    void apply_gain_to_chunk(audio_chunk& chunk, float gain_db);
};
```

#### 具体 DSP 实现

```cpp
// compat/audio/dsp/volume_control.cpp
class VolumeControlDSP : public dsp {
    float volume_linear_ = 1.0f;  // 0.0 to 1.0
    float volume_db_ = 0.0f;      // dB
    
public:
    void run(audio_chunk* chunk, abort_callback& abort) override {
        audio_sample* data = chunk->get_data();
        size_t samples = chunk->get_sample_count() * chunk->get_channels();
        
        for (size_t i = 0; i < samples; ++i) {
            data[i] *= volume_linear_;
        }
    }
    
    void set_volume(float db) {
        volume_db_ = db;
        volume_linear_ = powf(10.0f, db / 20.0f);
    }
};

// compat/audio/dsp/equalizer.cpp
class EqualizerDSP : public dsp {
    // 简单 10 段 EQ
    static constexpr size_t BANDS = 10;
    float gains_db_[BANDS] = {0.0f};
    
    // 滤波器系数（简化）
    struct Filter {
        float freq;
        float q;
        float gain_db;
    } filters_[BANDS];
    
public:
    void run(audio_chunk* chunk, abort_callback& abort) override {
        // 应用每个频段的增益
        // 实际实现需要双二阶滤波器
    }
};
```

#### 集成到播放引擎

```cpp
// 修改 core/playback_engine.cpp
Result PlaybackEngine::initialize(IAudioOutput* audio_output) {
    // 现有代码...
    
    // 初始化 DSP 链
    dsp_chain_manager_ = std::make_unique<DSPChainManager>();
    replaygain_processor_ = std::make_unique<ReplayGainProcessor>();
    
    // 加载配置
    dsp_chain_manager_->load_config(config_->get_string("dsp", "chain", ""));
    
    return Result::Success;
}

void PlaybackEngine::audio_callback(void* buffer, size_t frames, void* user_data) {
    // 现有解码逻辑...
    
    // 创建 audio_chunk 包装解码数据
    audio_chunk_impl chunk;
    chunk.set_data((audio_sample*)buffer, frames, channels, sample_rate);
    
    // 应用 ReplayGain
    if (replaygain_processor_) {
        replaygain_processor_->apply_gain(chunk, current_track_);
    }
    
    // 通过 DSP 链处理
    if (dsp_chain_manager_) {
        abort_callback_impl abort;
        dsp_chain_manager_->process(chunk, abort);
    }
    
    // 继续播放...
}
```

#### 任务清单
- [ ] **Week 1**: DSP 链基础架构和 VolumeControlDSP
- [ ] **Week 1**: ReplayGain 信息读取和应用
- [ ] **Week 2**: Equalizer DSP（基础版本）
- [ ] **Week 2**: 配置管理（保存/加载 DSP 链设置）
- [ ] **Week 3**: 性能优化和低延迟处理
- [ ] **Week 3**: 集成测试

---

## 快速胜利：立即可实施的低悬果实

这些任务可在 1-2 天内完成，能立即改善用户体验：

### 1. 更新文档，诚实说明当前限制

```markdown
# README.md - 在 "Features" 章节添加

## ⚠️ 兼容性状态：部分可用

**当前 foobar2000 兼容性评级：15/100**

| 功能 | 兼容性 | 预估完成时间 |
|------|--------|-------------|
| 加载 foobar2000 插件 | ❌ 未实现 | 4-6 周 |
| 复杂元数据查询 | ❌ 不支持 | 2-3 周 |
| Titleformat 脚本 | ❌ 缺失 | 3-4 周 |
| ReplayGain 支持 | ❌ 未实现 | 2 周 |
| DSP 链 | ❌ 部分 | 2-3 周 |
| 播放列表迁移 | ❌ 不适用 | 待调查 |

**我们目前正在积极实施这些功能。查看 [COMPATIBILITY_RECOVERY_PLAN.md] 了解详情。**
```

### 2. 在 CMake 中添加明确的警告

```cmake
# CMakeLists.txt
if(ENABLE_FOOBAR_COMPAT)
    message(WARNING "
    ═══════════════════════════════════════════════════════
    ⚠️  foobar2000 兼容性警告
    
    当前兼容性层仅包含接口存根，
    无法实际加载 foobar2000 插件。
    
    完整实现预计需要 4-6 个月。
    查看 COMPATIBILITY_RECOVERY_PLAN.md 了解详情。
    ═══════════════════════════════════════════════════════
    ")
endif()
```

### 3. 在启动时显示兼容性状态

```cpp
// src/main.cpp
int main() {
    // ...
    
#ifdef ENABLE_FOOBAR_COMPAT
    std::cout << "═══════════════════════════════════════════════════" << std::endl;
    std::cout << "foobar2000 Compatibility Layer Status:" << std::endl;
    std::cout << "  SDK Interfaces: " << (AreSDKInterfacesImplemented() ? "✅" : "❌") << std::endl;
    std::cout << "  Plugin Loader: " << (IsPluginLoaderImplemented() ? "✅" : "❌") << std::endl;
    std::cout << "  Metadata Support: " << (IsEnhancedMetadataEnabled() ? "✅" : "❌") << std::endl;
    std::cout << "  Titleformat Script: " << (IsTitleformatEnabled() ? "✅" : "❌") << std::endl;
    std::cout << "  DSP/RG: " << (IsDSPChainEnabled() ? "✅" : "❌") << std::endl;
    std::cout << "═══════════════════════════════════════════════════" << std::endl;
#endif
    
    // ...
}
```

---

## 工作计划时间表

### 第 1-2 周：基础 SDK 和加载器
```
Week 1:
  Mon: sdk_implementations/ 目录 + service_base + abort_callback
  Tue: file_info (关键！多值字段) + audio_chunk
  Wed: 字符串工具 + audio_info
  Thu: plugin_loader/ 基础架构
  Fri: DLL 加载函数 + 服务工厂解析

Week 2:
  Mon: 服务注册适配器
  Tue: 测试用 foo_input_std.dll 加载
  Wed: 解决链接错误和 vtable 问题
  Thu: 错误处理 + 文档
  Fri: 代码审查 + 合并
```

### 第 3-4 周：元数据增强
```
Week 3:
  Mon/Tue: EnhancedMetadata 类（多值字段）
  Wed/Thu: FLAC 解码器更新 + stats 实现
  Fri: 持久化层设计

Week 4:
  Mon/Tue: SQLite 集成 + 基础查询
  Wed/Thu: 查询解析器（IS, AND, OR）
  Fri: 集成测试
```

### 第 5-6 周：脚本和音频
```
Week 5:
  Mon/Tue: Titleformat 词法分析
  Wed/Thu: 语法树 + 字段替换
  Fri: 条件表达式

Week 6:
  Mon/Tue: DSP 链架构 + VolumeControl
  Wed/Thu: ReplayGain 实现
  Fri: 集成测试 + 文档更新
```

### 第 7-8 周：硬化和测试
```
Week 7:
  Mon/Tue: 测试套件扩展
  Wed/Thu: Bug 修复
  Fri: 性能优化

Week 8:
  Mon/Tue: 文档完善
  Wed/Thu: 用户反馈收集
  Fri: 小版本发布
```

---

## 资源分配建议

### 最小可行团队（4 人）

1. **工程师 A**：SDK 接口和加载器（核心）
2. **工程师 B**：元数据和查询引擎
3. **工程师 C**：Titleformat 脚本引擎
4. **工程师 D**：音频管道 + 集成测试

### 需要的技能

- **C++17**: 必须精通模板元编程和 ABI
- **Win32 API**: DLL 加载、COM 接口、线程模型
- **音频处理**: DSP、数字信号处理基础
- **解析器编写**: 递归下降、LALR 等
- **SQLite**: 用于元数据存储

---

## 成功指标

### 短期（8 周后）
- [ ] 至少 1 个 foobar2000 插件可加载并工作
- [ ] 多值字段元数据正确显示
- [ ] 简单 Titleformat 脚本（%artist% - %title%）工作
- [ ] ReplayGain 应用基本增益
- [ ] 兼容性评级：45-55/100

### 长期（6 个月后）
- [ ] 80% 的常用 foobar2000 插件工作
- [ ] 复杂查询语法（%album artist% IS X AND %date% AFTER Y）
- [ ] 完整 Titleformat 功能（所有函数和操作符）
- [ ] 稳定的 DSP 链（EQ、重采样等）
- [ ] 兼容性评级：≥80/100

---

## 附录：紧急联系人

```
技术负责人: [TBD]
项目管理员: [TBD]

每日站会时间: 09:00 UTC
每周代码审查: 周五 15:00 UTC
```

### 相关资源
- [Foobar2000 SDK](https://wiki.hydrogenaud.io/index.php?title=Foobar2000_SDK)
- [当前架构文档](.qoder/quests/music-player-development.md)
- [完整修复计划](COMPATIBILITY_RECOVERY_PLAN.md)

### 术语表

- **Titleformat**：foobar2000 的声明式脚本语言，用于格式化显示字符串
- **Metadb**：foobar2000 的元数据库，支持复杂查询和统计
- **Service**：foobar2000 的插件接口模式，类似 COM
- **DSP**：数字信号处理
- **ReplayGain**：标准化音频文件响度的标准

---

**文档版本**: 1.0
**最后更新**: 2025-12-09
**作者**: 技术架构团队
**状态**: 紧急执行
