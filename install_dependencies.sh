#!/bin/bash

# 系统依赖安装脚本
echo "开始安装系统依赖..."
echo "=================================="

# 更新包列表
echo "1. 更新包列表..."
sudo apt update

# 安装编译工具
echo "2. 安装编译工具..."
sudo apt install -y build-essential cmake pkg-config

# 安装bison和flex
echo "3. 安装bison和flex..."
sudo apt install -y bison flex

# 安装X11和OpenGL依赖
echo "4. 安装X11和OpenGL依赖..."
sudo apt install -y libx11-dev libxext-dev libxrender-dev libxrandr-dev libxfixes-dev
sudo apt install -y libxcb1-dev libxcb-xfixes0-dev libxcb-shape0-dev libxcb-render0-dev
sudo apt install -y libxcb-render-util0-dev libxcb-keysyms1-dev libxcb-icccm4-dev
sudo apt install -y libxcb-image0-dev libxcb-shm0-dev libxcb-util1-dev
sudo apt install -y libgl1-mesa-dev libglu1-mesa-dev libopengl0-dev

# 安装Qt5开发依赖
echo "5. 安装Qt5开发依赖..."
sudo apt install -y qt5-default qtbase5-dev qtmultimedia5-dev libqt5serialport5-dev

# 安装音频和视频依赖
echo "6. 安装音频和视频依赖..."
sudo apt install -y libasound2-dev libpulse-dev libavcodec-dev libavformat-dev libswscale-dev

# 安装网络依赖
echo "7. 安装网络依赖..."
sudo apt install -y libssl-dev libcurl4-openssl-dev

# 安装图像处理依赖
echo "8. 安装图像处理依赖..."
sudo apt install -y libjpeg-dev libpng-dev libtiff-dev

# 安装字体和文本渲染依赖
echo "9. 安装字体和文本渲染依赖..."
sudo apt install -y libfontconfig1-dev libfreetype6-dev libharfbuzz-dev

# 安装其他必要依赖
echo "10. 安装其他必要依赖..."
sudo apt install -y git wget unzip pkg-config

echo "=================================="
echo "系统依赖安装完成！"
echo "现在可以运行 ./install_qt5.sh 来安装Qt5" 