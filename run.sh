#!/bin/bash

# ==============================================================================
# Student Simulation Runner
# ==============================================================================
# Usage:
#   ./run.sh                  -> Runs with default settings (Seed: 42, Duration: 60s)
#   ./run.sh --verbose        -> Runs in verbose mode
#   ./run.sh --seed 123       -> Runs with specific seed
#   ./run.sh --help           -> Shows help menu
# ==============================================================================

EXECUTABLE="./simulation"
MAKEFILE="Makefile"

# 1. Compilation Step (The Gatekeeper)
echo "----------------------------------------------------------------"
echo "🛠️  Compiling Simulation..."
echo "----------------------------------------------------------------"

if [ ! -f "$MAKEFILE" ]; then
    echo "❌ Error: Makefile not found! Are you in the right directory?"
    exit 1
fi

make clean > /dev/null 2>&1
make

if [ $? -ne 0 ]; then
    echo "❌ Compilation FAILED. Please fix the errors above."
    exit 1
fi

echo "✅ Compilation Successful."
echo ""

# Detect architecture and disable ASLR to fix TSAN memory mapping error
RUN_CMD="setarch $(uname -m) -R $EXECUTABLE"

# 3. Argument Parsing & Execution
echo "----------------------------------------------------------------"
echo "🚀 Starting Simulation..."
echo "----------------------------------------------------------------"

# Pass all command line arguments ($@) directly to the C++ program
$RUN_CMD "$@"

EXIT_CODE=$?

echo ""
echo "----------------------------------------------------------------"
if [ $EXIT_CODE -eq 0 ]; then
    echo "🏁 Simulation Finished (Exit Code: 0)"
    echo "   If you see 'Integrity: PASS', good job!"
elif [ $EXIT_CODE -eq 124 ]; then
    echo "💀 TIMEOUT/DEADLOCK DETECTED!"
    echo "   The simulation hung and was killed."
else
    echo "⚠️  Simulation exited with code $EXIT_CODE."
    echo "   (This might be a sanitizer error or a crash)."
fi
echo "----------------------------------------------------------------"
