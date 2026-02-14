# src/CMakeLists.txt (Coordinator)

# 1. Domain (Pure Logic)
add_subdirectory(domain)

# 2. Application (Business Logic, Pipeline)
add_subdirectory(application)

# Core boundary for cross-platform reuse:
# time_tracer_core := domain + application
add_library(time_tracer_core INTERFACE)
target_link_libraries(time_tracer_core INTERFACE
    time_tracker_domain
    time_tracker_application
)

include(CoreBoundaryRules)
enforce_core_include_boundary(
    ROOT "${PROJECT_SOURCE_DIR}/src"
    CORE_DIRS
        "domain"
        "application"
    FORBIDDEN_PREFIXES
        "api/"
        "infrastructure/"
)

# 3. Infrastructure (IO, Persistence, Config)
add_subdirectory(infrastructure)

# 4. API Layer (Interface)
add_subdirectory(api/cli)

# 5. Main Executable
add_executable(time_tracker_cli api/cli/main.cpp)
setup_app_target(time_tracker_cli)

if(CMAKE_EXE_LINKER_FLAGS_RELEASE MATCHES "fuse-ld=lld")
    target_link_libraries(time_tracker_cli PRIVATE
        -Wl,--whole-archive
        time_tracker_cli_commands
        -Wl,--no-whole-archive
        time_tracker_cli_lib
    )
else()
    target_link_libraries(time_tracker_cli PRIVATE
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,time_tracker_cli_commands>"
        time_tracker_cli_lib
    )
endif()

if(BUILD_TESTING)
    set(TIME_TRACKER_CORE_API_TEST_SOURCES
        application/tests/support/fakes.cpp
        application/tests/support/test_support.cpp
        application/tests/modules/convert_ingest_validate_tests.cpp
        application/tests/modules/report_tests.cpp
        application/tests/modules/data_query_tests.cpp
        application/tests/test_main.cpp
    )
    add_executable(time_tracker_core_api_tests
        ${TIME_TRACKER_CORE_API_TEST_SOURCES}
    )
    setup_app_target(time_tracker_core_api_tests)
    target_link_libraries(time_tracker_core_api_tests PRIVATE
        time_tracker_application
    )
    add_test(
        NAME time_tracker_core_api_tests
        COMMAND time_tracker_core_api_tests
    )
endif()

# --- Post Build Actions (Moved from root) ---
# 1. Config Copy
add_custom_command(
    TARGET time_tracker_cli POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/config"
    "$<TARGET_FILE_DIR:time_tracker_cli>/config"
    COMMENT "Copying config folder for time_tracker_cli"
)

# 2. Win32 DLL Copy
# Need to include the module from root cmake folder
set(ALL_TARGETS time_tracker_cli)
include("${CMAKE_SOURCE_DIR}/cmake/Win32PostBuildCopy.cmake")
