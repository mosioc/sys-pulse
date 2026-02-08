#!/bin/bash

# SysPulse Launcher Script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXECUTABLE="$SCRIPT_DIR/build/bin/SysPulse"

# Check if executable exists
if [ ! -f "$EXECUTABLE" ]; then
    echo "Error: SysPulse executable not found at $EXECUTABLE"
    exit 1
fi

# Set library path to avoid snap conflicts
export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH

# Run the application
exec "$EXECUTABLE" "$@"
