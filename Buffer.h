#pragma once

#include <cstring>
#include <cstdint>
#include <atomic>
#include <array>
#include <bit>
#include <string_view>

template <std::size_t capacity>
class Buffer
{
    static_assert(std::has_single_bit(capacity),
            "Buffer capacity must be power of 2");

    public:
        explicit Buffer() {};
        int loopCount_{0};

        bool Push(unsigned char* data, int16_t size)
        {
            std::size_t current_write = writeIndex.load(std::memory_order_relaxed);

            // incr size by 2 so we can insert 2 byte size value header
            int16_t sizeHeader = size + 2;

            std::size_t target_write = current_write;

            bool needsPadding = ((current_write & (capacity -1)) + sizeHeader) > capacity;
            if(needsPadding) [[unlikely]]
            {
                target_write += (capacity - (current_write & (capacity -1)));
            }

            if(target_write + sizeHeader
                >= readIndex.load(std::memory_order_acquire) + capacity)
            {
                return false;
            }

            if(needsPadding) [[unlikely]]
            {
                loopCount_++;
                // message won't fit within capacity, write '0' size header to tell reader
                // (Marked unlikely as this will only happen once per loop)
                int16_t paddingIndicator = 0;
                std::memcpy(&BUFFER[current_write & (capacity -1)], &paddingIndicator, 2);

                current_write = target_write;
            }

            // Now write size header and data
            std::memcpy(&BUFFER[current_write & (capacity -1)], &sizeHeader, 2);
            std::memcpy(&BUFFER[(current_write + 2) & (capacity -1)], data, size);

            writeIndex.store(current_write + sizeHeader, std::memory_order_release);

            return true;
        }

        bool Read(std::string_view& outView)
        {
            std::size_t current_read = readIndex.load(std::memory_order_relaxed);

            if(current_read == writeIndex.load(std::memory_order_acquire))
            {
                return false;
            }

            // length of message stored in first 2 bytes
            int16_t readSize{0};
            std::memcpy(&readSize, &BUFFER[current_read & (capacity -1)], 2);

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

                std::memcpy(&readSize, &BUFFER[current_read & (capacity -1)], 2);
            }
            // read size includes 2 byte size header, decrease for string view length
            readSize -= 2;
            // construct string view of data using size
            outView = std::string_view(&BUFFER[(current_read + 2) & (capacity -1)], readSize);

            return true;
        }

        // Increment read pointer after consumer thread is finished working
        // on data returned by Read()
        void ReadComplete()
        {
            std::size_t current_read = readIndex.load(std::memory_order_relaxed);

            int16_t readSize{0};
            std::memcpy(&readSize, &BUFFER[current_read & (capacity -1)], 2);

            // Advance past end-of-buffer padding
            if(readSize == 0) [[unlikely]]
            {
                current_read += (capacity - (current_read & (capacity - 1)));

                std::memcpy(&readSize, &BUFFER[current_read & (capacity -1)], 2);
            }

            readIndex.store(current_read + readSize, std::memory_order_release);
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
