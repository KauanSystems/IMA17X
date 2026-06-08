# IMA17X - Ultra-Performance InfernalMemoryAllocator

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![C++](https://img.shields.io/badge/C++-20-00599C?logo=cplusplus)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/Platform-Linux%20x86__64-E34F26?logo=linux)](https://kernel.org)

**IMA17X** is a lock-free, ultra-low latency memory allocator designed for High-Frequency Trading (HFT) and environments where cache contention is the primary performance bottleneck.

---

## 📈 Benchmark: IMA17X vs System Malloc

Validation performed using `StressTest.cpp`, simulating extreme multithreaded load (**50,000,000 operations per thread**).

| Allocator | Throughput (Ops/Sec) | Execution Time | Speedup |
| :--- | :--- | :--- | :--- |
| 🟢 **Custom IMA17X** | **~126.8M ops/s** | **0.394321s** | **17.35x Faster** |
| 🔴 **System std::malloc** | ~7.2M ops/s | 6.886300s | Baseline |

> **Engineering Note:** Superior performance is achieved by total elimination of mutexes/locks and the physical isolation of sensitive data across distinct cache lines.

---

## 🛠️ Build & Run

Optimized for **Linux** (x86_64). Requires a compiler with C++20 support.

```bash
git clone https://github.com/KauanSystems/IMA17X.git

cd InfernalMemoryAllocator

mkdir build && cd build

cmake -DCMAKE_BUILD_TYPE=Release .. && make -j$(nproc)
```

---

## 🛠️ Low-Level Engineering (Deep Dive)

The project implements advanced CPU architecture concepts to extract maximum silicon performance:

### 1. False Sharing Mitigation (`alignas(128)`)
Critical structures, such as the `Atomic_Anchor`, are aligned to **128 bytes**. This ensures each thread operates on an exclusive cache line, eliminating "Cache Line Ping-Pong" and allowing performance to scale linearly with the number of CPU cores.

### 2. ABA Protection via Versioning (Tagging)
We utilize a protected atomic pointer (Tag + Pointer). Every address modification increments a 16-bit version tag, ensuring the allocator is immune to the ABA Problem (memory recycling errors) without the overhead of Garbage Collection or Hazard Pointers.

### 3. O(1) Allocation Logic
Block management algorithms operate in constant time, providing deterministic latency which is vital for real-time systems and financial engines.

---

## 📂 Project Structure

*   `include/`: Header-only templates for **MemoryAllocator** and **ABAProtectedPtr**.
*   `src/stress_test.cpp`: High-concurrency benchmark suite.

```text
.
├── include/       # Header-only templates
├── src/           # Implementation and tests
├── CMakeLists.txt # Build system configuration
└── README.md      # Technical documentation
```

---
*Developed by Kauan Gomes for high-frequency trading and low-latency system environments.*
