#!/bin/bash

# Qt5安装脚本
# 设置vcpkg路径
VCPKG_PATH="/home/wwbing/DownLoad/vcpkg/vcpkg"

echo "开始安装Qt5组件..."
echo "=================================="

# 检查vcpkg是否存在
if [ ! -f "$VCPKG_PATH" ]; then
    echo "错误: vcpkg未找到，请检查路径: $VCPKG_PATH"
    exit 1
fi

# 更新vcpkg
echo "1. 更新vcpkg..."
$VCPKG_PATH update

# 清理失败的构建缓存（保留下载的源码）
echo "2. 清理失败的构建缓存..."
$VCPKG_PATH remove qt5-base --recurse

# 安装Qt5基础包（会使用已下载的源码）
echo "3. 安装Qt5基础包 (qt5-base)..."
$VCPKG_PATH install qt5-base 

# 检查安装是否成功
if [ $? -eq 0 ]; then
    echo "✓ Qt5基础包安装成功"
else
    echo "✗ Qt5基础包安装失败"
    exit 1
fi

# 安装Qt5多媒体模块
echo "3. 安装Qt5多媒体模块 (qt5-multimedia)..."
$VCPKG_PATH install qt5-multimedia 

if [ $? -eq 0 ]; then
    echo "✓ Qt5多媒体模块安装成功"
else
    echo "✗ Qt5多媒体模块安装失败"
    exit 1
fi

# 安装Qt5串口模块
echo "4. 安装Qt5串口模块 (qt5-serialport)..."
$VCPKG_PATH install qt5-serialport 

if [ $? -eq 0 ]; then
    echo "✓ Qt5串口模块安装成功"
else
    echo "✗ Qt5串口模块安装失败"
    exit 1
fi

# 安装Qt5工具
echo "5. 安装Qt5工具 (qt5-tools)..."
$VCPKG_PATH install qt5-tools --recurse

if [ $? -eq 0 ]; then
    echo "✓ Qt5工具安装成功"
else
    echo "✗ Qt5工具安装失败"
    exit 1
fi

# 显示安装结果
echo "=================================="
echo "Qt5安装完成！"
echo "已安装的Qt5包："
$VCPKG_PATH list | grep qt5

echo "=================================="
echo "安装位置: /home/wwbing/DownLoad/vcpkg/installed/x64-linux"
echo "现在可以运行 ./build.sh 来构建项目" 