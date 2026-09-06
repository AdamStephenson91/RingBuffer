#pragma once

#include <cstring>
#include <string_view>
#include <cstdint>
#include <new>
#include <iostream>

class ANNOUNCEMENT
{
    private:
        unsigned char buffer_[258];
        size_t   messageSize_{258};

    public:
        ANNOUNCEMENT() = default;

        size_t GetSize() { return messageSize_; }

        constexpr unsigned char GetMessageType() { 
            return 'a'; 
        }

        bool load(const unsigned char* buffer, size_t size)
        {
            if(size < 258) [[unlikely]]
            {
                return false;
            }

            std::memcpy(buffer_, buffer, 258);
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



        std::string_view Announcement() const
        {
            return std::string_view(reinterpret_cast<const char*>(buffer_ + 8), 250);
        }

        friend std::ostream& operator<<(std::ostream& os, const ANNOUNCEMENT& obj)
        {
            os << "ANNOUNCEMENT:\n";
            {
            os << "InstrumentId: " << obj.InstrumentId() << "\n";
            }
            {
            std::string_view txt = obj.Announcement();
            if (auto pos = txt.find('\0'); pos != std::string_view::npos)
                txt = txt.substr(0, pos);
            os << "Announcement: " << txt << "\n";
            }
            
            return os;
        }

};

