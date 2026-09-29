#pragma once

#include <cstring>
#include <cstdint>
#include <atomic>
#include <array>
#include <bit>
#include <string_view>

namespace BufferConfig {
    inline constexpr std::size_t HEADER_SIZE = sizeof(std::size_t);
}

template <std::size_t capacity>
class Buffer
{
    static_assert(std::has_single_bit(capacity),
            "Buffer capacity must be power of 2");

    public:
        explicit Buffer() {};
        int loopCount_{0};

        bool Push(unsigned char* data, std::size_t size)
        {
            std::size_t current_write = writeIndex.load(std::memory_order_relaxed);

            // incr size so we can insert size value header
            std::size_t sizeHeader = size + BufferConfig::HEADER_SIZE;

            std::size_t start_write = current_write;

            bool needsPadding = ((start_write & (capacity -1)) + sizeHeader > capacity);
            if(needsPadding) [[unlikely]]
            {
                start_write += (capacity - (start_write & (capacity -1)));
            }

            std::size_t end_write = start_write + sizeHeader;
            // Advance end index to the nearest 8 byte aligned address
            end_write = ((end_write + 7) & ~7);

            if(end_write >= readIndex.load(std::memory_order_acquire) + capacity)
            {
                return false;
            }

            if(needsPadding) [[unlikely]]
            {
                loopCount_++;
                // message won't fit within capacity, write '0' size header to tell reader
                // (Marked unlikely as this will only happen once per loop)
                std::size_t paddingIndicator = 0;
                std::memcpy(&BUFFER[current_write & (capacity -1)], &paddingIndicator, BufferConfig::HEADER_SIZE);
            }

            // Now write size header and data
            std::memcpy(&BUFFER[start_write & (capacity -1)], &sizeHeader, BufferConfig::HEADER_SIZE);
            std::memcpy(&BUFFER[(start_write + BufferConfig::HEADER_SIZE) & (capacity -1)], data, size);

            writeIndex.store(end_write, std::memory_order_release);
            return true;
        }

        bool Read(std::string_view& outView)
        {
            std::size_t current_read = readIndex.load(std::memory_order_relaxed);

            if(current_read == writeIndex.load(std::memory_order_acquire))
            {
                return false;
            }

            // length of message stored in header
            std::size_t readSize{0};
            std::memcpy(&readSize, &BUFFER[current_read & (capacity -1)], BufferConfig::HEADER_SIZE);

            // Read size of 0 indicates message won't fit within buffer,
            // Writer has looped back to the start again
            // (Marked unlikely as this will only happen once per loop)
            if(readSize == 0) [[unlikely]]
            {
                // Loop reader back to start of buffer
                current_read += (capacity - (current_read & (capacity - 1)));

                if(current_read == writeIndex.load(std::memory_order_acquire))
                {
                    return false;
                }

                std::memcpy(&readSize, &BUFFER[current_read & (capacity -1)], BufferConfig::HEADER_SIZE);
            }
            // read size includes size header, decrease for string view length
            readSize -= BufferConfig::HEADER_SIZE;
            // construct string view of data using size
            outView = std::string_view(&BUFFER[(current_read + BufferConfig::HEADER_SIZE) & (capacity -1)], readSize);

            return true;
        }

        // Increment read pointer after consumer thread is finished working
        // on data returned by Read()
        void ReadComplete()
        {
            std::size_t current_read = readIndex.load(std::memory_order_relaxed);
            std::size_t readSize{0};
            std::memcpy(&readSize, &BUFFER[current_read & (capacity -1)], BufferConfig::HEADER_SIZE);

            // Advance past end-of-buffer padding
            if(readSize == 0) [[unlikely]]
            {
                current_read += (capacity - (current_read & (capacity - 1)));

                std::memcpy(&readSize, &BUFFER[current_read & (capacity -1)], BufferConfig::HEADER_SIZE);
            }

            // Increment by read size, and then add alignment padding to nearest multiple of 8 address
            current_read += readSize;
            current_read = ((current_read + 7) & ~7);
            readIndex.store(current_read, std::memory_order_release);
        }

        void SetWriteDone() noexcept {
            writeDone.store(true, std::memory_order_release);
        }

        [[nodiscard]] bool IsWriteDone() const noexcept {
            return writeDone.load(std::memory_order_acquire);
        }

    private:
        // Alignment to prevent buffer from sharing cache line space with write pointer
        // (since capacity is a power of 2, the buffer should end on a multiple of 64)
        alignas(std::hardware_destructive_interference_size)
        std::array<char, capacity> BUFFER;

        alignas(std::hardware_destructive_interference_size)
        std::atomic<bool> writeDone{false};

        // separate by at least a cache line to prevent contention between threads
        alignas(std::hardware_destructive_interference_size)
        std::atomic<std::size_t> writeIndex{0};

        alignas(std::hardware_destructive_interference_size)
        std::atomic<std::size_t> readIndex{0};
};
