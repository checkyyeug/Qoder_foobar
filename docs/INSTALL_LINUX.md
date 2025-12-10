# Linux 安装指南

本文档介绍如何在Linux系统上安装音乐播放器所需的所有依赖。

## 系统要求

- Linux发行版 (Ubuntu, Debian, CentOS, RHEL, Fedora等)
- CMake 3.20 或更高版本
- C++17 兼容的编译器 (GCC 7+ 或 Clang 5+)

## 快速安装

### Ubuntu/Debian 系统

```bash
# 更新包管理器
sudo apt-get update

# 安装基础依赖
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config

# 安装音频库
sudo apt-get install -y \
    libasound2-dev \
    libpulse-dev

# 安装音频格式支持
sudo apt-get install -y \
    libflac-dev \
    libmp3lame-dev \
    libvorbis-dev \
    libogg-dev

# 安装其他可选依赖
sudo apt-get install -y \
    libopenmp-dev \
    libsdl2-dev
```

### CentOS/RHEL 8+ 系统

```bash
# 启用 EPEL 仓库
sudo dnf install epel-release

# 安装开发工具组
sudo dnf groupinstall -y "Development Tools"

# 安装基础依赖
sudo dnf install -y \
    cmake \
    git \
    pkgconfig

# 安装音频库
sudo dnf install -y \
    alsa-lib-devel \
    pulseaudio-libs-devel

# 安装音频格式支持
sudo dnf install -y \
    flac-devel \
    libmp3lame-devel \
    libvorbis-devel \
    libogg-devel

# 安装其他可选依赖
sudo dnf install -y \
    libgomp-devel \
    SDL2-devel
```

### Fedora 系统

```bash
# 安装基础依赖
sudo dnf install -y \
    gcc-c++ \
    cmake \
    git \
    pkgconfig

# 安装音频库
sudo dnf install -y \
    alsa-lib-devel \
    pulseaudio-libs-devel

# 安装音频格式支持
sudo dnf install -y \
    flac-devel \
    libmp3lame-devel \
    libvorbis-devel \
    libogg-devel

# 安装其他可选依赖
sudo dnf install -y \
    libgomp-devel \
    SDL2-devel
```

### Arch Linux

```bash
# 安装基础依赖
sudo pacman -S --needed \
    base-devel \
    cmake \
    git \
    pkgconf

# 安装音频库
sudo pacman -S \
    alsa-lib \
    pulseaudio

# 安装音频格式支持
sudo pacman -S \
    flac \
    lame \
    libvorbis \
    libogg

# 安装其他可选依赖
sudo pacman -S \
    openmp \
    sdl2
```

## 验证安装

安装完成后，运行以下命令验证：

```bash
# 检查ALSA
pkg-config --modversion alsa

# 检查FLAC
pkg-config --modversion flac

# 运行项目测试
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/bin/test_cross_platform
```

## 故障排除

### 问题1: ALSA 找不到

```bash
# 检查ALSA是否安装
dpkg -l | grep libasound2-dev  # Debian/Ubuntu
rpm -qa | grep alsa-lib-devel  # RHEL/CentOS

# 如果没有安装，运行对应的安装命令
```

### 问题2: 编译错误

```bash
# 确保使用正确的编译器
gcc --version  # 应该是7.0+
clang --version  # 或使用clang

# 确保CMake版本足够
cmake --version  # 应该是3.20+
```

### 问题3: 权限问题

如果遇到音频权限问题：

```bash
# 将用户添加到audio组
sudo usermod -a -G audio $USER

# 重新登录或重启
# 或者使用 pulseaudio
pulseaudio --start
```

## 音频系统配置

### ALSA 配置

```bash
# 测试ALSA
speaker-test -c 2 -t wav

# 查看音频设备
aplay -l
cat /proc/asound/cards
```

### PulseAudio 配置

```bash
# 启动PulseAudio服务
pulseaudio --start

# 测试音频
paplay /usr/share/sounds/alsa/Front_Left.wav
```

## 性能优化

为了获得更好的性能，可以：

```bash
# 设置实时调度优先级
sudo bash -c 'echo @audio - rtprio 99 >> /etc/security/limits.conf'
sudo bash -c 'echo @audio - memlock unlimited >> /etc/security/limits.conf'

# 或使用systemd用户服务
systemctl --user enable pulseaudio
systemctl --user start pulseaudio
```

## 下一步

安装完所有依赖后，请参考 [BUILD.md](BUILD.md) 了解如何构建项目。