cmake_minimum_required(VERSION 3.10)

# --- Project Definition ---
project(BillReprocessor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- MODIFIED: Find the system-installed nlohmann_json library ---
# This command searches for the library in your UCRT64 environment.
# It requires that you have installed it via pacman, for example:
# pacman -S mingw-w64-ucrt-x86_64-nlohmann-json
find_package(nlohmann_json 3.11.0 REQUIRED)

# --- Define a library for our modules ---
add_library(bill_lib
    BillValidator.cpp
    BillModifier.cpp
    Reprocessor.cpp
)

# --- Link the found nlohmann_json library to our library ---
# find_package provides the 'nlohmann_json::nlohmann_json' target which includes the necessary headers.
target_link_libraries(bill_lib PUBLIC nlohmann_json::nlohmann_json)


# --- Define the main executable ---
add_executable(main main.cpp)

# --- Link our library to the main executable ---
target_link_libraries(main PRIVATE bill_lib)

# --- Ensure UTF-8 source file interpretation on Windows (MSVC / MinGW) ---
if(MSVC)
    target_compile_options(main PRIVATE /utf-8)
    target_compile_options(bill_lib PRIVATE /utf-8)
else()
    # For GCC/Clang on Windows (like in your UCRT environment)
    target_compile_options(main PRIVATE -finput-charset=UTF-8)
    target_compile_options(bill_lib PRIVATE -finput-charset=UTF-8)
endif()

# --- Print instructions for the user ---
message(STATUS "Build system configured to use the system-installed nlohmann_json library.")
message(STATUS "To build, run the './build.sh' script.")

