# the minimum version of CMake.
cmake_minimum_required(VERSION 3.5.0)
project(tracer_harmony)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(DEFINED PACKAGE_FIND_FILE)
    include(${PACKAGE_FIND_FILE})
endif()

# 包含 NAPI 层自己以及 C++ Core 的头文件
include_directories(${NATIVERENDER_ROOT_PATH}
                    ${NATIVERENDER_ROOT_PATH}/include
                    ${NATIVERENDER_ROOT_PATH}/../../../../../../tracer_core/src)

add_library(entry SHARED napi_init.cpp)

# 动态链接 NDK 的 NAPI, hilog 以及外部提供的 time_tracer_core 动态库 (目前由预编译或 CMake 提供)
target_link_libraries(entry PUBLIC libace_napi.z.so libhilog_ndk.z.so)