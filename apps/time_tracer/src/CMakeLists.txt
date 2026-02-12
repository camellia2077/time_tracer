# src/CMakeLists.txt (Coordinator)

# 1. Domain (Pure Logic)
add_subdirectory(domain)

# 2. Infrastructure (IO, Persistence, Config)
add_subdirectory(infrastructure)

# 3. Application (Business Logic, Pipeline)
add_subdirectory(application)

# 4. API Layer (Interface)
add_subdirectory(api/cli)

# 5. Main Executable
add_executable(time_tracker_cli main_cli.cpp)
setup_app_target(time_tracker_cli)

target_link_libraries(time_tracker_cli PRIVATE
    -Wl,--whole-archive time_tracker_cli_commands -Wl,--no-whole-archive
    time_tracker_cli_lib
    time_tracker_application
)

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
