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
# 
LLVM_IR="$1"

$LLVM_PATH/bin/opt -load-pass-plugin build/HelloWorldPlugin.so --passes hello-world-pass $LLVM_IR
# $LLVM_PATH/bin/opt -load-pass-plugin build/HelloWorldPlugin.so --passes indvars -S $LLVM_IR