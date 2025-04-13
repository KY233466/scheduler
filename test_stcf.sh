#!/bin/bash

# Directory containing test files
test_dir="./tests"
# Directory where provided correct outputs are stored
answers_dir="./answers"
# Directory to store your actual outputs (your program's results)
my_answers_dir="./my_answers"

# Make sure both answer directories exist
mkdir -p "$answers_dir"
mkdir -p "$my_answers_dir"

echo "Running tests..."

# List of test numbers
test_numbers=(1)
# test_numbers=(1 2 3 4 5 6 7 8 9 10 11 12 13 14 15)

# Loop through tests
for i in "${test_numbers[@]}"; do
    input_file="$test_dir/test_stcf_${i}.proc"
    expected_output="$answers_dir/test_stcf_${i}.output"
    actual_output="$my_answers_dir/test_stcf_${i}.actual"

    # Run the scheduler and redirect output
    ./sched_stcf "$input_file" > "$actual_output"

    # Compare actual output with expected output
    if diff -q "$expected_output" "$actual_output" > /dev/null; then
        echo "Test $i passed."
    else
        echo "Test $i failed! Check $actual_output for your output and $expected_output for expected output. Input is $input_file"
    fi
done

echo "Testing complete."