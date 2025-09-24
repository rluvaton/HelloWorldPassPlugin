#!/bin/bash

LLVM_PATH="/opt/homebrew/opt/llvm"

expected_output="foo
bar
baz
bez"

actual_output=$($LLVM_PATH/bin/opt -load-pass-plugin build/HelloWorldPlugin.so --passes hello-world-pass test.ll 2>&1 1>/dev/null)

if [ "$actual_output" = "$expected_output" ]; then
    echo "Test passed."
    exit 0
else
    echo "Test failed. Output was:"
    echo "$actual_output"
    exit 1
fi
