# --- 1. Project Configuration ---
# ]Sets the minimum required CMake version and defines the project name and language. 
cmake_minimum_required(VERSION 3.16) 
project(log_generator_project CXX)

# --- 2. C++ Standard ---
# ]Sets the C++ standard to C++20, which is required by the source code.
set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- 3. Dependency Handling (nlohmann/json) ---
# We use find_package to locate the nlohmann/json library installed in the MSYS2 environment.
find_package(nlohmann_json 3.11.3 REQUIRED) 

# --- 4. Executable Definition (MODIFIED) ---
# ]Creates the executable target from ALL a project's source files.
# ]We list all .cpp files that need to be compiled.
set(SOURCES
    main.cpp
    Config.cpp
    LogGenerator.cpp
    Utils.cpp             # <-- 新增
    FileManager.cpp       # <-- 新增
    PerformanceReporter.cpp # <-- 新增
)
add_executable(log_generator ${SOURCES})

# --- 5. Linking Dependencies ---
# Links the executable with the nlohmann/json library. ]This does not need to be changed.
target_link_libraries(log_generator PRIVATE nlohmann_json::nlohmann_json)

# --- 6. Optimization and Version-Specific Compilation Options ---
#  ]Adds high-performance optimizations for "Release" builds. 
#    ]-O3: Aggressive optimization level. 
#    ]-march=native: Optimizes the code for the build machine's CPU architecture. 
#    ]-s: Strips symbols from the executable to reduce its size (linker option). 
#    ]-flto: Enables Link-Time Optimization (LTO) for better global optimization. 
#  ]Compilation options for GCC/Clang [cite: 11]
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(log_generator PRIVATE
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Release>:-march=native>
  )
endif()

# Enables Link-Time Optimization (LTO) for the Release configuration
set_property(TARGET log_generator PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

# Adds the strip flag (-s) to the linker flags for the Release configuration
set_property(TARGET log_generator PROPERTY LINK_FLAGS_RELEASE "-s")


# --- 7. Status Messages ---
# ]Prints a helpful message after CMake configuration is complete.
message(STATUS "Project configured. To compile, run:") 
message(STATUS "  cmake --build . --config Release")
message(STATUS "The executable will be found in the build directory.")