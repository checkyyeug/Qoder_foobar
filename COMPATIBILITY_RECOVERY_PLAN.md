# foobar2000 兼容性修复计划

## 执行摘要

基于深度技术分析，当前音乐播放器与 foobar2000 的兼容性评级仅为 **15/100**（严重不兼容）。本项目旨在通过跨领域的系统性重构，将兼容性提升至可接受水平（≥80/100）。

## 当前状态评估

### 已验证的严重问题

| 问题域 | 当前状态 | 影响程度 | 优先级 |
|--------|----------|----------|--------|
| 兼容层实现 | 仅空框架，无实际适配器 | 阻塞性 | P0 |
| 元数据系统 | 简单键值对，无 metadb | 严重 | P0 |
| 脚本系统 | 完全缺失 | 严重 | P1 |
| 内存管理ABI | 现代RAII vs 引用计数 | 严重 | P1 |
| 线程模型 | 过度简化 | 中等 | P2 |
| 音频管道 | 缺少 DSP/RG/重采样 | 严重 | P1 |
| 配置系统 | JSON vs cfg_*变量 | 中等 | P2 |

### 架构不匹配分析

```
foobar2000 架构:
├─ metadb (复杂元数据库)
├─ Titleformat 脚本引擎
├─ 引用计数内存模型
├─ 多线程架构 (UI/解码/音频/元数据线程)
├─ 完整音频处理链 (input→DSP→RG→output)
└─ 全局配置系统

当前项目架构:
├─ 简单键值对元数据
├─ 无脚本支持
├─ 现代 C++ RAII
├─ 简化线程模型
├─ 简单处理链 (decoder→playback→output)
└─ JSON 配置文件
```

## 分阶段修复策略

### 第一阶段：基础兼容性层（P0）

**目标**：实现可加载 foobar2000 插件的基本能力

#### 1.1 内存管理兼容层
```cpp
// 引用计数适配器
class RefCountedAdapter {
    template<typename T>
    class service_ptr_t {
        T* ptr;
        void add_ref() { if (ptr) ptr->service_add_ref(); }
        void release() { if (ptr) ptr->service_release(); }
    };
    
    // 自定义分配器桥接
    class pfc::allocator : public std::allocator<char> {
        // 桥接到项目内存管理
    };
};
```

**实现要点：**
- 桥接 foobar2000 `service_base` 到项目基础设施
- 实现引用计数与 RAII 的智能指针互操作
- 提供 pfc::allocator 兼容层

#### 1.2 核心 SDK 接口实现
```cpp
// 在 compat/sdk_interfaces/ 目录下
class abort_callback_impl : public abort_callback {
    std::atomic<bool> aborted{false};
    std::function<bool()> callback;
public:
    bool is_aborting() const override { return aborted; }
};

class file_info_impl : public file_info {
    // 多值字段支持
    std::unordered_map<std::string, std::vector<std::string>> meta_fields;
    audio_info_impl audio_info;
};
```

**详细任务：**
- [ ] 实现 `abort_callback`、`file_info`、`audio_chunk` 等基础接口
- [ ] 创建内存分配器桥接层
- [ ] 实现服务注册表互操作
- [ ] 添加 ABI 调用约定适配器

#### 1.3 动态加载机制
```cpp
class FoobarPluginLoader {
    // Windows: LoadLibrary + GetProcAddress
    // 处理 foobar2000 特定的导出符号
    
    struct PluginModule {
        HMODULE handle;
        std::vector<service_factory*> service_factories;
    };
};
```

### 第二阶段：元数据分析与重构（P0）

**目标**：实现 metadb 兼容层和复杂查询

#### 2.1 Metadb 兼容层
```cpp
// compat/metadata/metadb_adapter.h
class metadb_handle_impl : public metadb_handle {
    // 内部包装 native track handle
    std::shared_ptr<Track> native_track;
    file_info_impl cached_info;
    
public:
    // 实现 foobar2000 接口
    const file_info& get_info() override;
    metadb* get_db() override;
};

class metadb_impl : public metadb {
    // 索引和缓存
    std::unordered_map<std::string, std::shared_ptr<metadb_handle_impl>> handle_cache;
    
    // 直接在 native 库上执行查询
    QueryResult query(const char* query_string) override;
};
```

**关键差异处理：**
- 支持 foobar2000 的统计信息（播放次数、最后播放时间）
- 实现字段名映射（`%artist%` → `artist`）
- 多值字段序列化/反序列化
- 复杂查询解析器（Q 语法）

#### 2.2 查询引擎
```cpp
// compat/metadata/query_parser.h
class QueryParser {
    // %album artist% IS "The Beatles" AND %date% AFTER 1965
    
    struct QueryNode {
        enum Type { FIELD, OPERATOR, VALUE, LOGICAL_OP };
        std::string field_name;  // 带 % 符号
        std::string op;          // IS, AFTER, BEFORE, GREATER, LESS
        std::string value;
        std::vector<QueryNode> children;  // 用于 AND/OR
    };
    
    QueryNode parse(const std::string& query);
};
```

**详细任务：**
- [ ] 创建元数据中间层，同时支持两种模型
- [ ] 实现查询解析器和执行引擎
- [ ] 添加统计信息持久化
- [ ] 支持实时元数据更新
- [ ] 集成到播放列表管理器

### 第三阶段：脚本引擎（P1）

**目标**：实现 Titleformat 脚本支持

#### 3.1 Titleformat 解析器与虚拟机
```cpp
// compat/scripting/titleformat_vm.h
class TitleformatVM {
    // 字节码指令
    enum Opcode {
        OP_PUSH_FIELD,      // 压入字段值
        OP_PUSH_LITERAL,    // 压入字面值
        OP_IF,              // 条件判断
        OP_CONDITIONAL,     // ? 操作符
        OP_FUNC_CALL,       // 函数调用 $function()
        OP_CONCAT,          // 字符串连接
    };
    
    std::vector<uint8_t> bytecode;
    std::vector<std::string> literals;
    
public:
    // 编译脚本
    Result compile(const std::string& script);
    
    // 执行脚本
    std::string execute(metadb_handle* track);
};

// 函数表
class FunctionRegistry {
    std::unordered_map<std::string, std::function<std::string(const std::vector<std::string>&)>> functions;
    
    // 内置函数
    void register_builtin_functions() {
        functions["if"] = [](const auto& args) -> std::string {
            return !args.empty() && !args[0].empty() ? args[0] : "";
        };
        
        functions["len"] = [](const auto& args) -> std::string {
            return std::to_string(args.empty() ? 0 : args[0].length());
        };
        
        // 更多函数...
    }
};
```

#### 3.2 脚本与 UI 集成
```cpp
// UI 列格式器
class ColumnFormatProvider {
    TitleformatVM vm;
    std::string script;
    
public:
    std::string format_track(metadb_handle* track) {
        return vm.execute(track);
    }
};

// 播放列表标题格式化
class PlaylistTitleFormatter {
    // 使用脚本生成播放列表项显示文本
};
```

**详细任务：**
- [ ] 词法分析和语法解析器
- [ ] 字节码编译器和虚拟机
- [ ] 内置函数库（约 50+ 个函数）
- [ ] 与 metadb 的字段解析集成
- [ ] UI 绑定（播放列表、状态栏等）
- [ ] 脚本调试和错误报告

### 第四阶段：线程模型重构（P1-P2）

**目标**：匹配 foobar2000 的多线程架构

#### 4.1 线程架构适配
```cpp
// compat/threading/thread_manager.h
class ThreadManager {
    enum ThreadType {
        MAIN_THREAD,      // UI 和消息处理
        DECODER_THREAD,   // 后台解码
        AUDIO_THREAD,     // 实时音频输出
        METADB_THREAD,    // 元数据操作
    };
    
    // 线程本地存储，用于识别当前线程类型
    static thread_local ThreadType current_thread_type;
    
    // 确保在特定线程上执行
    void run_on_thread(ThreadType type, std::function<void()> task);
};

// 阻塞调用适配器
class ThreadBlockingAdapter {
    // foobar2000 大量依赖阻塞调用和事件循环
    // 需要创建消息泵或纤程机制
};
```

#### 4.2 异步操作兼容
```cpp
// 回调适配器
class MainThreadCallback {
    // 从工作线程发布到主线程
    virtual void callback_run() = 0;
};
```

**详细任务：**
- [ ] 重构播放引擎为多线程架构
- [ ] 实现线程安全的元数据访问
- [ ] 创建线程间通信机制
- [ ] 添加阻塞调用兼容性
- [ ] 实现主线程回调系统

### 第五阶段：音频管道增强（P1）

**目标**：实现完整的音频处理链

#### 5.1 DSP 链架构
```cpp
// compat/audio/dsp_chain_adapter.h
class DSPChainImpl : public dsp_chain {
    std::vector<service_ptr_t<dsp>> dsp_chain;
    
    // 音频处理
    void run(audio_chunk* chunk, abort_callback& abort) override {
        for (auto& dsp : dsp_chain) {
            dsp->run(chunk, abort);
        }
    }
};

// 音量控制适配器
class ReplayGainAdapter {
    // 从 metadb 读取 RG 信息
    // 应用音轨/专辑增益
};
```

#### 5.2 重采样与格式转换
```cpp
class ResamplerManager {
    // 集成 libsamplerate 或 speexdsp
    // 匹配 foobar2000 的重采样质量设置
};
```

**详细任务：**
- [ ] DSP 接口和插件加载器
- [ ] ReplayGain 处理
- [ ] 高质量重采样
- [ ] 格式转换管道
- [ ] 实时路由和配置

## 集成方案

### 构建系统集成
```cmake
# CMakeLists.txt 更新
if(ENABLE_FOOBAR_COMPAT)
    # 添加所有新的兼容性子目录
    add_subdirectory(compat/sdk_interfaces)
    add_subdirectory(compat/metadata)
    add_subdirectory(compat/scripting)
    add_subdirectory(compat/threading)
    add_subdirectory(compat/audio)
    
    # 依赖库
    find_package(FFTW REQUIRED)  # 用于 DSP
    find_package(SampleRate REQUIRED)  # 重采样
endif()
```

### 运行时初始化
```cpp
// 修改后的初始化流程
int main() {
    CoreEngine engine;
    engine.initialize();
    
    // 初始化兼容性层
#ifdef ENABLE_FOOBAR_COMPAT
    FoobarCompatManager compat_manager;
    compat_manager.initialize(config);
    
    // 设置服务桥接
    ServiceBridge::install(&engine, &compat_manager);
#endif
    
    // 加载 foobar2000 插件
    compat_manager.scan_plugins("C:/foobar2000/components");
}
```

## 测试策略

### 单元测试覆盖
```cpp
// 每个模块需要独立测试
TEST_SUITE("foobar_sdk_interfaces") {
    TEST("service_ptr_t manages reference count correctly");
    TEST("abort_callback can abort operations");
}

TEST_SUITE("metadb_adapter") {
    TEST("metadb_handle provides correct file_info");
    TEST("Query parser handles complex queries");
    TEST("Multi-value fields serialized correctly");
}

TEST_SUITE("titleformat_vm") {
    TEST("Simple field substitution works");
    TEST("Conditional expressions evaluate correctly");
    TEST("Function calls execute with proper arguments");
}

TEST_SUITE("dsp_chain") {
    TEST("DSP chain processes audio in correct order");
    TEST("ReplayGain applies correct gain");
}
```

### 集成测试
```cpp
// 使用真实 foobar2000 插件进行测试
TEST("Load actual foobar2000 FLAC decoder") {
    auto* loader = get_foobar_plugin_loader();
    auto plugin = loader->load("foo_input_flac.dll");
    
    // 打开测试文件
    service_ptr_t<input_decoder> decoder;
    plugin->open_decoder(decoder, "test.flac", abort);
    
    // 验证解码功能
    audio_chunk chunk;
    CHECK(decoder->decode_run(chunk, abort));
}

TEST("Execute complex titleformat script") {
    std::string script = "%artist% - %title% '['%bitrate% kbps']'";
    auto track = metadb->handle_create("test.flac");
    
    TitleformatVM vm;
    vm.compile(script);
    std::string result = vm.execute(track);
    
    CHECK(result.contains("kbps"));
}
```

### 性能基准测试
```cpp
// 确保与原生 foobar2000 性能相当
BENCHMARK("Decode performance vs foobar2000") {
    // 对比解码速度
}

BENCHMARK("Query execution speed") {
    // 对比大库的查询性能
}

BENCHMARK("Titleformat evaluation") {
    // 对比脚本执行速度
}
```

## 时间线与里程碑

### 阶段 1：基础层（6-8 周）
- **周 1-2**：SDK 接口实现
- **周 3-4**：内存管理和 ABI 适配
- **周 5-6**：插件加载器
- **周 7**：集成测试
- **周 8**：文档和示例

**交付物**：可加载简单 foobar2000 插件的测试程序

### 阶段 2：元数据（8-10 周）
- **周 1-2**：metadb_handle 和 file_info 实现
- **周 3-4**：元数据索引和缓存
- **周 5-6**：查询解析器和执行引擎
- **周 7-8**：统计信息集成
- **周 9-10**：性能优化和测试

**交付物**：可执行复杂查询的元数据库

### 阶段 3：脚本（10-12 周）
- **周 1-2**：词法分析和语法设计
- **周 3-4**：编译器实现
- **周 5-6**：内置函数库
- **周 7-8**：UI 集成
- **周 9-10**：调试和错误处理
- **周 11-12**：测试套件

**交付物**：完整的 Titleformat 引擎

### 阶段 4：多线程（6-8 周）
- **周 1-2**：线程管理器
- **周 3-4**：播放引擎重构
- **周 5-6**：线程安全保证
- **周 7-8**：性能调优

**交付物**：稳定的多线程播放引擎

### 阶段 5：音频处理（8-10 周）
- **周 1-2**：DSP 接口和基础插件
- **周 3-4**：ReplayGain 集成
- **周 5-6**：重采样引擎
- **周 7-8**：DSP 链管理
- **周 9-10**：音频质量保证测试

**交付物**：功能完整的音频处理链

### 集成与稳定（4 周）
- **周 1-2**：端到端集成
- **周 3-4**：Bug 修复和性能优化

### 总耗时：42-52 周（10-13 个月）

## 风险评估

### 高风险
1. **ABI 不兼容性**：即使适配器也可能不稳定
   - **缓解**：大量使用自动化测试，逐步启用功能
2. **性能下降**：兼容性层引入开销
   - **缓解**：基准测试，性能分析，关键路径优化
3. **复杂插件失败**：依赖深层内部 API
   - **缓解**：分级支持策略，核心插件优先

### 中风险
1. **线程安全问题**：C++ 模型差异
   - **缓解**：严格的线程安全规则和审查
2. **内存泄漏**：引用计数管理错误
   - **缓解**：泄漏检测工具，RAII 包装器
3. **用户期望**：无法 100% 兼容
   - **缓解**：清晰的文档，已知限制列表

## 资源需求

### 人员
- **架构师（1 人）**：总体设计，SDK 接口（全职）
- **C++ 专家（2 人）**：核心兼容层（全职）
- **音频工程师（1 人）**：DSP 和音频管道（全职）
- **测试工程师（1 人）**：自动化测试（全职）
- **文档工程师（0.5 人）**：技术文档和用户指南（兼职）

### 工具和预算
- **Visual Studio Professional**：$1,200/许可
- **CLion**（可选）：$200/年/许可
- **静态分析工具**（PVS-Studio）：$2,500/年
- **性能分析工具**（Intel VTune）：$900/年
- **测试基础设施**：约 $5,000 初始设置
- **持续集成**：GitHub Actions / Azure DevOps

### 许可证考量
- **libsamplerate**：BSD 风格（可接受）
- **FFTW**：GPL（需要商业许可用于闭源）
- **speexdsp**：BSD（推荐替代）
- **Vorbis**：BSD 风格

## 成功标准

### 功能完整性
- [ ] 可加载 80% 的 foobar2000 输入插件
- [ ] 95% 的 Titleformat 脚本功能工作
- [ ] 复杂查询性能在 80% 的 foobar2000 速度内
- [ ] ReplayGain 和 DSP 链功能正常

### 稳定性
- [ ] 可连续播放 24 小时无崩溃
- [ ] 内存泄漏 <1MB/小时
- [ ] 线程安全测试全部通过
- [ ] 崩溃率 <0.1%（优于 foobar2000 历史版本）

### 用户体验
- [ ] 用户调查显示配置迁移满意度 >70%
- [ ] 插件兼容性按需查显示 >90% 满意度
- [ ] 性能不被大部分用户认为退步

## 附录

### 相关文档链接
- [Foobar2000 SDK 文档](https://wiki.hydrogenaud.io/index.php?title=Foobar2000_SDK)
- [Titleformat 参考](https://wiki.hydrogenaud.io/index.php?title=Foobar2000:Title_Formatting_Reference)
- [Query Syntax](https://wiki.hydrogenaud.io/index.php?title=Foobar2000:Query_syntax)
- [当前项目架构](.qoder/quests/music-player-development.md)

### 技术参考资料
- Windows COM 互操作：IUnknown, IClassFactory
- Service Provider Pattern
- 插件架构设计模式（Erich Gamma）
- Real-Time Audio Processing 最佳实践

---

**创建日期**: 2025-12-09
**最后更新**: 2025-12-09
**作者**: 技术架构团队
**文档版本**: 1.0
