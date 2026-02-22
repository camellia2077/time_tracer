set(TIME_TRACKER_ANDROID_BRIDGE_SOURCES
    "native_bridge.cpp"
    "native_bridge_calls.cpp"
    "native_bridge_registration.cpp"
    "android_runtime_factory.cpp"
)

add_library(time_tracker_android_bridge SHARED
    ${TIME_TRACKER_ANDROID_BRIDGE_SOURCES}
)
setup_app_target(time_tracker_android_bridge)
target_link_libraries(time_tracker_android_bridge PRIVATE
    tracer_adapters
    tracer_core
)

# Android app side loads this library by name.
set_target_properties(time_tracker_android_bridge PROPERTIES
    OUTPUT_NAME "time_tracker_android_bridge"
)

# Phase 1 layering target name for Android app delivery.
add_library(tracer_android_app INTERFACE)
target_link_libraries(tracer_android_app INTERFACE
    time_tracker_android_bridge
)
