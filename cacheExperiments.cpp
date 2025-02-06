#include <algorithm>
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>  
#include <fstream>
#include <sstream>

#include <sched.h>    
#include <unistd.h>   

constexpr size_t KB = 1024;
constexpr size_t MB = 1024 * KB;
constexpr int NUM_TRIALS = 5;

void pin_to_cpu(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    
    if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &cpuset) != 0) {
        std::cerr << "Failed to set CPU affinity\n";
    } else {
        std::cout << "Pinned to CPU " << core_id << "\n";
    }
}

// Reads cache size from Linux sysfs
size_t get_cache_size(int level) {
    std::string path = "/sys/devices/system/cpu/cpu0/cache/index" + std::to_string(level) + "/size";
    std::ifstream file(path);
    if (!file) return 0;

    std::string line;
    std::getline(file, line);
    file.close();

    std::stringstream ss(line);
    size_t size;
    char suffix;
    ss >> size >> suffix;
    if (suffix == 'K') size *= KB;
    if (suffix == 'M') size *= MB;
    
    return size;
}

double benchmark_random_access(size_t array_size) {
    std::vector<int> data(array_size / sizeof(int), 1);
    volatile int sum = 0;  

    std::vector<size_t> indices(data.size());
    for (size_t i = 0; i < indices.size(); ++i) {
        indices[i] = i;
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices.begin(), indices.end(), g);

    double total_time = 0;

    for (int trial = 0; trial < NUM_TRIALS; ++trial) {
        auto start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < data.size(); ++i) {
            sum += data[indices[i]];
        }
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> duration = end - start;
        total_time += duration.count();
    }

    return total_time / NUM_TRIALS;  // Return average time
}


int main() {
    pin_to_cpu(0);

    size_t L1_SIZE = get_cache_size(0);
    size_t L2_SIZE = get_cache_size(2);
    size_t L3_SIZE = get_cache_size(3);
    size_t RAM_BOUNDARY = 128 * MB;  // Approximate RAM boundary

    std::cout << "Detected Cache Sizes:\n";
    std::cout << "L1 Cache: " << L1_SIZE / KB << " KB\n";
    std::cout << "L2 Cache: " << L2_SIZE / KB << " KB\n";
    std::cout << "L3 Cache: " << L3_SIZE / KB << " KB\n";
    std::cout << "RAM Boundary Approx: " << RAM_BOUNDARY / MB << " MB\n";

    std::cout << "\nTesting cache effects with random memory access...\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Size (KB)\tTime (ms)\tBandwidth (MB/s)\tNotes\n";

    for (size_t size = 32 * KB; size <= 512 * MB; size *= 2) {
        double avg_time = benchmark_random_access(size) * 1000; 
        double bandwidth = (size / (1024.0 * 1024.0)) / (avg_time / 1000.0);

        std::string notes = "";
        if (size == L1_SIZE) notes = "L1 Boundary ↓";
        if (size == L2_SIZE) notes = "L2 Boundary ↓";
        if (size == L3_SIZE) notes = "L3 Boundary ↓";
        if (size == RAM_BOUNDARY) notes = "RAM Boundary ↓";

        std::cout << size / KB << "\t\t" << avg_time << "\t\t" << bandwidth << "\t\t" << notes << "\n";
    }

    return 0;
}
