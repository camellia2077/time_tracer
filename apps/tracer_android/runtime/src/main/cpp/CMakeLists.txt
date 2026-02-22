cmake_minimum_required(VERSION 3.22.1)
project(tracer_android_native LANGUAGES C CXX)

get_filename_component(APPS_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../.." ABSOLUTE)
set(TIME_TRACER_DIR "${APPS_ROOT}/time_tracer")

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_INSTALLER OFF CACHE BOOL "" FORCE)
set(ENABLE_CLANG_TIDY OFF CACHE BOOL "" FORCE)
set(ENABLE_PCH OFF CACHE BOOL "" FORCE)
set(WARNING_LEVEL 0 CACHE STRING "" FORCE)
set(DISABLE_OPTIMIZATION OFF CACHE BOOL "")
set(ENABLE_LTO OFF CACHE BOOL "")

# ttc = time_tracer_core
set(TIME_TRACER_CORE_BINARY_DIR "${CMAKE_BINARY_DIR}/ttc")
add_subdirectory("${TIME_TRACER_DIR}" "${TIME_TRACER_CORE_BINARY_DIR}")

set(TIME_TRACER_ANDROID_API_DIR
    "${TIME_TRACER_DIR}/src/api/android"
)
set(TIME_TRACER_CORE_C_API_DIR
    "${TIME_TRACER_DIR}/src/api/core_c"
)
set(TIME_TRACKER_ANDROID_BRIDGE_SOURCES
    "${TIME_TRACER_ANDROID_API_DIR}/native_bridge.cpp"
    "${TIME_TRACER_ANDROID_API_DIR}/native_bridge_calls.cpp"
    "${TIME_TRACER_ANDROID_API_DIR}/native_bridge_registration.cpp"
    "${TIME_TRACER_ANDROID_API_DIR}/android_runtime_factory.cpp"
    "${TIME_TRACER_ANDROID_API_DIR}/android_runtime_factory_resolver.cpp"
    "${TIME_TRACER_ANDROID_API_DIR}/android_runtime_factory_catalog.cpp"
    "${TIME_TRACER_CORE_C_API_DIR}/time_tracer_core_c_api.cpp"
    "${TIME_TRACER_CORE_C_API_DIR}/time_tracer_core_c_api_internal.cpp"
    "${TIME_TRACER_CORE_C_API_DIR}/time_tracer_core_c_api_workflow.cpp"
    "${TIME_TRACER_CORE_C_API_DIR}/time_tracer_core_c_api_reporting.cpp"
)

add_library(time_tracker_android_bridge SHARED
    ${TIME_TRACKER_ANDROID_BRIDGE_SOURCES}
)
target_include_directories(time_tracker_android_bridge PRIVATE
    "${TIME_TRACER_DIR}/src"
)
target_link_libraries(time_tracker_android_bridge PRIVATE
    tracer_adapters
    tracer_core
)

# Android app side loads this library by name.
set_target_properties(time_tracker_android_bridge PROPERTIES
    OUTPUT_NAME "time_tracker_android_bridge"
)

# Keep layering alias for Android app delivery.
add_library(tracer_android_app INTERFACE)
target_link_libraries(tracer_android_app INTERFACE
    time_tracker_android_bridge
)
