#pragma once

#include <cstring>
#include <string_view>
#include <cstdint>
#include <new>
#include <iostream>

class ORDER_CANCEL
{
    private:
        unsigned char buffer_[16];
        static constexpr std::size_t messageSize_{16};

    public:
        ORDER_CANCEL() = default;

        constexpr std::size_t GetSize() { return messageSize_; }

        constexpr unsigned char GetMessageType() { 
            return 'c'; 
        }

        bool load(const unsigned char* buffer, std::size_t size)
        {
            if(size < 16) [[unlikely]]
            {
                return false;
            }

            std::memcpy(buffer_, buffer, 16);
            return true;
        }

        bool load(std::string_view sv)
        {
            return load(reinterpret_cast<const unsigned char*>(sv.data()), sv.size());
        }

        uint64_t InstrumentId() const
        {
            uint64_t val;
            std::memcpy(&val, buffer_ + 0, sizeof(val));
            return val;
        }
        uint64_t OrderId() const
        {
            uint64_t val;
            std::memcpy(&val, buffer_ + 8, sizeof(val));
            return val;
        }




        friend std::ostream& operator<<(std::ostream& os, const ORDER_CANCEL& obj)
        {
            os << "ORDER_CANCEL:\n";
            {
            os << "InstrumentId: " << obj.InstrumentId() << "\n";
            }
            {
            os << "OrderId: " << obj.OrderId() << "\n";
            }
            
            return os;
        }

};

