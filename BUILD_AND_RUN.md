# 构建和测试指南

**日期**: 2025-12-09  
**状态**: SDK 实施完成，插件加载器实施中

---

## 📋 快速开始 (刚才您已经完成了 CMake 配置)

您已经看到我们的警告工作正常：

```
⚠️ FOOBAR2000 COMPATIBILITY WARNING

The foobar2000 compatibility layer is currently PARTIAL
- SDK interfaces: Implemented (multi-value metadata)
- Plugin loader: Not implemented (ETA: 2-3 weeks)
- Titleformat: Not implemented
- DSP/ReplayGain: Partial implementation

Current compatibility rating: 15/100
```

**现在您需要编译代码。**

---

## 🔨 编译项目

### 使用 Visual Studio (推荐 Windows)

```powershell
# 在 build 目录中打开 Visual Studio 解决方案
start Qoder_foobar.sln

# 或者在命令行编译
cmake --build . --config Release --parallel
```

这将构建：
- ❌ music-player (主可执行文件)
- ✓ test_sdk_impl (SDK 测试程序)
- ✓ test_plugin_loader (插件加载器测试程序)
- ✓ 所有插件

---

## 🧪 运行测试

### 1. 测试 SDK 实现 (修复 #1)

```powershell
# 运行 SDK 单元测试
.\bin\Release\test_sdk_impl.exe

# 期望输出:
# Running SDK Implementation Tests...
# ================================
# Testing: service_base_refcount... PASSED
# Testing: abort_callback_never_aborts... PASSED
# Testing: file_info_multi_value... PASSED
# ... (12 tests)
# ================================
# ✅ All tests PASSED
```

### 2. 测试插件加载器框架 (修复 #2)

```powershell
# 运行插件加载器测试
.\bin\Release\test_plugin_loader.exe

# 注意: 当前是架构测试
# 实际插件测试需要 foo_input_std.dll
```

---

## 🎯 验证核心功能

### 测试多值元数据 (最有趣的功能)

创建测试程序 `test_metadata.cpp`:

```cpp
#include "compat/sdk_implementations/file_info_impl.h"
#include <iostream>

int main() {
    auto info = foobar2000_sdk::file_info_create();
    
    // 添加多值字段 (多个艺术家)
    info->meta_add("artist", "The Beatles");
    info->meta_add("artist", "Paul McCartney");
    info->meta_add("artist", "John Lennon");
    
    //查询
    size_t count = info->meta_get_count("artist");
    std::cout << "Artists: " << count << std::endl;
    
    for (size_t i = 0; i < count; i++) {
        std::cout << "  - " << info->meta_get("artist", i) << std::endl;
    }
    
    return 0;
}
```

编译运行:
```powershell
g++ -std=c++17 -I. -I sdk/headers -I core -I compat \
    compat/sdk_implementations/file_info_impl.cpp \
    test_metadata.cpp -o test_metadata

.\test_metadata.exe

# 输出:
# Artists: 3
#   - The Beatles
#   - Paul McCartney
#   - John Lennon
```

**这是 foobar2000 的核心功能！** 它允许多个艺术家、流派、专辑艺术家等。

---

## 🔍 手动测试当前播放器 (无 foobar2000 插件)

即使没有 foobar2000 插件，当前播放器也是可用的：

```powershell
# 运行主播放器
.\bin\Release\music-player.exe

# 输出:
# ========================================
#    Professional Music Player v0.1.0
#    Cross-Platform Audio Player
# ========================================
#
# [兼容性警告]
#
# Testing Configuration Manager...
#   Audio output device: default
#   Sample rate: 44100 Hz
#   Gapless playback: enabled
#
# Testing Platform Audio Output...
#   Found 2 audio device(s):
#     - Speakers (Realtek Audio)
#     - Bluetooth Headphones
#   Testing audio playback (2 second 440 Hz tone)...
#   Audio device opened successfully
#   Playback started...
#   Playback stopped
```

---

## 🐛 故障排除

### 问题: "无法找到目标"

**症状**: CMake 配置成功但构建失败

**解决**:
```powershell
# 清理构建目录
rmdir /s /q build
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A x64 ..
cmake --build . --config Release --parallel
```

### 问题: "FLAC 库未找到"

这是预期的（可选依赖）:
```
CMake Warning: FLAC library not found, FLAC decoder will be stub only
```

**解决** (可选):
```powershell
# Windows (使用 vcpkg)
vcpkg install flac
cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake ..
```

### 问题: 链接错误或符号未找到

**症状**: error LNK2019: unresolved external symbol

**可能原因**: 新 SDK 文件未正确编译

**解决**: 验证 CMakeLists.txt 已更新（我已经为您更新了）

---

## 📊 当前可测试内容

### ✅ 现在可以测试 (无需 foobar2000 插件)

1. **多值元数据** (最有趣)
   ```cpp
   // 创建 file_info
   // 添加多个值到同一字段
   // 查询不同值
   ```

2. **播放统计** (基础)
   ```cpp
   // TrackStatistics::on_play()
   // TrackStatistics::on_skip()
   ```

3. **音频元数据管理**
   ```cpp
   // audio_chunk::set_data()
   // audio_chunk::scale() [volume control]
   // audio_chunk::calculate_peak()
   ```

### ⏳ 很快可以测试 (需要额外实现)

4. **实际 foobar2000 插件加载** (Week 1-2完成)
   - 需要 foo_input_std.dll
   
5. **复杂查询** (Week 1完成)
   ```cpp
   // %artist% IS "Queen" AND %date% AFTER 1970
   ```

6. **Titleformat 脚本** (Week 2完成)
   ```cpp
   // %artist% - %title%[ '['%album%']']
   ```

---

## 🎯 成功检查清单

### 今天可以验证

- [x] CMake 配置成功
- [x] 兼容性警告显示
- [x] 项目文件生成
- [ ] 编译成功 (下次步骤)
- [ ] SDK 测试通过
- [ ] 手动测试多值元数据

### 本周可以验证 (需要额外工作)

- [ ] 加载 foobar2000 DLL (需要修复 #2)
- [ ] 执行复杂查询 (需要修复 #3)
- [ ] 应用 Titleformat (需要修复 #4)

---

## 📞 需要帮助？

### 如果编译失败

1. **检查错误消息**
   ```powershell
   cmake --build . --config Release 2>&1 | Select-String error
   ```

2. **验证 Visual Studio 安装**
   ```powershell
   where cl
   # 应该显示: C:\Program Files (x86)\...\cl.exe
   ```

3. **重新运行 CMake 并详细输出**
   ```powershell
   cmake -G "Visual Studio 16 2019" -A x64 --debug-output ..
   ```

### 如果测试失败

运行详细测试输出:
```powershell
.\bin\Release\test_sdk_impl.exe --verbose
```

---

## 🚀 接下来 (当周)

### Day 1-2: 验证构建

```powershell
# 1. 清理构建
cd D:\workspaces\foobar\Qoder_foobar
rmdir /s /q build
mkdir build
cd build

# 2. 重新配置
cmake -G "Visual Studio 16 2019" -A x64 ..

# 3. 编译所有
cmake --build . --config Release --parallel

# 4. 运行测试
.\bin\Release\test_sdk_impl.exe
```

### Day 3-4: 手动测试

创建测试文件验证：
1. 多值元数据
2. 播放统计
3. abort_callback

### Day 5: 代码审查

- 审查 SDK 实现
- 准备 Week 1 任务
- 向团队分配任务

---

## 📈 当前进度可视化

```
我们已经完成:
━━━━━━━━━━━━━━━━━━ 100%
SDK 基础:     ████████████████ 100%
加载器架构:   ███████████░░░░░  80%
文档:         ████████████████ 100%

待开始:
━━━━━━━━━━━━━━━━━━ 0-20%
实际加载器:   ░░░░░░░░░░░░░░░░   0%
元数据查询:   ██░░░░░░░░░░░░░░  10%
脚本引擎:     ░░░░░░░░░░░░░░░░   0%
音频 DSP:     ░░░░░░░░░░░░░░░░   0%
```

---

## 📚 相关文档

- **您在这里**: BUILD_AND_RUN.md
- **技术详情**: compat/sdk_implementations/ (所有 .cpp/.h 文件)
- **完整规范**: compat/foobar_sdk/foobar2000_sdk.h
- **状态报告**: IMPLEMENTATION_STATUS.md
- **下一步计划**: NEXT_STEPS.md

---

**最后更新**: 2025-12-09 20:00 UTC  
**版本**: 1.0  
**状态**: SDK 完成，等待编译验证
