#include "InfernalMemoryAllocator.hpp"

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>

struct Payload { uint64_t data[2]; };

const int TEST_BATCH_SIZE = 1000;

inline void escape(void* Ptr);
void worker_malloc(size_t iterations);
void worker_pool(InfernalMemoryAllocator<Payload>& pool, size_t iterations);

int main() {
    const unsigned int num_threads = std::thread::hardware_concurrency();
    const size_t iterations_per_thread = 50000000;
    InfernalMemoryAllocator<Payload> pool(10000000); 

    std::cout << " -----------------------------------------------------------------------" << std::endl;
    std::cout << "   IMA17X - INFERNAL MEMORY ALLOCATOR 17X - BENCHMARK TEST VS MALLOC.            " << std::endl;
    std::cout << " -----------------------------------------------------------------------" << std::endl;
    std::cout << "\n Hardware Threads          : " << num_threads << "." << std::endl;
    std::cout << " Total Operations          : " << (num_threads * iterations_per_thread) << "." << std::endl;
    std::cout << " Operation per Thread      : " << iterations_per_thread << "." << std::endl;
    std::cout << " Payload Size              : " << sizeof(Payload) << " bytes" << "." << std::endl;
    
    auto start_pool = std::chrono::high_resolution_clock::now(); {
        std::vector<std::thread> threads;
        for (unsigned int i = 0; i < num_threads; ++i) 
            threads.emplace_back(worker_pool, std::ref(pool), iterations_per_thread);
        for (auto& t : threads) t.join();
    } auto end_pool = std::chrono::high_resolution_clock::now();
    
    auto start_malloc = std::chrono::high_resolution_clock::now(); {
        std::vector<std::thread> threads;
        for (unsigned int i = 0; i < num_threads; ++i) 
            threads.emplace_back(worker_malloc, iterations_per_thread);
        for (auto& t : threads) t.join();
    } auto end_malloc = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double> d_pool = end_pool - start_pool;
    std::chrono::duration<double> d_malloc = end_malloc - start_malloc;

    double speedup = d_malloc.count() / d_pool.count();

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "\n RESULTS:\n";
    std::cout << " ---------------------------------------------------------" << std::endl;
    std::cout << " INFERNAL MEMORY ALLOCATOR : " << d_pool.count() << " seconds" << "." << std::endl;
    std::cout << " System Std Malloc         : " << d_malloc.count() << " seconds" << "." << std::endl;
    std::cout << " ---------------------------------------------------------" << std::endl;
    std::cout << "\n SPEEDUP                   : " << speedup << "x faster" << "." << std::endl; // 27.

    if (speedup > 10.0) { std::cout << "\n INSANE PERFORMANCE ACHIEVED!\n" << std::endl; }

    return 0;
} // main();

inline void escape(void* Ptr) {
#if defined(__GNUC__) || defined(__clang__)
    asm volatile("" : : "g"(Ptr) : "memory");
#endif
}

void worker_malloc(size_t iterations) {
    std::vector<Payload*> batch(TEST_BATCH_SIZE);
    size_t loops = iterations / TEST_BATCH_SIZE;
    
    for (size_t i = 0; i < loops; ++i) {
        for (int j = 0; j < TEST_BATCH_SIZE; ++j) {
            batch[j] = static_cast<Payload*>(std::malloc(sizeof(Payload)));
            escape(batch[j]);
        } for (int j = 0; j < TEST_BATCH_SIZE; ++j) { std::free(batch[j]); }
    } // Se.
} // worker_malloc();

void worker_pool(InfernalMemoryAllocator<Payload>& pool, size_t iterations) {
    std::vector<Payload*> batch(TEST_BATCH_SIZE);
    size_t loops = iterations / TEST_BATCH_SIZE;
    
    for (size_t i = 0; i < loops; ++i) {
        for (int j = 0; j < TEST_BATCH_SIZE; ++j) {
            batch[j] = pool.Allocate();
            escape(batch[j]);
        } for (int j = 0; j < TEST_BATCH_SIZE; ++j) { pool.Deallocate(batch[j]); }
    } // Se.
} // worker_pool();
