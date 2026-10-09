#pragma once

#include <QDate>
#include <QSqlQuery>
#include <QString>

struct Stats
{
    int totalRooms = 0, available = 0, occupied = 0, reserved = 0, maintenance = 0;
    int customers = 0;
    int reservations = 0, active = 0, completed = 0;
    int bills = 0;
    double revenue = 0;
};

struct BillResult
{
    int nights = 0;
    double pricePerNight = 0;
    double roomTotal = 0;
    double services = 0;
    double subtotal = 0;
    double tax = 0;
    double total = 0;
};

// All database logic for the GUI. Uses Qt's default SQL connection.
class HotelService
{
public:
    static constexpr double TAX_RATE = 0.10;

    bool open(const QString& path, QString& error);

    // Data for the tables
    QSqlQuery roomsQuery();         // id, number, type, price, status
    QSqlQuery customersQuery();     // id, name, phone, email
    QSqlQuery reservationsQuery();  // id, customer, room, check-in, check-out, status
    QSqlQuery billsQuery();         // id, reservation, customer, room, room, services, tax, total

    // Actions (return false and fill 'error' on failure)
    void addSampleRooms();
    bool addRoom(int number, const QString& type, double price, QString& error);
    bool addCustomer(const QString& name, const QString& phone,
                     const QString& email, QString& error);
    bool makeReservation(int customerId, int roomId, const QDate& checkIn,
                         const QDate& checkOut, QString& error);
    bool checkIn(int reservationId, QString& error);
    bool checkOut(int reservationId, QString& error);
    bool generateBill(int reservationId, double services,
                      BillResult& bill, QString& error);

    Stats stats();
};