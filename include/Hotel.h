#ifndef HOTEL_H
#define HOTEL_H

#include "Database.h"

class Hotel
{
private:
    Database database;

public:
    Hotel();

    bool initialize();

    void addSampleRooms();

    void showRooms();

    void addCustomer();

    void showCustomers();

    void makeReservation();

    void showReservations();

    void checkIn();

    void checkOut();

    void generateBill();

    void showReports();

    void menu();
};

#endif