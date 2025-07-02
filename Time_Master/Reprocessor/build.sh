#!/bin/bash

# --- Script Configuration ---
# Exit immediately if a command exits with a non-zero status.
set -e

# --- Main Logic ---

# Get the absolute path of the directory where the script is located.
# This ensures the script works correctly even if called from another directory.
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Change the current working directory to the script's directory.
cd "$SCRIPT_DIR"
echo "==> Switched to project directory: $(pwd)"

# Define the build directory name.
BUILD_DIR="build"

# Check if the build directory already exists.
if [ -d "$BUILD_DIR" ]; then
    echo "==> Found existing build directory. Removing it..."
    # Remove the directory and all its contents forcefully.
    rm -rf "$BUILD_DIR"
    echo "==> Old build directory removed."
fi

# Create a new, empty build directory.
echo "==> Creating new build directory..."
mkdir "$BUILD_DIR"

# Navigate into the new build directory.
cd "$BUILD_DIR"
echo "==> Entered build directory: $(pwd)"

# Run CMake to generate the build files (e.g., Makefiles).
# The ".." tells CMake to look for CMakeLists.txt in the parent directory.
echo "==> Running CMake to configure the project..."
cmake ..

# Run the build command.
# This tells CMake to invoke the underlying build system (like make or ninja) to compile the code.
echo "==> Compiling the project..."
cmake --build .

# --- Completion Message ---
echo ""
echo "================================================"
echo "Build complete!"
echo "The executable 'main' is located in the '$BUILD_DIR' directory."
echo "You can run it from the project root with: ./$BUILD_DIR/main"
echo "================================================"


