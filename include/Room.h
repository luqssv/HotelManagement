#ifndef ROOM_H
#define ROOM_H

#include <string>

enum class RoomStatus {
    AVAILABLE,
    OCCUPIED,
    RESERVED,
    MAINTENANCE
};

enum class RoomType {
    SINGLE,
    DOUBLE,
    SUITE
};

class Room{
    private:
        int id;
        int roomNumber;
        RoomType type;
        double price;
        RoomStatus status;
    public:
        Room();
        Room(int id, int roomNumber, RoomType type, double price, RoomStatus status);
        int getId() const;
        int getRoomNumber() const;
        
        RoomType getType() const;
        double getPrice() const;
        RoomStatus getStatus() const;

        void setStatus(RoomStatus status);

        void display() const;

        static std::string statusToString(RoomStatus status);
        static std::string typeToString(RoomType type);
};

#endif