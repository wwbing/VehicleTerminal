# 智能车载终端系统

## 项目简介

智能车载终端系统是一个基于Qt框架开发的综合性车载信息娱乐系统，集成了多种实用功能，为驾驶者提供全方位的车载体验。

## 主要功能

### 🎵 音乐播放器
- 支持本地音乐播放
- 集成网易云音乐API，支持在线搜索和播放
- 支持歌单播放、歌词显示
- 美观的音乐播放界面

### 🗺️ 地图导航
- 集成百度地图API
- GPS定位功能
- 实时导航和路径规划
- 支持UART串口通信

### 🌤️ 天气信息
- 实时天气查询和显示
- 天气预报功能
- 温湿度传感器数据采集（DHT11）
- 美观的天气界面展示

### 📹 监控系统
- 摄像头实时监控
- 光线传感器（AP3216）集成
- 环境监测功能
- 支持视频录制

### 🎤 语音识别
- 语音命令控制
- 实时语音识别
- 支持多种语音指令
- 集成网络语音识别API

### ⏰ 时钟功能
- 实时时钟显示
- 日期时间同步
- 简洁的时钟界面

### ⚙️ 系统设置
- 系统参数配置
- 用户偏好设置
- 界面个性化设置

## 技术架构

### 开发环境
- **框架**: Qt 5/6
- **语言**: C++
- **编译器**: 支持C++11标准
- **平台**: Linux嵌入式系统

### 核心模块
```
VehicleTerminal/
├── main.cpp                 # 主程序入口
├── mainwindow.cpp/h/ui      # 主窗口界面
├── settingwindow.cpp/h/ui   # 设置窗口
├── clock.cpp/h/ui          # 时钟模块
├── dht11.cpp/h             # 温湿度传感器
├── speechrecognition.cpp/h  # 语音识别
├── Music/                  # 音乐播放模块
├── Map/                    # 地图导航模块
├── Weather/                # 天气信息模块
├── Monitor/                # 监控系统模块
└── img/                    # 资源文件
```

### 硬件支持
- **传感器**: DHT11温湿度传感器、AP3216光线传感器
- **通信**: UART串口、I2C总线
- **音频**: 音频录制和播放
- **摄像头**: 视频采集和监控
- **GPS**: 定位模块

## 安装和运行

### 环境要求
- Ubuntu 20.04 或更高版本
- C++11 兼容的编译器
- vcpkg 包管理器

### 快速安装（推荐）

#### 方法一：一键安装
```bash
# 给脚本添加执行权限
chmod +x setup_project.sh

# 运行完整设置脚本
./setup_project.sh
```

#### 方法二：分步安装
```bash
# 1. 安装系统依赖
chmod +x install_dependencies.sh
./install_dependencies.sh

# 2. 安装Qt5
chmod +x install_qt5.sh
./install_qt5.sh

# 3. 构建项目
chmod +x build.sh
./build.sh
```

### 手动安装步骤
1. 安装系统依赖：
   ```bash
   sudo apt update
   sudo apt install -y build-essential cmake pkg-config bison flex
   sudo apt install -y qt5-default qtbase5-dev qtmultimedia5-dev libqt5serialport5-dev
   ```

2. 使用vcpkg安装Qt5：
   ```bash
   /home/wwbing/DownLoad/vcpkg/vcpkg install qt5-base qt5-multimedia qt5-serialport qt5-tools
   ```

3. 构建项目：
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=/home/wwbing/DownLoad/vcpkg/scripts/buildsystems/vcpkg.cmake
   make -j$(nproc)
   ```

### 运行步骤
1. 编译成功后，可执行文件位于 `build/bin/VehicleTerminal`
2. 运行程序：
   ```bash
   ./build/bin/VehicleTerminal
   ```
3. 系统将启动主界面
4. 通过界面按钮访问各个功能模块

## 配置说明

### 网络配置
- 确保网络连接正常，用于天气查询和音乐下载
- 配置网易云音乐API访问权限

### 硬件配置
- 连接DHT11传感器到指定GPIO引脚
- 配置UART串口用于GPS通信
- 连接摄像头设备
- 配置音频设备

### API配置
项目使用以下外部API：
- 网易云音乐API（音乐搜索和播放）
- 百度地图API（地图和导航）
- 天气API（天气信息查询）
- 语音识别API（语音命令处理）

## 项目结构

```
VehicleTerminal/
├── 相关测试程序/           # 测试程序
├── 设备树/                # 设备树文件
├── 驱动/                  # 硬件驱动
├── VehicleTerminal/       # 主程序源码
│   ├── main.cpp
│   ├── mainwindow.*
│   ├── settingwindow.*
│   ├── clock.*
│   ├── dht11.*
│   ├── speechrecognition.*
│   ├── Music/            # 音乐模块
│   ├── Map/              # 地图模块
│   ├── Weather/          # 天气模块
│   ├── Monitor/          # 监控模块
│   └── img/              # 图片资源
├── 运行智能车载终端步骤.docx
├── 音乐下载API.txt
└── README.md
```

## 功能特色

- **模块化设计**: 各功能模块独立，便于维护和扩展
- **用户友好**: 直观的图形界面，易于操作
- **实时性**: 支持实时数据采集和显示
- **多媒体支持**: 完整的音频视频处理能力
- **网络集成**: 丰富的在线服务集成
- **硬件集成**: 完善的传感器和硬件支持

## 开发说明

### 代码规范
- 遵循Qt编程规范
- 使用C++11标准
- 模块化设计，低耦合高内聚
- 完善的错误处理机制

### 扩展开发
- 新增功能模块可参考现有模块结构
- 遵循项目的命名规范和代码风格
- 添加相应的UI文件和资源文件
- 更新项目配置文件

## 许可证

本项目仅供学习和研究使用。

## 联系方式

如有问题或建议，请通过以下方式联系：
- 提交Issue到项目仓库
- 发送邮件至项目维护者

---

**注意**: 使用前请确保硬件设备正确连接，网络环境正常，并配置相应的API密钥。 