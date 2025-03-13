# CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(BattlefieldProject)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 包含的头文件目录
include_directories(${CMAKE_SOURCE_DIR})

# 查找所有源文件
file(GLOB SOURCES
    "engine.cpp"
    "field.cpp"
    "main.cpp"
    "olcPixelGameEngine.cpp"
    "units.cpp"
)

# 可执行文件目标
add_executable(Battlefield ${SOURCES})

# 设置链接库（如果有的话）
# target_link_libraries(Battlefield <libraries>)

# 安装目标（可选）
# install(TARGETS Battlefield DESTINATION bin)
