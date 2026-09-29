#pragma once

#include <cstring>
#include <string_view>
#include <cstdint>
#include <new>
#include <iostream>

class ORDER_MODIFY
{
    private:
        unsigned char buffer_[25];
        static constexpr std::size_t messageSize_{25};

    public:
        ORDER_MODIFY() = default;

        constexpr std::size_t GetSize() { return messageSize_; }

        constexpr unsigned char GetMessageType() { 
            return 'm'; 
        }

        bool load(const unsigned char* buffer, std::size_t size)
        {
            if(size < 25) [[unlikely]]
            {
                return false;
            }

            std::memcpy(buffer_, buffer, 25);
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

        uint32_t OrderSize() const
        {
            uint32_t val;
            std::memcpy(&val, buffer_ + 16, sizeof(val));
            return val;
        }
        uint32_t OrderPrice() const
        {
            uint32_t val;
            std::memcpy(&val, buffer_ + 20, sizeof(val));
            return val;
        }

        unsigned char OrderSide() const
        {
            return buffer_[24];
        }


        friend std::ostream& operator<<(std::ostream& os, const ORDER_MODIFY& obj)
        {
            os << "ORDER_MODIFY:\n";
            {
            os << "OrderSide: " << obj.OrderSide() << "\n";
            }
            {
            os << "OrderSize: " << obj.OrderSize() << "\n";
            }
            {
            os << "OrderPrice: " << obj.OrderPrice() << "\n";
            }
            {
            os << "InstrumentId: " << obj.InstrumentId() << "\n";
            }
            {
            os << "OrderId: " << obj.OrderId() << "\n";
            }
            
            return os;
        }

};

