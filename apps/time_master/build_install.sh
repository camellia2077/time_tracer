#!/bin/bash

# If any command fails, exit the script immediately.
set -e

# Switch to the script's directory so it can be run from anywhere.
cd "$(dirname "$0")"
echo "--- Switched to script directory: $(pwd)"

echo "--- Executing Python script to handle the full installation process..."

# Execute the Python script, passing the 'install' command and any other arguments.
python build.py install "$@"

echo "--- Python script finished."