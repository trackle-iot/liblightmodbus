#!/bin/sh
cd "$(dirname "$0")"
tar xvf samples.tar
AFL_USE_ASAN=1 afl-clang++ -Wall -I../../include slavefuzz.cpp -m32 -g -O3 -DFUZZ -o slavefuzz
AFL_SKIP_CPUFREQ=1 afl-fuzz -i samples -o out -m 800 ./slavefuzz