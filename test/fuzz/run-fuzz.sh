#!/bin/sh
AFL_USE_ASAN=1 afl-clang -Wall -I../include test.c -m32 -g -O3 -DFUZZ
AFL_SKIP_CPUFREQ=1 afl-fuzz -i samples -o out -m 800 ./a.out