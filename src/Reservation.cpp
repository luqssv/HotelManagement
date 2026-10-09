#include "Reservation.h"
#include <iostream>

Reservation::Reservation()
{
    id = 0;
    customerId = 0;
    roomId = 0;
    checkIn = "";
    checkOut = "";
    status = "RESERVED";
}

Reservation::Reservation(
    int id,
    int customerId,
    int roomId,
    const std::string& checkIn,
    const std::string& checkOut,
    const std::string& status
)
{
    this->id = id;
    this->customerId = customerId;
    this->roomId = roomId;
    this->checkIn = checkIn;
    this->checkOut = checkOut;
    this->status = status;
}

int Reservation::getId() const
{
    return id;
}

int Reservation::getCustomerId() const
{
    return customerId;
}

int Reservation::getRoomId() const
{
    return roomId;
}

std::string Reservation::getCheckIn() const
{
    return checkIn;
}

std::string Reservation::getCheckOut() const
{
    return checkOut;
}

std::string Reservation::getStatus() const
{
    return status;
}

void Reservation::setStatus(const std::string& status)
{
    this->status = status;
}

void Reservation::display() const
{
    std::cout
        << "Reservation ID: " << id
        << " | Customer ID: " << customerId
        << " | Room ID: " << roomId
        << " | Check-in: " << checkIn
        << " | Check-out: " << checkOut
        << " | Status: " << status
        << std::endl;
}