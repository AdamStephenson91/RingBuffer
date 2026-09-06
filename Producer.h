#pragma once

#include "DummyMessages.h"
#include "Buffer.h"

#include <iostream>
#include <cstddef>
#include <array>
#include <vector>
#include <random>
#include <immintrin.h> // Required for _mm_pause()
#include <cstring>
#include <fstream>

template <std::size_t msgArraySize>
class Producer {
    public:
        Producer() = default;

        bool InitData(bool logData, bool verbose);

        template<std::size_t capacity>
        void WriteData(Buffer<capacity>& ringBuffer, bool verbose);

    private:
        std::array<unsigned char, msgArraySize> MSG_ARRAY{};
};

template <std::size_t msgArraySize>
bool Producer<msgArraySize>::InitData(bool logData, bool verbose) {

    if(verbose)
        std::cout << "Producer data pool size: " << msgArraySize << " bytes" << std::endl;

    // output file for dumping input messages if logData enabled
    std::ofstream outputFile;

    if(logData)
    {
        outputFile.open("inputData.txt");
        if(!outputFile.is_open()) {
            std::cerr << "Error: could not open input file, aborting" << std::endl;
            return false;
        }
    }

    // Populate char buffer with an assortment of dummy messages
    enum class MsgChoice {
            Announcement = 0, OrderAdd, OrderModify, OrderCancel, OrderExecuted, Count
    };

    // Size of each list of messages
    constexpr std::size_t PoolSize = msgArraySize / 20;

    std::vector<Announcement>   announcementPool(PoolSize);
    std::vector<OrderAdd>       addPool(PoolSize);
    std::vector<OrderModify>    modifyPool(PoolSize);
    std::vector<OrderCancel>    cancelPool(PoolSize);
    std::vector<OrderExecuted>  execPool(PoolSize);

    // count instances pushed to buffer for each message type
    int announceCount = 0;
    int orderAddCount = 0;
    int orderModCount = 0;
    int orderCanCount = 0;
    int orderExeCount = 0;

    // Serialise messages in random order
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int>(MsgChoice::Count) - 1);

    unsigned char*       writePtr = MSG_ARRAY.data();
    unsigned char* const endPtr   = MSG_ARRAY.data() + msgArraySize;

    uint64_t currentSequenceNumber = 1;

    while (writePtr < endPtr) {
        MsgChoice choice = static_cast<MsgChoice>(dis(gen));

        // Helper variables to track the sizes needed for this specific loop iteration
        std::size_t payloadSize = 0;
        unsigned char msgTypeChar = ' ';

        // Check which payload was chosen to figure out its size and type marker
        switch (choice) {
            case MsgChoice::Announcement:  payloadSize = sizeof(Announcement);
                                           msgTypeChar = Announcement::msgType;  break;
            case MsgChoice::OrderAdd:      payloadSize = sizeof(OrderAdd);
                                           msgTypeChar = OrderAdd::msgType;      break;
            case MsgChoice::OrderModify:   payloadSize = sizeof(OrderModify);
                                           msgTypeChar = OrderModify::msgType;   break;
            case MsgChoice::OrderCancel:   payloadSize = sizeof(OrderCancel);
                                           msgTypeChar = OrderCancel::msgType;   break;
            case MsgChoice::OrderExecuted: payloadSize = sizeof(OrderExecuted);
                                           msgTypeChar = OrderExecuted::msgType; break;
        }

        // Add 2 bytes containing total header + payload size, used by RingBuffer logic
        int16_t messageLengthField = static_cast<int16_t>(sizeof(Header) + payloadSize);

        // Total space required = Message length field + Header + Payload
        std::size_t totalRequiredSpace = sizeof(messageLengthField) + messageLengthField;

        // If the message won't fit in the remaining buffer, stop completely
        if (writePtr + totalRequiredSpace > endPtr) {
            break;
        }

        // Create header packet
        Header loopHeader{currentSequenceNumber, msgTypeChar};

        if(logData)
            outputFile << loopHeader;
        // Serialize the matching payload immediately behind the header
        switch (choice) {
            case MsgChoice::Announcement:
                if (!announcementPool.empty()) {
                    std::memcpy(writePtr, &messageLengthField, sizeof(int16_t));
                    writePtr += sizeof(int16_t);

                    // only write header if we have a valid message ready to pair with it
                    writePtr = loopHeader.Serialize(writePtr);
                    currentSequenceNumber++;

                    writePtr = announcementPool.back().Serialize(writePtr);

                    if(logData)
                        outputFile << announcementPool.back();

                    announcementPool.pop_back();

                    announceCount++;
                }
                break;

            case MsgChoice::OrderAdd:
                if (!addPool.empty()) {
                    std::memcpy(writePtr, &messageLengthField, sizeof(int16_t));
                    writePtr += sizeof(int16_t);

                    writePtr = loopHeader.Serialize(writePtr);
                    currentSequenceNumber++;

                    writePtr = addPool.back().Serialize(writePtr);

                    if(logData)
                        outputFile << addPool.back();

                    addPool.pop_back();

                    orderAddCount++;
                }
                break;

            case MsgChoice::OrderModify:
                if (!modifyPool.empty()) {
                    std::memcpy(writePtr, &messageLengthField, sizeof(int16_t));
                    writePtr += sizeof(int16_t);

                    writePtr = loopHeader.Serialize(writePtr);
                    currentSequenceNumber++;

                    writePtr = modifyPool.back().Serialize(writePtr);

                    if(logData)
                        outputFile << modifyPool.back();

                    modifyPool.pop_back();

                    orderModCount++;
                }
                break;

            case MsgChoice::OrderCancel:
                if (!cancelPool.empty()) {
                    std::memcpy(writePtr, &messageLengthField, sizeof(int16_t));
                    writePtr += sizeof(int16_t);

                    writePtr = loopHeader.Serialize(writePtr);
                    currentSequenceNumber++;

                    writePtr = cancelPool.back().Serialize(writePtr);

                    if(logData)
                        outputFile << cancelPool.back();

                    cancelPool.pop_back();

                    orderCanCount++;
                }
                break;

            case MsgChoice::OrderExecuted:
                if (!execPool.empty()) {
                    std::memcpy(writePtr, &messageLengthField, sizeof(int16_t));
                    writePtr += sizeof(int16_t);

                    writePtr = loopHeader.Serialize(writePtr);
                    currentSequenceNumber++;

                    writePtr = execPool.back().Serialize(writePtr);

                    if(logData)
                        outputFile << execPool.back();

                    execPool.pop_back();

                    orderExeCount++;
                }
                break;
        }

    }

    outputFile.close();

    if(verbose) {
        int totalCount = announceCount + orderAddCount + orderModCount +
                         orderCanCount + orderExeCount;

        std::cout << "Data population finished, message counts:\n"
                  << "Announcement:   " << announceCount << "\n"
                  << "Order Add:      " << orderAddCount << "\n"
                  << "Order Modify:   " << orderModCount << "\n"
                  << "Order Cancel:   " << orderCanCount << "\n"
                  << "Order Executed: " << orderExeCount << "\n"
                  << "Total:          " << totalCount    << std::endl;
    }

    return true;
}

template <std::size_t msgArraySize>
template <std::size_t capacity>
void Producer<msgArraySize>::WriteData(Buffer<capacity>& ringBuffer, bool verbose) {
    // Write data stored in MSG_ARRAY to RingBuffer
    unsigned char*       writePtr = MSG_ARRAY.data();
    unsigned char* const endPtr   = MSG_ARRAY.data() + msgArraySize;

    int pushCount = 0;
    while (writePtr < endPtr) {

        uint16_t currentMsgSize = 0;
        std::memcpy(&currentMsgSize, writePtr, sizeof(uint16_t));

        if (currentMsgSize == 0) [[unlikely]]
        {
            // messages won't fit in the buffer perfectly,
            // reached end of messages and found padding
            break;
        }

        if (ringBuffer.Push(writePtr + sizeof(uint16_t), currentMsgSize)) {
            // Advance past the 2-byte size prefix + the contents size
            pushCount ++;

            writePtr += sizeof(uint16_t) + currentMsgSize;
        } else {
            // Buffer is full, spin-loop until the reader thread advances
            _mm_pause();
        }
    }

    if(verbose)
    std::cout << "Writer done, pushed: " << pushCount << " elements to buffer" << std::endl;

    ringBuffer.SetWriteDone();
}

