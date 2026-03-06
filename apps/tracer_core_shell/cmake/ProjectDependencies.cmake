# External dependency orchestration for tracer_core_shell.

if(ANDROID)
    include("${CMAKE_CURRENT_LIST_DIR}/AndroidDependencies.cmake")
    configure_time_tracer_android_dependencies()
else()
    include("${CMAKE_CURRENT_LIST_DIR}/HostDependencies.cmake")
    configure_time_tracer_host_dependencies()
endif()
