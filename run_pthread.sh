#!/usr/bin/env bash

#sudo sysctl -w kernel.perf_event_paranoid=0
mode=$1

# basic O0 run without anything to explore ASM / debugging
if [ "$mode" == "1" ]; then
    echo "[-O0 basic]"
    export OMP_NUM_THREADS=1
    g++ -std=c++20 -O3 -fopenmp -pthread -march=native \
        -ftree-vectorize -funroll-loops bw_test_pthread.cpp -o bw_test_pthread
fi

# O3+SIMD
if [ "$mode" == "2" ]; then
    echo "-O3 vectorized]"
    export OMP_NUM_THREADS=1
    g++ -std=c++20 -O3 -fopenmp -march=native -ftree-vectorize -funroll-loops \
        -fverbose-asm -save-temps bw_test_pthread.cpp -o bw_test_pthread
fi

# O3+SIMD+OMP (show with and without the pragma)
if [ "$mode" == "3" ]; then
    echo "-O3 vectorized + OMP]"
    export OMP_NUM_THREADS=$(nproc)
    g++ -std=c++20 -O3 -fopenmp -march=native -ftree-vectorize -funroll-loops \
        -fverbose-asm -save-temps bw_test_pthread.cpp -o bw_test_pthread
fi

#./bw_test
perf stat ./bw_test_pthread

# useful commands to look at# lscpu
# sudo lshw -class memory
