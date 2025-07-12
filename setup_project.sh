#!/bin/bash

# 此脚本可自动完成VehicleTerminal项目的设置和构建过程。
# 它执行三个主要任务:
# 1. 安装构建Qt和项目所需的系统依赖项。
# 2. 使用vcpkg安装Qt5库。
# 3. 使用CMake构建VehicleTerminal项目。
#
# 用法:
#   ./setup_project.sh [all|deps|qt|build]
#
# 参数:
#   all     (默认) 运行所有步骤: deps -> qt -> build.
#   deps    只安装系统依赖项。
#   qt      只通过vcpkg安装Qt库。
#   build   只构建项目。

set -e # 如果一个命令以非零状态退出，则立即退出。

# --- 配置 ---
VCPKG_ROOT="/home/wwbing/DownLoad/vcpkg"

# --- 函数 ---

install_system_deps() {
    echo "=================================="
    echo "步骤 1: 安装系统依赖"
    echo "=================================="
    sudo apt update
    sudo apt install -y build-essential cmake pkg-config git wget unzip
    sudo apt install -y bison flex
    
    echo "---> 为Qt安装X11和OpenGL依赖..."
    sudo apt install -y libx11-dev libxext-dev libxrender-dev libxrandr-dev libxfixes-dev \
                       libxcb1-dev libxcb-xfixes0-dev libxcb-shape0-dev libxcb-render0-dev \
                       libxcb-render-util0-dev libxcb-keysyms1-dev libxcb-icccm4-dev \
                       libxcb-image0-dev libxcb-shm0-dev libxcb-util1-dev \
                       libgl1-mesa-dev libglu1-mesa-dev libopengl0-dev

    echo "---> 安装多媒体和其他库..."
    sudo apt install -y libasound2-dev libpulse-dev libavcodec-dev libavformat-dev libswscale-dev \
                       libssl-dev libcurl4-openssl-dev \
                       libjpeg-dev libpng-dev libtiff-dev \
                       libfontconfig1-dev libfreetype6-dev libharfbuzz-dev
                       
    # 注意: 以下来自apt的Qt包是作为备用安装的，
    # 用于为vcpkg的Qt构建过程提供任何缺失的头文件或依赖。
    # 项目本身将链接到vcpkg构建的Qt库。
    echo "---> 安装系统Qt5开发包作为vcpkg的构建依赖..."
    sudo apt install -y qtbase5-dev qtmultimedia5-dev libqt5serialport5-dev
    
    echo "✓ 系统依赖安装完成。"
}

install_qt_libs() {
    echo "=================================="
    echo "步骤 2: 通过vcpkg安装Qt5"
    echo "=================================="
    
    local VCPKG_EXECUTABLE="$VCPKG_ROOT/vcpkg"

    if [ ! -f "$VCPKG_EXECUTABLE" ]; then
        echo "✗ 错误: 在 $VCPKG_EXECUTABLE 未找到vcpkg。"
        echo "  请先将vcpkg克隆到 $VCPKG_ROOT。"
        echo "  git clone https://github.com/microsoft/vcpkg.git $VCPKG_ROOT"
        echo "  $VCPKG_ROOT/bootstrap-vcpkg.sh"
        exit 1
    fi
    
    echo "---> 更新vcpkg..."
    "$VCPKG_EXECUTABLE" update

    # 定义要安装的Qt包
    local QT_PACKAGES=("qt5-base" "qt5-multimedia" "qt5-serialport" "qt5-tools")

    for pkg in "${QT_PACKAGES[@]}"; do
        echo "---> 尝试安装 $pkg..."
        # 使用 --recurse 和 remove 来清理失败的依赖。
        # 对qt5-tools使用 --recurse 安装，以确保在需要时所有依赖都被重新构建。
        "$VCPKG_EXECUTABLE" remove "$pkg" --recurse || true
        if [ "$pkg" == "qt5-tools" ]; then
             "$VCPKG_EXECUTABLE" install "$pkg" --recurse
        else
             "$VCPKG_EXECUTABLE" install "$pkg"
        fi

        if [ $? -eq 0 ]; then
            echo "✓ $pkg 安装成功。"
        else
            echo "✗ $pkg 安装失败。"
            exit 1
        fi
    done

    echo "✓ Qt5安装完成。"
    echo "已安装的包:"
    "$VCPKG_EXECUTABLE" list | grep qt5
}

build_project() {
    echo "=================================="
    echo "步骤 3: 构建VehicleTerminal项目"
    echo "=================================="

    local TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
    
    if [ ! -f "$TOOLCHAIN_FILE" ]; then
        echo "✗ 错误: 在 $TOOLCHAIN_FILE 未找到vcpkg工具链文件。"
        exit 1
    fi
    
    echo "---> 使用CMake配置项目..."
    mkdir -p build
    cd build

    cmake .. \
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_PREFIX_PATH="$VCPKG_ROOT/installed/x64-linux" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

    echo "---> 使用make编译项目..."
    make -j$(nproc)

    cd ..
    echo "---> 为clangd创建compile_commands.json符号链接..."
    if [ -f "build/compile_commands.json" ]; then
        ln -sf build/compile_commands.json compile_commands.json
    fi
    
    echo "✓ 构建完成!"
    echo "  可执行文件位于: build/bin/VehicleTerminal"
}


# --- 主逻辑 ---
main() {
    ACTION=${1:-all} # 如果没有提供参数，默认为 'all'

    case "$ACTION" in
        all)
            install_system_deps
            install_qt_libs
            build_project
            ;;
        deps)
            install_system_deps
            ;;
        qt)
            install_qt_libs
            ;;
        build)
            build_project
            ;;
        *)
            echo "错误: 无效参数。请使用 'all', 'deps', 'qt', 或 'build'."
            exit 1
            ;;
    esac
    
    echo "=================================="
    echo "脚本成功执行完毕。"
    echo "=================================="
}

main "$@" 