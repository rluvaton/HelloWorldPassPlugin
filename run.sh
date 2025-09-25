#!/bin/bash

# Exit on failure
set -e

cmake --build build 

LLVM_PATH="/opt/homebrew/opt/llvm"

# $LLVM_PATH/bin/opt --print-passes

# $LLVM_PATH/bin/opt \
#   -load-pass-plugin build/HelloWorldPlugin.so \
#   --passes hello-world-pass \
#   not-optimized.ll

actual_output=$($LLVM_PATH/bin/opt -load-pass-plugin build/HelloWorldPlugin.so --passes hello-world-pass -S ./llvm_irs/not-optimized.ll 2>&1 1>/dev/null)


# if [ "$actual_output" = "$expected_output" ]; then
#     echo "Test passed."
#     exit 0
# else
    echo "Test failed. Output was:"
    echo "$actual_output"
    exit 1
# fi
