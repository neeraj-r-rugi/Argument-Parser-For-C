#!/bin/bash
# This script compiles and runs the test program for the argument parser library. It demonstrates various command-line argument combinations to verify that the parser correctly handles different types of arguments, including strings, integers, booleans, floats, and multiple values.
gcc -fsanitize=address,leak -g -o test test.c

if [ $# -gt 0 ]; then
    fail="$1"
else
    fail="--true"
fi

if  [ "$fail" != "--fail" ]; then
./test -H "localhost"
echo "-----------------------------"
./test -H "192.168.1.17" --verbose
echo "-----------------------------"
./test -H "localhost" -p 5000
echo "-----------------------------"
./test -H "localhost" --timeout 2.5
echo "-----------------------------"
./test -H "localhost" -t -- -tag1 -tag2 -tag3 -- --verbose
echo "-----------------------------"
./test -H "localhost" --timeout 40.0 -t -- -tag1 -tag2 -tag3 -tag4 -tag5 -tag6 
echo "-----------------------------"
./test -H "localhost" -t "tag 9" "tag 11" "tag 13" --timeout -3.5 --verbose -p 6007 --val 10 20 30
echo "-----------------------------"
./test --help
fi

if [ "$fail" == "--fail" ]; then
    echo "Testing error handling with invalid arguments:"
    echo "-----------------------------"
    ./test -H "localhost" -p not_a_number
    echo "-----------------------------"
    ./test -H "localhost" --timeout not_a_float
    echo "-----------------------------"
    ./test -H "localhost" -t
    echo "-----------------------------"
    ./test -H "localhost" -t tag1 --unknown
    echo "-----------------------------"
    ./test -H "localhost" -p 999999999999
fi
