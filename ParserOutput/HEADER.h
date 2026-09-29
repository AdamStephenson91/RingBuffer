#pragma once

#include <cstring>
#include <string_view>
#include <cstdint>
#include <new>
#include <iostream>

class HEADER
{
    private:
        unsigned char buffer_[9];
        static constexpr std::size_t messageSize_{9};

    public:
        HEADER() = default;

        constexpr std::size_t GetSize() { return messageSize_; }


        bool load(const unsigned char* buffer, std::size_t size)
        {
            if(size < 9) [[unlikely]]
            {
                return false;
            }

            std::memcpy(buffer_, buffer, 9);
            return true;
        }

        bool load(std::string_view sv)
        {
            return load(reinterpret_cast<const unsigned char*>(sv.data()), sv.size());
        }

        uint64_t SequenceNumber() const
        {
            uint64_t val;
            std::memcpy(&val, buffer_ + 0, sizeof(val));
            return val;
        }


        unsigned char MessageType() const
        {
            return buffer_[8];
        }


        friend std::ostream& operator<<(std::ostream& os, const HEADER& obj)
        {
            os << "HEADER:\n";
            {
            os << "MessageType: " << obj.MessageType() << "\n";
            }
            {
            os << "SequenceNumber: " << obj.SequenceNumber() << "\n";
            }
            
            return os;
        }

};

