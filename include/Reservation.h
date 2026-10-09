#ifndef RESERVATION_H
#define RESERVATION_H

#include <string>

class Reservation
{
private:
    int id;
    int customerId;
    int roomId;

    std::string checkIn;
    std::string checkOut;
    std::string status;

public:
    Reservation();

    Reservation(
        int id,
        int customerId,
        int roomId,
        const std::string& checkIn,
        const std::string& checkOut,
        const std::string& status
    );

    int getId() const;
    int getCustomerId() const;
    int getRoomId() const;

    std::string getCheckIn() const;
    std::string getCheckOut() const;
    std::string getStatus() const;

    void setStatus(const std::string& status);

    void display() const;
};

#endif