cmake_minimum_required(VERSION 2.8)
project(hetaira)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})
set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_CXX_FLAGS "-std=c++11 -fno-rtti")
set(CMAKE_CXX_FLAGS "-std=c++11")

# set(CMAKE_EXE_LINKER_FLAGS "-Wl,-whole-archive")

# 这个set可用于部分静态部分动态链接
# set(CMAKE_EXE_LINKER_FLAGS "-Wl,-static -lcjson -Wl,-call_shared")

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(/usr/local/mysql-connector-c++/include)
link_directories(/usr/local/mysql-connector-c++/lib)
add_subdirectory(src)
