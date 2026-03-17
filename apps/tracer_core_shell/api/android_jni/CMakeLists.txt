set(TIME_TRACKER_ANDROID_BRIDGE_SOURCES
    "native_bridge.cpp"
    "native_bridge_calls.cpp"
    "jni_runtime_code_bridge.cpp"
    "native_bridge_query_calls.cpp"
    "native_bridge_registration.cpp"
    "${PROJECT_SOURCE_DIR}/host/native_bridge_crypto_calls.cpp"
    "${PROJECT_SOURCE_DIR}/host/crypto_progress_bridge.cpp"
    "${PROJECT_SOURCE_DIR}/host/native_bridge_progress.cpp"
    "${PROJECT_SOURCE_DIR}/host/android_runtime_config_bridge.cpp"
    "${PROJECT_SOURCE_DIR}/host/android_runtime_factory.cpp"
    "${PROJECT_SOURCE_DIR}/host/android_runtime_factory_resolver.cpp"
    "${PROJECT_SOURCE_DIR}/host/android_runtime_factory_catalog.cpp"
)

set_source_files_properties(
    "${PROJECT_SOURCE_DIR}/host/android_runtime_config_bridge.cpp"
    PROPERTIES
        CXX_SCAN_FOR_MODULES ON
)

add_library(tt_android_bridge SHARED
    ${TIME_TRACKER_ANDROID_BRIDGE_SOURCES}
)
setup_app_target(tt_android_bridge)
target_include_directories(tt_android_bridge PRIVATE
    "${PROJECT_SOURCE_DIR}"
)
target_link_libraries(tt_android_bridge PRIVATE
    tc_adapters_iface
    tc_all_lib
    tc_core_bridge_common_c_api_lib
)

# Android app side loads this library by name.
set_target_properties(tt_android_bridge PROPERTIES
    OUTPUT_NAME "time_tracker_android_bridge"
)

# Phase 1 layering target name for Android app delivery.
add_library(tt_android_app_iface INTERFACE)
target_link_libraries(tt_android_app_iface INTERFACE
    tt_android_bridge
)
