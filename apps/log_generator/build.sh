#!/bin/bash

# Script to automate the build process for the log_generator project in an MSYS2/MinGW environment.
# This version avoids deleting the 'build' directory to allow for faster incremental builds.

# --- Exit immediately if a command fails ---
set -e

# --- 1. Navigate to the script's directory ---
# This ensures that the script runs correctly regardless of where it's called from.
echo "--> Navigating to script directory..."
cd "$(dirname "$0")"
echo "--> Current directory: $(pwd)"
echo ""

# --- 2. Ensure the build directory exists ---
# If a 'build' directory does not exist, it is created.
# An existing build directory is kept for faster incremental builds.
echo "--> Ensuring 'build' directory exists..."
mkdir -p build
echo "--> 'build' directory is ready."
echo ""

# --- 3. Enter the build directory ---
cd build

# --- 4. Configure the project with CMake ---
# The "MSYS Makefiles" generator is specified for the MSYS2 environment.
# CMAKE_BUILD_TYPE is set to "Release" to enable optimizations (-O3, -flto, -march=native, -s)
# defined in the CMakeLists.txt file.
# This step re-generates makefiles only if CMakeLists.txt has changed.
echo "--> Configuring the project with CMake (Release mode)..."
cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release ..
echo ""

# --- 5. Compile the project ---
# CMake will automatically determine what needs to be recompiled.
echo "--> Compiling the project (incremental build)..."
cmake --build .
echo ""

# --- 6. Finalization Message ---
echo "=========================================="
echo "  Build completed successfully!         "
echo "=========================================="
echo "The executable 'log_generator.exe' is located in the 'build' directory."
echo ""