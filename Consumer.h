#pragma once
#include "MESSAGES.h"
#include <cstddef>
#include <string_view>
#include <immintrin.h>
#include <iostream>
#include <fstream>

template<std::size_t capacity>
class Buffer;

class Consumer {
    public:
        Consumer() = default;

        template<std::size_t capacity>
        void ReadData(Buffer<capacity>& ringBuffer, bool logData, bool verbose);

        void HandleMessage(ANNOUNCEMENT&   msg);
        void HandleMessage(ORDER_ADD&      msg);
        void HandleMessage(ORDER_MODIFY&   msg);
        void HandleMessage(ORDER_CANCEL&   msg);
        void HandleMessage(ORDER_EXECUTED& msg);

        void Reset() { currentSeq_ = 0; }

    private:

        void ParseData(std::string_view message);

        void FailedMessageLoad(std::string_view message) {
            std::cout << "Failed to load message:\n" 
                      << message << std::endl;
        }

        // File to dump ring buffer reads to, if logging enabled
        std::ofstream outputFile_;

        // Last received sequence number
        size_t currentSeq_{0};

        // Global message instances
        HEADER header_;
        ANNOUNCEMENT announcement_;
        ORDER_ADD orderAdd_;
        ORDER_MODIFY orderModify_;
        ORDER_CANCEL orderCancel_;
        ORDER_EXECUTED orderExecuted_;
};

template<std::size_t capacity>
void Consumer::ReadData(Buffer<capacity>& ringBuffer, bool logData, bool verbose)
{
    if(logData)
    {
        outputFile_.open("outputData.txt");
        if(!outputFile_.is_open()) {
            std::cerr << "Error: could not open output file, aborting" << std::endl;
            return;
        }
    }

    std::string_view message;

    unsigned int i = 0;
    while (true) {
        if (ringBuffer.Read(message)) {
            ParseData(message);
            i++;
            // Advance the read pointer now that we are done with it
            ringBuffer.ReadComplete();

        } else {
            // Buffer is empty, check if writer is finished
            if (ringBuffer.IsWriteDone()) [[unlikely]] {
                break; // No more data, exit loop
            }

            _mm_pause(); // Still writing and empty
        }
    }
    Reset();

    if(verbose) {
        std::cout << "Reader complete, read: " << i 
                  << " messages from the buffer" << std::endl;
        std::cout << "Buffer looped: " << ringBuffer.loopCount_ 
                  << " times" << std::endl;
    }
}

void Consumer::ParseData(std::string_view message)
{
    if (message.size() < header_.GetSize()) [[unlikely]] {
        return;
    }

    const unsigned char* headerPtr =
            reinterpret_cast<const unsigned char*>(message.data());
 
    if(header_.load(headerPtr, header_.GetSize()))
    {
        uint64_t seq = header_.SequenceNumber();
        if(seq != currentSeq_ + 1)
        {
            std::cout << "ERR received sequence out of order:\n"
                      << "Received: " << seq << ", expected: " 
                      << currentSeq_ << std::endl;
        }
        currentSeq_ = seq;

        outputFile_ << header_ << std::endl;
        // Advance to the main message
        message.remove_prefix(header_.GetSize());

        const unsigned char* msgPtr = 
            reinterpret_cast<const unsigned char*>(message.data());

        switch(header_.MessageType())
        {
            case announcement_.GetMessageType():
            {
                if(announcement_.load(msgPtr, message.size())) [[likely]] {
                    HandleMessage(announcement_);
                } else {
                    FailedMessageLoad(message);
                }

                break;
            }
            case orderAdd_.GetMessageType():
            {
                if(orderAdd_.load(msgPtr, message.size())) [[likely]] {
                    HandleMessage(orderAdd_);
                } else {
                    FailedMessageLoad(message);
                }
                break;
            }
            case orderModify_.GetMessageType():
            { 
                if(orderModify_.load(msgPtr, message.size())) [[likely]] {
                    HandleMessage(orderModify_);
                } else {
                    FailedMessageLoad(message);
                }
                break;
            }
            case orderCancel_.GetMessageType():
            {
                if(orderCancel_.load(msgPtr, message.size())) [[likely]] {
                    HandleMessage(orderCancel_);
                } else {
                    FailedMessageLoad(message);
                }
                break;
            }
            case orderExecuted_.GetMessageType():
            {
                if(orderExecuted_.load(msgPtr, message.size())) [[likely]] {
                    HandleMessage(orderExecuted_);
                } else {
                    FailedMessageLoad(message);
                }
                break;
            }
            default:
            {
                std::cout << "ERR: Unknown message type received: "
                          << header_.MessageType() << std::endl;
                break;
            }
        }
    }
    else
    {
        std::cout << "Failed to parse header in message:\n" << message << std::endl;
    }
}

void Consumer::HandleMessage(ANNOUNCEMENT&   msg)
{
    outputFile_ << msg << std::endl;
}

void Consumer::HandleMessage(ORDER_ADD&      msg)
{
    outputFile_ << msg << std::endl;
}

void Consumer::HandleMessage(ORDER_MODIFY&   msg)
{
    outputFile_ << msg << std::endl;
}

void Consumer::HandleMessage(ORDER_CANCEL&   msg)
{
    outputFile_ << msg << std::endl;
}

void Consumer::HandleMessage(ORDER_EXECUTED& msg)
{
    outputFile_ << msg << std::endl;
}

