cmake_minimum_required(VERSION 3.22.1)
project(tracer_android_native LANGUAGES C CXX)

get_filename_component(APPS_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../.." ABSOLUTE)
set(TRACER_CORE_DIR "${APPS_ROOT}/tracer_core")

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_INSTALLER OFF CACHE BOOL "" FORCE)
set(ENABLE_CLANG_TIDY OFF CACHE BOOL "" FORCE)
set(ENABLE_PCH OFF CACHE BOOL "" FORCE)
set(WARNING_LEVEL 0 CACHE STRING "" FORCE)
set(DISABLE_OPTIMIZATION OFF CACHE BOOL "")
set(ENABLE_LTO OFF CACHE BOOL "")

# ttc = tracer_core
set(TRACER_CORE_BINARY_DIR "${CMAKE_BINARY_DIR}/ttc")
add_subdirectory("${TRACER_CORE_DIR}" "${TRACER_CORE_BINARY_DIR}")

set(TRACER_ANDROID_API_DIR
    "${TRACER_CORE_DIR}/src/api/android"
)
set(TRACER_CORE_C_API_DIR
    "${TRACER_CORE_DIR}/src/api/core_c"
)
set(TIME_TRACKER_ANDROID_BRIDGE_SOURCES
    "${TRACER_ANDROID_API_DIR}/native_bridge.cpp"
    "${TRACER_ANDROID_API_DIR}/native_bridge_calls.cpp"
    "${TRACER_ANDROID_API_DIR}/native_bridge_crypto_calls.cpp"
    "${TRACER_ANDROID_API_DIR}/native_bridge_query_calls.cpp"
    "${TRACER_ANDROID_API_DIR}/native_bridge_progress.cpp"
    "${TRACER_ANDROID_API_DIR}/native_bridge_registration.cpp"
    "${TRACER_ANDROID_API_DIR}/android_runtime_factory.cpp"
    "${TRACER_ANDROID_API_DIR}/android_runtime_factory_resolver.cpp"
    "${TRACER_ANDROID_API_DIR}/android_runtime_factory_catalog.cpp"
    "${TRACER_CORE_C_API_DIR}/tracer_core_c_api.cpp"
    "${TRACER_CORE_C_API_DIR}/tracer_core_c_api_internal.cpp"
    "${TRACER_CORE_C_API_DIR}/tracer_core_c_api_workflow.cpp"
    "${TRACER_CORE_C_API_DIR}/tracer_core_c_api_reporting.cpp"
)

add_library(time_tracker_android_bridge SHARED
    ${TIME_TRACKER_ANDROID_BRIDGE_SOURCES}
)
target_include_directories(time_tracker_android_bridge PRIVATE
    "${TRACER_CORE_DIR}/src"
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
