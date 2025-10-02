#!/bin/bash

echo "Running semantic analyzer tests..."
./run_semantic_test > test_output.txt 2>&1

echo "=== FINAL TEST RESULTS ==="
tail -10 test_output.txt

echo ""
echo "=== TEST SUMMARY ==="
grep -A 5 "SEMANTIC ANALYZER TEST RESULTS" test_output.txt