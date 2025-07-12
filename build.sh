#!/bin/bash

# 设置vcpkg工具链文件路径
VCPKG_ROOT="/home/wwbing/DownLoad/vcpkg"
TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

# 创建构建目录
mkdir -p build
cd build

# 配置项目
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$VCPKG_ROOT/installed/x64-linux" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# 构建项目
make -j$(nproc)

# 创建编译数据库符号链接，方便clangd使用
cd ..
if [ ! -L compile_commands.json ]; then
    ln -sf build/compile_commands.json compile_commands.json
fi

echo "构建完成！可执行文件位于 build/bin/VehicleTerminal"
