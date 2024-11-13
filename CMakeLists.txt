cmake_minimum_required(VERSION 3.15)

# 设置编译器路径
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_OBJC_COMPILER "/usr/bin/clang")
set(CMAKE_OBJCXX_COMPILER "/usr/bin/clang++")

project(minimal_text_editor VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用 Objective-C++ 支持
if(APPLE)
    enable_language(OBJCXX)
endif()

# 源文件
set(SOURCES
    src/core/rope.cpp
    src/core/buffer.cpp
    src/main.mm
)

# 添加可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE 
    src
    src/core
)

# macOS 特定设置
if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "-framework Cocoa"
    )
endif() 