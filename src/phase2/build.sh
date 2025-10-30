#!/bin/bash
# Build script for phase2 - YM2151 tone generator

set -e

echo "Building Nuked-OPM tone generator..."

# Get the directory of this script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Use zig cc if available, otherwise fall back to gcc
if command -v zig &> /dev/null; then
    echo "Using zig cc as compiler..."
    CC="zig cc"
else
    echo "zig not found, using gcc as fallback..."
    CC="gcc"
fi

# Compile the C program
cd "$SCRIPT_DIR"
$CC -o opm_generator main.c opm.c -lm

echo "Build complete! Executable: $SCRIPT_DIR/opm_generator"
