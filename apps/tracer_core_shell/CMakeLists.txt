# --- START OF FILE CMakeLists.txt ---
cmake_minimum_required(VERSION 3.28)

project(TimeTrackerApp VERSION "0.7.2" LANGUAGES C CXX)

set(CMAKE_CLI_FORCE_UTF8 ON)
cmake_policy(SET CMP0115 NEW)

include(CTest)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/ProjectOptions.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/ProjectDependencies.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/ProjectFeatures.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/ProjectLayout.cmake")

include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/BuildMetadata.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/TargetSetup.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuildTypeFlags.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/RuntimeManifest.cmake")
tt_generate_runtime_manifest()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CoreTargets.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/AddFormatTarget.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/AddTidyFixTarget.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Packaging.cmake")

message(STATUS "CMake configuration finished. Targets are ready to be built.")
