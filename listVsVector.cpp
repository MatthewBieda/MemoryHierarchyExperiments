#include <iostream>
#include <vector>
#include <list>
#include <chrono>
#include <random>

constexpr int NUM_INSERTIONS = 100000;

// Benchmarking random insertion in std::vector
double benchmark_vector_insertion() {
    std::vector<int> vec(100000, 0);
    vec.reserve(200000);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 100000);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_INSERTIONS; ++i) {
        int pos = dist(gen);
        vec.insert(vec.begin() + pos, i);
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end - start;
    return duration.count();
}

// Function to benchmark std::list insertions at random positions
double benchmark_list_insertion() {
    std::list<int> lst(NUM_INSERTIONS);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 100000);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_INSERTIONS; ++i) {
        int pos = dist(gen);
        auto it = lst.begin();
        std::advance(it, pos);
        lst.insert(it, i);
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end - start;
    return duration.count();
}

int main() {
    std::cout << "Benchmarking container insertions with " << NUM_INSERTIONS << " elements.\n";

    double vector_time = benchmark_vector_insertion();
    std::cout << "std::vector insertion time: " << vector_time << " seconds\n";

    double list_time = benchmark_list_insertion();
    std::cout << "std::list insertion time: " << list_time << " seconds\n";

    return 0;
}
