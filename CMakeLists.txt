# 设置CMake版本
cmake_minimum_required(VERSION 3.28)

# 设置工程名
project(CodeSnippet)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)

# 设置 Debug 模式下的编译选项
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")

# 设置 Release 模式下的编译选项
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# 设置生成可执行程序，并添加代码文件
add_executable(CodeSnippet main.cpp
        MString.cpp
        MString.h
        ClubMember.cpp
        ClubMember.h
        Logger.cpp
        Logger.h
        SLogger.hpp
)
