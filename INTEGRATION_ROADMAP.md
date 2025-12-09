# foobar2000 兼容性 - 集成路线图

## 当前状态 (截至 2025-12-09)

### 已完成 ✅

**修复 #1: SDK 基础接口** (100%)
- service_base (引用计数)
- abort_callback (操作取消)
- file_info (多值元数据) [关键!]
- audio_chunk (音频数据)
- metadb_handle (数据库句柄)
- 测试套件 (12+ 测试)

**代码量**: ~3,100 行

**修复 #2: 插件加载器** (80%)
- FoobarPluginLoader 架构
- Windows DLL 加载 (LoadLibrary)
- ServiceFactoryWrapper
- ServiceRegistryBridge
- 基础错误处理

**代码量**: ~1,371 行

## 待完成 ⏳

### 修复 #3: 增强元数据系统 (Week 1)
**目标**: 替换简单键值对为完整 metadb

**工作项**:
- EnhancedMetadb 类 (handle 缓存, 查询接口)
- QueryParser (查询字符串到 AST, 评估)
- SQLite 集成 (统计持久化)
- 与现有元数据系统桥接

**预计代码量**: ~1,500 行

### 修复 #4: Titleformat 脚本引擎 (Week 2)
**目标**: 实现声明性脚本语言用于 UI

**优先级**:
1. 字段替换 (%artist% → "Queen") - P0
2. 条件 ($if) - P0
3. 括号操作符 [] - P0
4. 基础函数 - P1

**预计代码量**: ~2,000 行

### 修复 #5: 音频管道 DSP (Week 3)
**目标**: 实现 DSP 链和 ReplayGain

**工作项**:
- DSPChainManager (DSP 链管理)
- ReplayGainProcessor (音量标准化)
- VolumeControlDSP (基础 DSP)
- EqualizerDSP (10 段 EQ)

**预计代码量**: ~1,200 行

## 时间线

Week 1 (完成修复 #3):
  - EnhancedMetadb 实现
  - 基础查询功能
  - SQLite 集成

Week 2 (完成修复 #4):
  - Titleformat 解释器
  - 字段替换和条件
  - UI 集成

Week 3 (完成修复 #5):
  - DSP 架构
  - ReplayGain
  - 音量控制

Week 4 (集成测试):
  - 端到端测试
  - Bug 修复
  - 性能优化

## 成功标准

✅ 至少 1 个 foobar2000 插件加载并工作
✅ 多值元数据显示正确
✅ Titleformat 基础功能 (%artist% - %title%)
✅ ReplayGain 应用增益
✅ 兼容性评级 ≥45/100

## 资源

- 人力: 4 名工程师 (1 名已在 SDK 工作)
- 预算: $38,600 (8 周)
- 工具: Visual Studio 2019, SQLite, 测试设备

## 风险

HIGH: 团队分配延迟
MEDIUM: ABI 兼容性复杂性
LOW: 性能开销

## 下一步

立即任务:
1. 分配工程师到修复 #3-5
2. 建立 Git 分支 feature/foobar-compat
3. 配置 CI/CD
4. 开始 Week 1 实施
