#pragma once

#include <iostream>
#include <cstdint>
#include <string_view>
#include <cstring>
#include <random>

#pragma pack(push, 1) // Remove struct padding for strict byte ordering,
                      // allowing us to parse at fixed offsets later

class RandomGenerator {
public:
    static uint64_t get_uint64(uint64_t min, uint64_t max) {
        std::uniform_int_distribution<uint64_t> dist(min, max);
        return dist(get_engine());
    }

    static uint32_t get_uint32(uint32_t min, uint32_t max) {
        std::uniform_int_distribution<uint32_t> dist(min, max);
        return dist(get_engine());
    }

    static unsigned char get_side() {
        std::uniform_int_distribution<unsigned short> dist(0, 1);
        return (dist(get_engine()) == 0) ? 'B' : 'S';
    }

private:
    static std::mt19937_64& get_engine() {
        thread_local std::mt19937_64 engine(std::random_device{}());
        return engine;
    }
};

struct Header {
    uint64_t SequenceNumber;
    unsigned char MessageType;

    unsigned char* Serialize(unsigned char* dest) const {
        std::memcpy(dest, this, sizeof(Header));
        return dest + sizeof(Header);
    }

    friend std::ostream& operator<<(std::ostream& os, const Header& obj) {
        os << "HEADER:\n"
           << "MessageType: " << obj.MessageType << "\n"
           << "SequenceNumber: " << obj.SequenceNumber << "\n"
           << "\n";
        return os;
    }
};

struct Announcement {
    uint64_t InstrumentId;
    unsigned char AnnouncementText[250];
    static constexpr char msgType = 'a';

    Announcement() {
        InstrumentId = RandomGenerator::get_uint64(1000, 1000000);

        std::snprintf(reinterpret_cast<char*>(AnnouncementText),
                     sizeof(AnnouncementText),
                     "This is an announcement message");
    }

    unsigned char* Serialize(unsigned char* dest) const {
        std::memcpy(dest, this, sizeof(Announcement));
        return dest + sizeof(Announcement);
    }

    friend std::ostream& operator<<(std::ostream& os, const Announcement& obj) {
        // Construct a safe view of the text buffer up to its limit
        std::string_view text(reinterpret_cast<const char*>(obj.AnnouncementText), sizeof(obj.AnnouncementText));

        // Strip trailing null characters or padding if present
        if (size_t pos = text.find('\0'); pos != std::string_view::npos) {
            text = text.substr(0, pos);
        }

        os << "ANNOUNCEMENT:\n"
           << "InstrumentId: " << obj.InstrumentId << "\n"
           << "Announcement: " << text << "\n"
           << "\n";
        return os;
    }
};

struct OrderAdd {
    uint64_t InstrumentId;
    uint64_t OrderId;
    uint32_t OrderSize;
    uint32_t OrderPrice;
    unsigned char OrderSide;
    static constexpr char msgType = 'o';

    OrderAdd() {
        InstrumentId = RandomGenerator::get_uint64(1000, 100000);
        OrderId      = RandomGenerator::get_uint64(1000, 100000);
        OrderSize    = RandomGenerator::get_uint32(1, 100);
        OrderPrice   = RandomGenerator::get_uint32(1, 100);
        OrderSide    = RandomGenerator::get_side();
    }

    unsigned char* Serialize(unsigned char* dest) const {
        std::memcpy(dest, this, sizeof(OrderAdd));
        return dest + sizeof(OrderAdd);
    }

    friend std::ostream& operator<<(std::ostream& os, const OrderAdd& obj) {
        os << "ORDER_ADD:\n"
           << "OrderSide: " << obj.OrderSide << "\n"
           << "OrderSize: " << obj.OrderSize << "\n"
           << "OrderPrice: " << obj.OrderPrice << "\n"
           << "InstrumentId: " << obj.InstrumentId << "\n"
           << "OrderId: " << obj.OrderId << "\n"
           << "\n";
        return os;
    }
};

struct OrderModify {
    uint64_t InstrumentId;
    uint64_t OrderId;
    uint32_t OrderSize;
    uint32_t OrderPrice;
    unsigned char OrderSide;
    static constexpr char msgType = 'm';

    OrderModify() {
        // Reuse the helper for numbers and sides
        InstrumentId = RandomGenerator::get_uint64(1000, 100000);
        OrderId      = RandomGenerator::get_uint64(1000, 100000);
        OrderSize    = RandomGenerator::get_uint32(1, 100);
        OrderPrice   = RandomGenerator::get_uint32(1, 100);
        OrderSide    = RandomGenerator::get_side();
    }

    unsigned char* Serialize(unsigned char* dest) const {
        std::memcpy(dest, this, sizeof(OrderModify));
        return dest + sizeof(OrderModify);
    }

    friend std::ostream& operator<<(std::ostream& os, const OrderModify& obj) {
        os << "ORDER_MODIFY:\n"
           << "OrderSide: " << obj.OrderSide << "\n"
           << "OrderSize: " << obj.OrderSize << "\n"
           << "OrderPrice: " << obj.OrderPrice << "\n"
           << "InstrumentId: " << obj.InstrumentId << "\n"
           << "OrderId: " << obj.OrderId << "\n"
           << "\n";
        return os;
    }
};

struct OrderCancel {
    uint64_t InstrumentId;
    uint64_t OrderId;
    static constexpr char msgType = 'c';

    OrderCancel() {
        // Reuse the helper for numbers and sides
        InstrumentId = RandomGenerator::get_uint64(100, 100000);
        OrderId      = RandomGenerator::get_uint64(100, 100000);
    }

    unsigned char* Serialize(unsigned char* dest) const {
        std::memcpy(dest, this, sizeof(OrderCancel));
        return dest + sizeof(OrderCancel);
    }

    friend std::ostream& operator<<(std::ostream& os, const OrderCancel& obj) {
        os << "ORDER_CANCEL:\n"
           << "InstrumentId: " << obj.InstrumentId << "\n"
           << "OrderId: " << obj.OrderId << "\n"
           << "\n";
        return os;
    }
};

struct OrderExecuted {
    uint64_t InstrumentId;
    uint64_t OrderIdAggressor;
    uint64_t OrderIdPassive;
    uint32_t PriceAtTrade;
    uint32_t TradeSize;
    unsigned char MMTFlags[15];
    static constexpr char msgType = 'e';

    OrderExecuted() {
        // Reuse the helper for numbers and sides
        InstrumentId     = RandomGenerator::get_uint64(1000, 100000);
        OrderIdAggressor = RandomGenerator::get_uint64(1000, 100000);
        OrderIdPassive   = RandomGenerator::get_uint64(1000, 100000);
        PriceAtTrade     = RandomGenerator::get_uint32(1, 100);
        TradeSize        = RandomGenerator::get_uint32(1, 100);
        
        std::snprintf(reinterpret_cast<char*>(MMTFlags),
                     sizeof(MMTFlags),
                     "12-D----P-A1--");
    }

    unsigned char* Serialize(unsigned char* dest) const {
        std::memcpy(dest, this, sizeof(OrderExecuted));
        return dest + sizeof(OrderExecuted);
    }

    friend std::ostream& operator<<(std::ostream& os, const OrderExecuted& obj) {
        std::string_view text(reinterpret_cast<const char*>(obj.MMTFlags), sizeof(obj.MMTFlags));

        if (size_t pos = text.find('\0'); pos != std::string_view::npos) {
            text = text.substr(0, pos);
        }

        os << "ORDER_EXECUTED:\n"
           << "PriceAtTrade: " << obj.PriceAtTrade << "\n"
           << "TradeSize: " << obj.TradeSize << "\n"
           << "InstrumentId: " << obj.InstrumentId << "\n"
           << "OrderIdAggressor: " << obj.OrderIdAggressor << "\n"
           << "OrderIdPassive: " << obj.OrderIdPassive << "\n"
           << "MMTFlags: " << text << "\n"
           << "\n";
        return os;
    }
};

#pragma pack(pop)
