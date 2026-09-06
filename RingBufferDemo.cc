#include "RingBufferDemo.h"
#include "Buffer.h"
#include <benchmark/benchmark.h>
#include <thread>
#include <iostream>
#include <sched.h>
#include <pthread.h>

// controls if we dump data before and after ring buffer transfer into files
// so we can diff them and check consistency
bool logData_ = false;

// turn on/off info messages such as number of messages generated,
// number of times buffer looped, when read and write threads finish, etc
bool verbose_ = false;

// Non-benchmarked run-through of ring buffer
template <size_t bufferSize>
int RunRingBufferDemo()
{
    Buffer<bufferSize> sharedRingBuffer;

    std::cout << "Ring buffer created with size of " << bufferSize
              << " bytes" << std::endl;
    std::cout << "Starting Ring Buffer read/write threads" << std::endl;
    std::jthread consumer_thread([&sharedRingBuffer]() {
            consumer.ReadData(sharedRingBuffer, logData_, verbose_);
    });

    std::jthread producer_thread([&sharedRingBuffer, &producer]() {
            producer.WriteData(sharedRingBuffer, verbose_);
    });

    std::cout << "Threads created, now processing..." << std::endl;
    return 0;
}

template <std::size_t BufferSize>
static void BM_RingBufferPerformance(benchmark::State& state) {
    // size of the producer's data cache to be passed through the ring buffer
    const std::size_t totalBytesProcessedPerRun = 100000;

    for (auto _ : state) {
        Buffer<BufferSize> sharedRingBuffer;

        std::jthread consumer_thread([&sharedRingBuffer]() {
            // pin consumer thread to core 0
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(0, &cpuset);
            pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

            consumer.ReadData(sharedRingBuffer, logData_, verbose_);
        });

        std::jthread producer_thread([&sharedRingBuffer]() {
            // pin producer to core 2 so it's on a different physical core
            // to the consumer thread (avoiding cache contention etc)
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(2, &cpuset);
            pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
            
            producer.WriteData(sharedRingBuffer, verbose_);
        });

    }

    state.SetBytesProcessed(state.iterations() * totalBytesProcessedPerRun);
}

// Run benchmark against different ring buffer sizes
BENCHMARK_TEMPLATE(BM_RingBufferPerformance, 1024);
BENCHMARK_TEMPLATE(BM_RingBufferPerformance, 2048);
BENCHMARK_TEMPLATE(BM_RingBufferPerformance, 4096);
BENCHMARK_TEMPLATE(BM_RingBufferPerformance, 8192);
BENCHMARK_TEMPLATE(BM_RingBufferPerformance, 16384);

int main(int argc, char** argv) {
    bool benchmark = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--log") {
            std::cout << "Data logging mode enabled.\n";
            logData_ = true;
        } else if (arg == "--verbose") {
            verbose_ = true;
        } else if (arg == "--benchmark") {
            benchmark = true;
        }
    }

    std::cout << "Initialising dummy packet data to be written...\n";
    if (!producer.InitData(logData_, verbose_)) {
        std::cerr << "ERROR: Data initialisation failed, exiting.\n";
        return 1;
    }
    std::cout << "Data initialisation complete.\n\n";

    if(benchmark)
    {
        // Initialize Google Benchmark after producer has initialised data
        ::benchmark::Initialize(&argc, argv);
    
        ::benchmark::RunSpecifiedBenchmarks();
        ::benchmark::Shutdown();
    }
    else
    {
        // run non-benchmark instance,
        // best used with verbose and logging enabled to test logic and data integrity
        RunRingBufferDemo<2048>();
    }
    return 0;
}
