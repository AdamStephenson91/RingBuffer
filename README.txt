This program provides a demonstration of writing and reading from a high performance,
lock-free ring buffer with variable-size, semi-random simulated market data packets.

It can be built by running make, with Google Benchmark as a dependency.

Usage:
./RingBufferDemo <flags>
Possible flags:
    --log:
        This causes the generated market data packets to be dumped to inputData.txt before they're passed through the buffer,
        and to be dumped to outputData.txt after being read by the consumer thread.
        This allows a diff between these files to verify data integrity.
    --benchmark:
        Enables Google Benchmark to measure performance.
        The buffer will be run with different buffer sizes.
        This should be run without using the --log flag.
    --verbose:
        This causes more information to be displayed in stdout,
        such as total packets processed, and number of times the ring buffer looped.
        It should usually not be enabled alongside benchmarking.

Recommended usage:
./RingBufferDemo --log --verbose
    A single run of the buffer that provides data input and output files,
    with extra information on the console.
./RingBufferDemo --benchmark
    Benchmarking with logging disabled for true performance.

