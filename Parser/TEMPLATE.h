#pragma once

#include <cstring>
#include <string_view>
#include <cstdint>
#include <new>
#include <iostream>

class %%MESSAGE%%
{
    private:
        unsigned char buffer_[%%MESSAGE_SIZE%%];
        static constexpr std::size_t messageSize_{%%MESSAGE_SIZE%%};

    public:
        %%MESSAGE%%() = default;

        constexpr std::size_t GetSize() { return messageSize_; }

        IF %%MESSAGE%% != HEADER
        constexpr unsigned char GetMessageType() { 
            return '%%MESSAGE_TYPE%%'; 
        }

        bool load(const unsigned char* buffer, std::size_t size)
        {
            if(size < %%MESSAGE_SIZE%%) [[unlikely]]
            {
                return false;
            }

            std::memcpy(buffer_, buffer, %%MESSAGE_SIZE%%);
            return true;
        }

        bool load(std::string_view sv)
        {
            return load(reinterpret_cast<const unsigned char*>(sv.data()), sv.size());
        }

        IF %%TYPE%% == int64
        uint64_t %%FIELD%%() const
        {
            uint64_t val;
            std::memcpy(&val, buffer_ + %%OFFSET%%, sizeof(val));
            return val;
        }

        IF %%TYPE%% == int32
        uint32_t %%FIELD%%() const
        {
            uint32_t val;
            std::memcpy(&val, buffer_ + %%OFFSET%%, sizeof(val));
            return val;
        }

        IF %%TYPE%% == char
        unsigned char %%FIELD%%() const
        {
            return buffer_[%%OFFSET%%];
        }

        IF %%TYPE%% == chararray
        std::string_view %%FIELD%%() const
        {
            return std::string_view(reinterpret_cast<const char*>(buffer_ + %%OFFSET%%), %%SIZE%%);
        }

        friend std::ostream& operator<<(std::ostream& os, const %%MESSAGE%%& obj)
        {
            os << "%%MESSAGE%%:\n";
            IF %%TYPE%% == char
            {
            os << "%%FIELD%%: " << obj.%%FIELD%%() << "\n";
            }
            IF %%TYPE%% == int32
            {
            os << "%%FIELD%%: " << obj.%%FIELD%%() << "\n";
            }
            IF %%TYPE%% == int64
            {
            os << "%%FIELD%%: " << obj.%%FIELD%%() << "\n";
            }
            IF %%TYPE%% == chararray
            {
            std::string_view txt = obj.%%FIELD%%();
            if (auto pos = txt.find('\0'); pos != std::string_view::npos)
                txt = txt.substr(0, pos);
            os << "%%FIELD%%: " << txt << "\n";
            }
            
            return os;
        }

};
