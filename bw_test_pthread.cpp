// bw_test_pthread.cpp
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <chrono>
#include <thread>
#include <omp.h>
#include "PerfEvent.hpp"

#define N_L1 1024             // 8KB
#define N_L2 (1024*1024 / 4)  // 2MB
#define N_L3 (1024*1024)      // 8MB
#define N_DDR (40*1024*1024)  // 320MB

static void* aligned_alloc64(std::size_t size) {
    void* ptr = nullptr;
    if (posix_memalign(&ptr, 64, size) != 0) {
        return nullptr;
    }
    return ptr;
}

static double seconds() {
    using namespace std::chrono;
    return duration<double>(high_resolution_clock::now().time_since_epoch()).count();
}

void run_test(std::size_t N, const char *label) {
    std::size_t bytes = N * sizeof(double);

    double *a = static_cast<double*>(aligned_alloc64(bytes));
    double *b = static_cast<double*>(aligned_alloc64(bytes));
    double *c = static_cast<double*>(aligned_alloc64(bytes));

    #pragma omp parallel for
    for (std::size_t i = 0; i < N; ++i) { a[i] = 1.0; b[i] = 2.0; c[i] = 0.0; }

    double t0 = seconds();
    double t1 = t0;
    std::size_t repeats = 0;
    PerfEvent e;
    


    std::jthread worker([&](std::stop_token stoken) {
        e.startCounters();
        while (!stoken.stop_requested()) {
        // v1
            #pragma omp parallel for // <--- comment/uncomment for funny results
            for (std::size_t i = 0; i < N; ++i) {
                c[i] = a[i] + b[i];
            }
            ++repeats;
        }
        e.stopCounters();
    });

    std::this_thread::sleep_for(std::chrono::seconds(2));
    worker.request_stop(); // Request the worker to stop
    worker.join();

    uint64_t cycles = e.getCounter("cycles");
    t1 = seconds();
    double elapsed = t1 - t0;
    double total_bytes = 3.0 * (double)bytes * (double)repeats;
    double size_mb = (double)bytes / (1024.0 * 1024.0);

    std::printf("%.2f MB: %.3f s  %.2f GB/s (%zu repeats) cycles=%zu\n",
                size_mb, elapsed, total_bytes / elapsed / 1e9, repeats, cycles);

    free(a); free(b); free(c);
}

int main() {
    omp_set_num_threads(omp_get_max_threads());

    run_test(N_L1, "L1 Test");
    run_test(N_L2, "L2 Test");
    run_test(N_L3, "L3 Test");
    run_test(N_DDR, "DDR Test");

    return 0;
}
