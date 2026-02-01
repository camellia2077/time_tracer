# --- 1. Project Configuration ---
cmake_minimum_required(VERSION 3.16)
project(log_generator_project CXX)

find_program(CCACHE_EXECUTABLE ccache)
if(CCACHE_EXECUTABLE)
    message(STATUS "ccache found: ${CCACHE_EXECUTABLE}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
else()
    message(STATUS "ccache not found. Proceeding without compiler cache.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# --- 2. Load Modules ---
include(StandardSettings)
include(Dependencies)
include(ClangTidy)
include(Sources)

# --- 3. Executable Definition ---
add_executable(log_generator ${SOURCES})
target_include_directories(log_generator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(log_generator PRIVATE tomlplusplus::tomlplusplus)

if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
    set_target_properties(log_generator PROPERTIES
        CXX_CLANG_TIDY "${CLANG_TIDY_EXE}"
    )
endif()

# --- 4. Apply Remaining Modules ---
include(CompilerOptions)
include(FileOperations)
include(AddTidyFixTarget)
include(AddFormatTarget)

# --- 5. Status Messages ---
message(STATUS "Project configured.")
message(STATUS "To compile, run: cmake --build . --config Release")
