cmake_minimum_required(VERSION 3.16)
project(VehicleTerminal VERSION 1.0.0 LANGUAGES C CXX)

# 设置C和C++标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# 查找Qt5组件
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Multimedia Network SerialPort)

# 设置Qt5的MOC、UIC、RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 包含目录 - 为clangd提供完整的头文件路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/VehicleTerminal)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/VehicleTerminal/Map)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/VehicleTerminal/Music)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/VehicleTerminal/Weather)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/VehicleTerminal/Monitor)

# 添加Qt5的包含目录
if(Qt5_FOUND)
    include_directories(${Qt5Core_INCLUDE_DIRS})
    include_directories(${Qt5Widgets_INCLUDE_DIRS})
    include_directories(${Qt5Multimedia_INCLUDE_DIRS})
    include_directories(${Qt5Network_INCLUDE_DIRS})
    include_directories(${Qt5SerialPort_INCLUDE_DIRS})
endif()

# 源文件
set(SOURCES
    VehicleTerminal/main.cpp
    VehicleTerminal/mainwindow.cpp
    VehicleTerminal/settingwindow.cpp
    VehicleTerminal/clock.cpp
    VehicleTerminal/dht11.cpp
    VehicleTerminal/speechrecognition.cpp
    VehicleTerminal/Map/baidumap.cpp
    VehicleTerminal/Map/gps.c
    VehicleTerminal/Map/uart.c
    VehicleTerminal/Monitor/ap3216.cpp
    VehicleTerminal/Monitor/capture_thread.cpp
    VehicleTerminal/Monitor/monitor.cpp
    VehicleTerminal/Music/musicplayer.cpp
    VehicleTerminal/Music/searchmusic.cpp
    VehicleTerminal/Weather/weather.cpp
)

# 头文件
set(HEADERS
    VehicleTerminal/mainwindow.h
    VehicleTerminal/settingwindow.h
    VehicleTerminal/clock.h
    VehicleTerminal/dht11.h
    VehicleTerminal/speechrecognition.h
    VehicleTerminal/Map/baidumap.h
    VehicleTerminal/Map/gps.h
    VehicleTerminal/Map/uart.h
    VehicleTerminal/Monitor/ap3216.h
    VehicleTerminal/Monitor/capture_thread.h
    VehicleTerminal/Monitor/monitor.h
    VehicleTerminal/Music/musicplayer.h
    VehicleTerminal/Music/searchmusic.h
    VehicleTerminal/Weather/weather.h
)

# UI文件
set(UI_FILES
    VehicleTerminal/mainwindow.ui
    VehicleTerminal/settingwindow.ui
    VehicleTerminal/clock.ui
    VehicleTerminal/Map/baidumap.ui
    VehicleTerminal/Monitor/monitor.ui
    VehicleTerminal/Music/musicplayer.ui
    VehicleTerminal/Music/searchmusic.ui
    VehicleTerminal/Weather/weather.ui
)

# 资源文件
set(RESOURCES
    VehicleTerminal/img.qrc
)

# 创建可执行文件
add_executable(VehicleTerminal ${SOURCES} ${HEADERS} ${UI_FILES} ${RESOURCES})

# 链接Qt5库
target_link_libraries(VehicleTerminal 
    Qt5::Core 
    Qt5::Widgets 
    Qt5::Multimedia 
    Qt5::Network 
    Qt5::SerialPort
)

# 设置输出目录
set_target_properties(VehicleTerminal PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 复制必要的文件到输出目录
file(COPY VehicleTerminal/Music/myMusic DESTINATION ${CMAKE_BINARY_DIR}/bin)
file(COPY VehicleTerminal/Music/images DESTINATION ${CMAKE_BINARY_DIR}/bin)
file(COPY VehicleTerminal/Map/img DESTINATION ${CMAKE_BINARY_DIR}/bin)
file(COPY VehicleTerminal/Weather/images DESTINATION ${CMAKE_BINARY_DIR}/bin)
file(COPY VehicleTerminal/Monitor/ap3216 DESTINATION ${CMAKE_BINARY_DIR}/bin)
file(COPY VehicleTerminal/Music/style.qss DESTINATION ${CMAKE_BINARY_DIR}/bin)

# 设置编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(VehicleTerminal PRIVATE DEBUG)
endif()

# 安装规则
install(TARGETS VehicleTerminal
    RUNTIME DESTINATION bin
)

# 安装资源文件
install(DIRECTORY VehicleTerminal/Music/myMusic DESTINATION bin)
install(DIRECTORY VehicleTerminal/Music/images DESTINATION bin)
install(DIRECTORY VehicleTerminal/Map/img DESTINATION bin)
install(DIRECTORY VehicleTerminal/Weather/images DESTINATION bin)
install(FILES VehicleTerminal/Monitor/ap3216 DESTINATION bin)
install(FILES VehicleTerminal/Music/style.qss DESTINATION bin)

# 生成编译数据库 - 对clangd很重要
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 