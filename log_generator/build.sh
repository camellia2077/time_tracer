#!/bin/bash

# Script to automate the build process for the log_generator project in an MSYS2/MinGW environment.

# --- Exit immediately if a command fails ---
set -e

# --- 1. Navigate to the script's directory ---
# This ensures that the script runs correctly regardless of where it's called from.
echo "--> Navigating to script directory..."
cd "$(dirname "$0")"
echo "--> Current directory: $(pwd)"
echo ""

# --- 2. Clean up the previous build directory ---
# If a 'build' directory exists, it is removed to ensure a clean build.
if [ -d "build" ]; then
    echo "--> Found an existing 'build' directory. Deleting it..."
    rm -rf build
    echo "--> 'build' directory deleted."
    echo ""
fi

# --- 3. Create a new build directory and enter it ---
echo "--> Creating a new 'build' directory..."
mkdir build
cd build
echo ""

# --- 4. Configure the project with CMake ---
# The "MSYS Makefiles" generator is specified for the MSYS2 environment.
# CMAKE_BUILD_TYPE is set to "Release" to enable optimizations (-O3, -flto, -march=native, -s)
# defined in the CMakeLists.txt file.
echo "--> Configuring the project with CMake (Release mode)..."
cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release ..
echo ""

# --- 5. Compile the project ---
echo "--> Compiling the project..."
cmake --build .
echo ""

# --- 6. Finalization Message ---
echo "=========================================="
echo "  Build completed successfully!         "
echo "=========================================="
echo "The executable 'log_generator.exe' is located in the 'build' directory."
echo ""

