#pragma once

#include <cstring>
#include <string_view>
#include <cstdint>
#include <new>
#include <iostream>

class ORDER_EXECUTED
{
    private:
        unsigned char buffer_[46];
        static constexpr std::size_t messageSize_{46};

    public:
        ORDER_EXECUTED() = default;

        constexpr std::size_t GetSize() { return messageSize_; }

        constexpr unsigned char GetMessageType() { 
            return 'e'; 
        }

        bool load(const unsigned char* buffer, std::size_t size)
        {
            if(size < 46) [[unlikely]]
            {
                return false;
            }

            std::memcpy(buffer_, buffer, 46);
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
        uint64_t OrderIdAggressor() const
        {
            uint64_t val;
            std::memcpy(&val, buffer_ + 8, sizeof(val));
            return val;
        }
        uint64_t OrderIdPassive() const
        {
            uint64_t val;
            std::memcpy(&val, buffer_ + 16, sizeof(val));
            return val;
        }

        uint32_t PriceAtTrade() const
        {
            uint32_t val;
            std::memcpy(&val, buffer_ + 24, sizeof(val));
            return val;
        }
        uint32_t TradeSize() const
        {
            uint32_t val;
            std::memcpy(&val, buffer_ + 28, sizeof(val));
            return val;
        }


        std::string_view MMTFlags() const
        {
            return std::string_view(reinterpret_cast<const char*>(buffer_ + 32), 14);
        }

        friend std::ostream& operator<<(std::ostream& os, const ORDER_EXECUTED& obj)
        {
            os << "ORDER_EXECUTED:\n";
            {
            os << "PriceAtTrade: " << obj.PriceAtTrade() << "\n";
            }
            {
            os << "TradeSize: " << obj.TradeSize() << "\n";
            }
            {
            os << "InstrumentId: " << obj.InstrumentId() << "\n";
            }
            {
            os << "OrderIdAggressor: " << obj.OrderIdAggressor() << "\n";
            }
            {
            os << "OrderIdPassive: " << obj.OrderIdPassive() << "\n";
            }
            {
            std::string_view txt = obj.MMTFlags();
            if (auto pos = txt.find('\0'); pos != std::string_view::npos)
                txt = txt.substr(0, pos);
            os << "MMTFlags: " << txt << "\n";
            }
            
            return os;
        }

};

