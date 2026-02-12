# src/domain/CMakeLists.txt
add_library(time_tracker_domain STATIC
    "utils/time_utils.cpp"
    # Interfaces (Optional for build but good for IDE)
    "repositories/i_project_repository.hpp"
)

setup_plugin_target(time_tracker_domain NO_STDCXXEXP)
