///////////////////////////////////////////////////////////////////////////////////////////////////
#include "InfernalMemoryAllocator.hpp"

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>

struct Payload { uint64_t data[2]; };

inline void escape(void* Ptr);
const int TEST_BATCH_SIZE = 1000;

void worker_pool(InfernalMemoryAllocator<Payload>& pool, size_t iterations);

int main() {
    const unsigned int num_threads = std::thread::hardware_concurrency();
    const size_t iterations_per_thread = 50000000;
    InfernalMemoryAllocator<Payload> pool(10000000); 

    std::cout << "\n -----------------------------------------------------------------------\n";
    std::cout << "    	      IMA17X - INFERNAL MEMORY ALLOCATOR BENCHMARK\n";
    std::cout << " -----------------------------------------------------------------------\n";
    std::cout << "\n Hardware Threads          : " << num_threads << ".\n";
    std::cout << " Total Operations          : " << (num_threads * iterations_per_thread) << ".\n";
    std::cout << " Operation per Thread      : " << iterations_per_thread << ".\n";
    std::cout << " Payload Size              : " << sizeof(Payload) << " bytes" << ".\n";
    
    std::cout << std::fixed << std::setprecision(6);
    
    std::cout << "\n\n Running Infernal Memory Allocator Benchmark..." << std::endl;
    auto start_pool = std::chrono::high_resolution_clock::now(); {
        std::vector<std::thread> threads;
        for (unsigned int K = 0; K < num_threads; ++K) 
            threads.emplace_back(worker_pool, std::ref(pool), iterations_per_thread);
        for (auto& G : threads) G.join();
    } auto end_pool = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double> d_pool = end_pool - start_pool;
    
    std::cout << "\n IMA17X finished in: " << d_pool.count() << " seconds.\n";
	std::cout << "\n -----------------------------------------------------------------------\n\n";
    return 0;
} // main();

inline void escape(void* Ptr) {
#if defined(__GNUC__) || defined(__clang__)
    asm volatile("" : : "g"(Ptr) : "memory");
#endif
}

void worker_pool(InfernalMemoryAllocator<Payload>& pool, size_t iterations) {
    std::vector<Payload*> batch(TEST_BATCH_SIZE);
    size_t loops = iterations / TEST_BATCH_SIZE;
    
    for (size_t K = 0; K < loops; ++K) {
        for (int G = 0; G < TEST_BATCH_SIZE; ++G) {
            batch[G] = pool.Allocate();
            	escape(batch[G]);
        } for (int N = 0; N < TEST_BATCH_SIZE; ++N) { pool.Deallocate(batch[N]); }
    } // Se.
} // worker_pool();

///////////////////////////////////////////////////////////////////////////////////////////////////
