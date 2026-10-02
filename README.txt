# High-Performance Lock-Free Ring Buffer

A demonstration of a multi-threaded, lock-free atomic ring buffer designed for low-latency ingest pipelines. This buffer safely passes simulated, variable-size market data packets between producer and consumer threads without mutual exclusion locks.

Additionally, the project features a **Tcl-based code generation parser** that automatically synthesizes optimized C++ access classes from a declarative field specification—a standard pattern used in production ultra-low latency feed handlers to eliminate manual serialization boilerplate.

## Key Technical Features
* **Lock-Free Concurrency:** Utilizes `std::atomic` memory barriers (sequentially consistent/acquire-release semantics) to coordinate high-throughput thread boundaries without context switching overhead.
* **Variable-Size Packet Handling:** Architected to handle dynamic packet lengths natively within a contiguous memory space, minimizing heap allocations.
* **Metaprogramming / Code Gen:** Features a custom Tcl engine (`/Parser`) to parse field specs and emit optimized C++ boilerplate classes for message serialization/deserialization.
* **Performance Benchmarking:** Integrated with **Google Benchmark** to analyze performance metrics across variable ring buffer sizes.
* **Deterministic Verification:** Built-in verification flags allowing zero-loss bitwise data integrity validation (`diff` mapping) between raw ingress and egress streams.

## Prerequisites & Dependencies
* **C++ Compiler:** Supporting C++17 or higher
* **Build System:** GNU Make
* **Scripting Engine:** Tcl (for header code generation)
* **Benchmarking:** Google Benchmark library

## Installation & Compilation
Clone the repository and compile using the provided Makefile:
```bash
git clone https://github.com
cd RingBuffer
make
```

## Usage & Execution
The binary supports multiple execution paths via runtime flags:

### 1. Data Integrity & Verification Mode
Generates market data packets, dumps them to `inputData.txt` on ingress, and outputs them to `outputData.txt` on egress. Use this to verify zero-loss data streams via file comparison.
```bash
./RingBufferDemo --log --verbose
diff inputData.txt outputData.txt
```

### 2. High-Performance Benchmarking Mode
Runs the buffer through automated Google Benchmark routines across varying allocations. *Note: Ensure logging is disabled during performance sweeps to prevent disk I/O bottlenecks.*
```bash
./RingBufferDemo --benchmark
```

## Architecture Notes
* **Cache Alignment:** Critical structures are padded to prevent false sharing across CPU cache lines (`hardware_destructive_interference_size`).
* **Zero Allocations in Path:** All message movement occurs over pre-allocated ring memory to guarantee deterministic performance execution bounds.
