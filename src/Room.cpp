#include "Room.h"
#include <iostream>

Room::Room(){
    id = 0;
    roomNumber = 0;
    type = RoomType::SINGLE;
    price = 0;
    status = RoomStatus::AVAILABLE;
}

Room::Room(int id, int roomNumber, RoomType type, double price, RoomStatus status){
    this->id = id;
    this->roomNumber = roomNumber;
    this->type = type;
    this->price = price;
    this->status = status;
}

int Room::getRoomNumber() const{
    return roomNumber;
}
RoomType Room::getType() const{
    return type;
}
double Room::getPrice() const{
    return price;
}
RoomStatus Room::getStatus() const{
    return status;
}

void Room::setStatus(RoomStatus status){
    this->status = status;
}

std::string Room::statusToString(RoomStatus status){
    switch (status)
    {
    case RoomStatus::AVAILABLE:
        return "AVAILABLE";
    case RoomStatus::OCCUPIED:
        return "OCCUPIED";
    case RoomStatus::RESERVED:
        return "RESERVED";
    case RoomStatus::MAINTENANCE:
        return "MAINTENANCE";
    }
    return "UNKNOWN";
}

std::string Room::typeToString(RoomType type){
    switch (type)
    {
    case RoomType::SINGLE:
        return "SINGLE";
    case RoomType::DOUBLE:
        return "DOUBLE";
    case RoomType::SUITE:
        return "SUITE";
    }
    return "UNKNOWN";
}

void Room::display() const{
    std::cout 
        << "Room: " << roomNumber
        << " | Type: " << typeToString(type)
        << " | Price: " << price << "EUR"
        << " | Status: " << statusToString(status)
        << '\n';
}
