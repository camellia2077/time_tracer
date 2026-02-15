set(TIME_TRACKER_ANDROID_BRIDGE_SOURCES
    "native_bridge.cpp"
)

add_library(time_tracker_android_bridge SHARED
    ${TIME_TRACKER_ANDROID_BRIDGE_SOURCES}
)
setup_app_target(time_tracker_android_bridge)
target_link_libraries(time_tracker_android_bridge PRIVATE
    time_tracker_infrastructure
    time_tracer_core
)

# Android app side loads this library by name.
set_target_properties(time_tracker_android_bridge PROPERTIES
    OUTPUT_NAME "time_tracker_android_bridge"
)
