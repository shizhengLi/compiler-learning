#!/bin/bash

echo "Running code generator test..."
./tests/test_codegen_comprehensive > codegen_test_output.txt 2>&1 &
TEST_PID=$!

# Wait up to 5 seconds for test to complete
sleep 5

# Check if test is still running
if kill -0 $TEST_PID 2>/dev/null; then
    echo "Test still running, killing process..."
    kill $TEST_PID
    wait $TEST_PID 2>/dev/null
fi

echo "=== CODE GENERATOR TEST RESULTS ==="
cat codegen_test_output.txt

echo ""
echo "=== FINAL RESULTS ==="
tail -10 codegen_test_output.txt

# Clean up
rm -f codegen_test_output.txt
rm -f test_*.asm