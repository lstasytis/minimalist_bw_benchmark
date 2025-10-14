#sudo sysctl -w kernel.perf_event_paranoid=0
mode=$1

# basic O0 run without anything to explore ASM / debugging
if [ "$mode" == "1" ]; then
    echo "[-O0 basic]"
    export OMP_NUM_THREADS=1
    g++ -O0 -fopenmp -fverbose-asm -save-temps -o bw_test bw_test.cpp
fi

# O3+SIMD
if [ "$mode" == "2" ]; then
    echo "-O3 vectorized]"
    export OMP_NUM_THREADS=1
    g++ -std=c++17 -O3 -fopenmp -march=native -ftree-vectorize -funroll-loops \
        -fverbose-asm -save-temps bw_test.cpp -o bw_test
fi

# O3+SIMD+OMP (show with and without the pragma)
if [ "$mode" == "3" ]; then
    echo "-O3 vectorized + OMP]"
    export OMP_NUM_THREADS=$(nproc)
    g++ -std=c++17 -O3 -fopenmp -march=native -ftree-vectorize -funroll-loops \
        -fverbose-asm -save-temps bw_test.cpp -o bw_test
fi

perf stat ./bw_test

# useful commands to look at
# lscpu
# sudo lshw -class memory
