# Shared include roots and legacy API compatibility layout.

if(NOT TARGET tc_core_shell_layout)
    add_library(tc_core_shell_layout INTERFACE)
    target_include_directories(tc_core_shell_layout INTERFACE
        "${PROJECT_SOURCE_DIR}/host"
        "${PROJECT_SOURCE_DIR}/api/c_api"
        "${PROJECT_SOURCE_DIR}/api/android_jni"
    )
    set_property(TARGET tc_core_shell_layout PROPERTY
        TT_SHELL_LEGACY_API_C_ROOT
        "${PROJECT_SOURCE_DIR}/api/c_api"
    )
    set_property(TARGET tc_core_shell_layout PROPERTY
        TT_SHELL_LEGACY_API_ANDROID_ROOT
        "${PROJECT_SOURCE_DIR}/api/android_jni"
    )
endif()

message(STATUS "tracer_core shell layout target enabled (tc_core_shell_layout).")
