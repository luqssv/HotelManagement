#include "HotelService.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QVariant>
#include <cmath>

namespace
{
// Run one statement with bound parameters (prevents SQL injection).
bool run(const QString& sql, const QVariantList& args,
         QString* error = nullptr, int* affected = nullptr)
{
    QSqlQuery q;
    q.prepare(sql);
    for (const QVariant& a : args)
        q.addBindValue(a);

    if (!q.exec())
    {
        if (error) *error = q.lastError().text();
        return false;
    }
    if (affected) *affected = q.numRowsAffected();
    return true;
}

// Run a query and return the first column of the first row (invalid if none).
QVariant scalar(const QString& sql, const QVariantList& args = {})
{
    QSqlQuery q;
    q.prepare(sql);
    for (const QVariant& a : args)
        q.addBindValue(a);

    if (q.exec() && q.next())
        return q.value(0);
    return {};
}

// BEGIN ... COMMIT, with automatic ROLLBACK if anything fails.
template <typename Body>
bool inTransaction(Body body, QString& error)
{
    QSqlDatabase db = QSqlDatabase::database();

    if (!db.transaction())
    {
        error = db.lastError().text();
        return false;
    }
    if (body(error) && db.commit())
        return true;

    if (error.isEmpty())
        error = db.lastError().text();
    db.rollback();
    return false;
}
} // namespace

bool HotelService::open(const QString& path, QString& error)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);

    if (!db.open())
    {
        error = db.lastError().text();
        return false;
    }

    // Same tables as the console version (plus a small settings table).
    const QStringList schema = {
        R"(CREATE TABLE IF NOT EXISTS rooms (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            room_number INTEGER UNIQUE NOT NULL,
            type TEXT NOT NULL,
            price REAL NOT NULL,
            status TEXT NOT NULL))",
        R"(CREATE TABLE IF NOT EXISTS customers (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            phone TEXT,
            email TEXT))",
        R"(CREATE TABLE IF NOT EXISTS reservations (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            customer_id INTEGER NOT NULL,
            room_id INTEGER NOT NULL,
            check_in TEXT NOT NULL,
            check_out TEXT NOT NULL,
            status TEXT NOT NULL))",
        R"(CREATE TABLE IF NOT EXISTS bills (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            reservation_id INTEGER NOT NULL,
            room_total REAL NOT NULL,
            services_total REAL NOT NULL,
            tax REAL NOT NULL,
            total REAL NOT NULL))",
        R"(CREATE TABLE IF NOT EXISTS settings (
            key TEXT PRIMARY KEY,
            value TEXT NOT NULL))"
    };

    for (const QString& sql : schema)
    {
        if (!run(sql, {}, &error))
            return false;
    }
    return true;
}

// ---------- Settings ----------

double HotelService::taxPercent()
{
    const QVariant v = scalar("SELECT value FROM settings WHERE key = 'tax_percent'");

    bool ok = false;
    const double percent = v.toDouble(&ok);

    return (v.isValid() && ok) ? percent : DEFAULT_TAX_PERCENT;
}

bool HotelService::setTaxPercent(double percent, QString& error)
{
    if (!std::isfinite(percent) || percent < 0 || percent > 100)
    {
        error = "The VAT rate must be between 0 and 100.";
        return false;
    }
    return run("INSERT OR REPLACE INTO settings (key, value) VALUES ('tax_percent', ?)",
               {QString::number(percent, 'f', 2)}, &error);
}

// ---------- Queries for the tables ----------

QSqlQuery HotelService::roomsQuery()
{
    QSqlQuery q;
    q.exec("SELECT id, room_number, type, price, status "
           "FROM rooms ORDER BY room_number");
    return q;
}

QSqlQuery HotelService::customersQuery()
{
    QSqlQuery q;
    q.exec("SELECT id, name, phone, email FROM customers ORDER BY id");
    return q;
}

QSqlQuery HotelService::reservationsQuery()
{
    QSqlQuery q;
    q.exec("SELECT reservations.id, customers.name, rooms.room_number, "
           "       reservations.check_in, reservations.check_out, reservations.status "
           "FROM reservations "
           "JOIN customers ON customers.id = reservations.customer_id "
           "JOIN rooms ON rooms.id = reservations.room_id "
           "ORDER BY reservations.id");
    return q;
}

QSqlQuery HotelService::billsQuery()
{
    QSqlQuery q;
    q.exec("SELECT bills.id, bills.reservation_id, customers.name, rooms.room_number, "
           "       bills.room_total, bills.services_total, bills.tax, bills.total "
           "FROM bills "
           "JOIN reservations ON reservations.id = bills.reservation_id "
           "JOIN customers ON customers.id = reservations.customer_id "
           "JOIN rooms ON rooms.id = reservations.room_id "
           "ORDER BY bills.id");
    return q;
}

// ---------- Actions ----------

void HotelService::addSampleRooms()
{
    struct Sample { int number; const char* type; double price; };
    const Sample samples[] = {
        {101, "SINGLE", 60}, {102, "SINGLE", 60},
        {201, "DOUBLE", 90}, {202, "DOUBLE", 90},
        {301, "SUITE", 160}
    };

    for (const Sample& s : samples)
    {
        run("INSERT OR IGNORE INTO rooms (room_number, type, price, status) "
            "VALUES (?, ?, ?, 'AVAILABLE')",
            {s.number, QString(s.type), s.price});
    }
}

bool HotelService::addRoom(int number, const QString& type, double price, QString& error)
{
    if (scalar("SELECT COUNT(*) FROM rooms WHERE room_number = ?", {number}).toInt() > 0)
    {
        error = "A room with this number already exists.";
        return false;
    }
    if (price <= 0)
    {
        error = "Price must be greater than zero.";
        return false;
    }
    return run("INSERT INTO rooms (room_number, type, price, status) "
               "VALUES (?, ?, ?, 'AVAILABLE')",
               {number, type, price}, &error);
}

bool HotelService::addCustomer(const QString& name, const QString& phone,
                               const QString& email, QString& error)
{
    if (name.trimmed().isEmpty())
    {
        error = "The customer name cannot be empty.";
        return false;
    }
    return run("INSERT INTO customers (name, phone, email) VALUES (?, ?, ?)",
               {name.trimmed(), phone.trimmed(), email.trimmed()}, &error);
}

bool HotelService::makeReservation(int customerId, int roomId, const QDate& checkIn,
                                   const QDate& checkOut, QString& error)
{
    if (!checkIn.isValid() || !checkOut.isValid() || checkOut <= checkIn)
    {
        error = "Check-out must be after check-in.";
        return false;
    }
    if (scalar("SELECT COUNT(*) FROM customers WHERE id = ?", {customerId}).toInt() == 0)
    {
        error = "Customer not found.";
        return false;
    }

    const QVariant status = scalar("SELECT status FROM rooms WHERE id = ?", {roomId});
    if (!status.isValid())
    {
        error = "Room not found.";
        return false;
    }
    if (status.toString() != "AVAILABLE")
    {
        error = "This room is not available.";
        return false;
    }

    const QString in = checkIn.toString(Qt::ISODate);
    const QString out = checkOut.toString(Qt::ISODate);

    const int overlaps = scalar(
        "SELECT COUNT(*) FROM reservations "
        "WHERE room_id = ? AND status IN ('RESERVED', 'CHECKED_IN') "
        "AND check_in < ? AND check_out > ?",
        {roomId, out, in}).toInt();

    if (overlaps > 0)
    {
        error = "The room is already booked for those dates.";
        return false;
    }

    return inTransaction([&](QString& e)
    {
        if (!run("INSERT INTO reservations "
                 "(customer_id, room_id, check_in, check_out, status) "
                 "VALUES (?, ?, ?, ?, 'RESERVED')",
                 {customerId, roomId, in, out}, &e))
            return false;

        int changed = 0;
        if (!run("UPDATE rooms SET status = 'RESERVED' "
                 "WHERE id = ? AND status = 'AVAILABLE'",
                 {roomId}, &e, &changed))
            return false;

        if (changed != 1)
        {
            e = "Could not reserve the room.";
            return false;
        }
        return true;
    }, error);
}

bool HotelService::checkIn(int reservationId, QString& error)
{
    const QVariant room = scalar(
        "SELECT room_id FROM reservations WHERE id = ? AND status = 'RESERVED'",
        {reservationId});

    if (!room.isValid())
    {
        error = "Reservation not found or already checked in.";
        return false;
    }
    const int roomId = room.toInt();

    return inTransaction([&](QString& e)
    {
        int changed = 0;

        if (!run("UPDATE reservations SET status = 'CHECKED_IN' "
                 "WHERE id = ? AND status = 'RESERVED'",
                 {reservationId}, &e, &changed))
            return false;
        if (changed != 1)
        {
            e = "Check-in failed.";
            return false;
        }

        if (!run("UPDATE rooms SET status = 'OCCUPIED' "
                 "WHERE id = ? AND status = 'RESERVED'",
                 {roomId}, &e, &changed))
            return false;
        if (changed != 1)
        {
            e = "The room is not in RESERVED state.";
            return false;
        }
        return true;
    }, error);
}

bool HotelService::checkOut(int reservationId, QString& error)
{
    const QVariant room = scalar(
        "SELECT room_id FROM reservations WHERE id = ? AND status = 'CHECKED_IN'",
        {reservationId});

    if (!room.isValid())
    {
        error = "Active reservation not found. Check-in may not have been done.";
        return false;
    }
    const int roomId = room.toInt();

    if (scalar("SELECT COUNT(*) FROM bills WHERE reservation_id = ?",
               {reservationId}).toInt() == 0)
    {
        error = "Generate the bill before checking out.";
        return false;
    }

    return inTransaction([&](QString& e)
    {
        int changed = 0;

        if (!run("UPDATE reservations SET status = 'CHECKED_OUT' "
                 "WHERE id = ? AND status = 'CHECKED_IN'",
                 {reservationId}, &e, &changed))
            return false;
        if (changed != 1)
        {
            e = "Check-out failed.";
            return false;
        }

        // Free the room only if no other active reservation uses it.
        return run("UPDATE rooms SET status = 'AVAILABLE' "
                   "WHERE id = ? AND NOT EXISTS ("
                   "  SELECT 1 FROM reservations WHERE room_id = rooms.id "
                   "  AND status IN ('RESERVED', 'CHECKED_IN'))",
                   {roomId}, &e);
    }, error);
}

bool HotelService::generateBill(int reservationId, double services,
                                BillResult& bill, QString& error)
{
    if (!std::isfinite(services) || services < 0)
    {
        error = "The services amount must be zero or more.";
        return false;
    }

    QSqlQuery q;
    q.prepare("SELECT reservations.check_in, reservations.check_out, "
              "       reservations.status, rooms.price "
              "FROM reservations JOIN rooms ON rooms.id = reservations.room_id "
              "WHERE reservations.id = ?");
    q.addBindValue(reservationId);

    if (!q.exec() || !q.next())
    {
        error = "Reservation not found.";
        return false;
    }

    const QDate in = QDate::fromString(q.value(0).toString(), Qt::ISODate);
    const QDate out = QDate::fromString(q.value(1).toString(), Qt::ISODate);
    const QString status = q.value(2).toString();
    const double price = q.value(3).toDouble();
    q.finish();

    if (status != "CHECKED_IN" && status != "CHECKED_OUT")
    {
        error = "The reservation must be checked in first.";
        return false;
    }
    if (scalar("SELECT COUNT(*) FROM bills WHERE reservation_id = ?",
               {reservationId}).toInt() > 0)
    {
        error = "A bill already exists for this reservation.";
        return false;
    }

    const int nights = static_cast<int>(in.daysTo(out));
    if (nights <= 0)
    {
        error = "Invalid reservation dates.";
        return false;
    }

    const double percent = taxPercent();

    bill.nights = nights;
    bill.pricePerNight = price;
    bill.roomTotal = price * nights;
    bill.services = services;
    bill.subtotal = bill.roomTotal + services;
    bill.taxPercent = percent;
    bill.tax = std::round(bill.subtotal * percent) / 100.0;   // rounded to cents
    bill.total = bill.subtotal + bill.tax;

    return run("INSERT INTO bills "
               "(reservation_id, room_total, services_total, tax, total) "
               "VALUES (?, ?, ?, ?, ?)",
               {reservationId, bill.roomTotal, bill.services, bill.tax, bill.total},
               &error);
}

// ---------- Deleting ----------

bool HotelService::deleteCustomer(int customerId, QString& error)
{
    if (scalar("SELECT COUNT(*) FROM reservations WHERE customer_id = ?",
               {customerId}).toInt() > 0)
    {
        error = "This customer still has reservations. Delete those first.";
        return false;
    }

    int changed = 0;
    if (!run("DELETE FROM customers WHERE id = ?", {customerId}, &error, &changed))
        return false;

    if (changed != 1)
    {
        error = "Customer not found.";
        return false;
    }
    return true;
}

bool HotelService::deleteRoom(int roomId, QString& error)
{
    if (scalar("SELECT COUNT(*) FROM reservations WHERE room_id = ?",
               {roomId}).toInt() > 0)
    {
        error = "This room still has reservations. Delete those first.";
        return false;
    }

    int changed = 0;
    if (!run("DELETE FROM rooms WHERE id = ?", {roomId}, &error, &changed))
        return false;

    if (changed != 1)
    {
        error = "Room not found.";
        return false;
    }
    return true;
}

bool HotelService::deleteReservation(int reservationId, QString& error)
{
    QSqlQuery q;
    q.prepare("SELECT room_id, status FROM reservations WHERE id = ?");
    q.addBindValue(reservationId);

    if (!q.exec() || !q.next())
    {
        error = "Reservation not found.";
        return false;
    }

    const int roomId = q.value(0).toInt();
    const QString status = q.value(1).toString();
    q.finish();

    if (status == "CHECKED_IN")
    {
        error = "This guest is checked in. Generate the bill and check out first.";
        return false;
    }

    return inTransaction([&](QString& e)
    {
        if (!run("DELETE FROM bills WHERE reservation_id = ?", {reservationId}, &e))
            return false;

        if (!run("DELETE FROM reservations WHERE id = ?", {reservationId}, &e))
            return false;

        // A reservation that was only holding the room: make it available again.
        return run("UPDATE rooms SET status = 'AVAILABLE' "
                   "WHERE id = ? AND status = 'RESERVED' "
                   "AND NOT EXISTS ("
                   "  SELECT 1 FROM reservations WHERE room_id = rooms.id "
                   "  AND status IN ('RESERVED', 'CHECKED_IN'))",
                   {roomId}, &e);
    }, error);
}

Stats HotelService::stats()
{
    Stats s;
    s.totalRooms   = scalar("SELECT COUNT(*) FROM rooms").toInt();
    s.available    = scalar("SELECT COUNT(*) FROM rooms WHERE status = 'AVAILABLE'").toInt();
    s.occupied     = scalar("SELECT COUNT(*) FROM rooms WHERE status = 'OCCUPIED'").toInt();
    s.reserved     = scalar("SELECT COUNT(*) FROM rooms WHERE status = 'RESERVED'").toInt();
    s.maintenance  = scalar("SELECT COUNT(*) FROM rooms WHERE status = 'MAINTENANCE'").toInt();
    s.customers    = scalar("SELECT COUNT(*) FROM customers").toInt();
    s.reservations = scalar("SELECT COUNT(*) FROM reservations").toInt();
    s.active       = scalar("SELECT COUNT(*) FROM reservations "
                            "WHERE status IN ('RESERVED', 'CHECKED_IN')").toInt();
    s.completed    = scalar("SELECT COUNT(*) FROM reservations "
                            "WHERE status = 'CHECKED_OUT'").toInt();
    s.bills        = scalar("SELECT COUNT(*) FROM bills").toInt();
    s.revenue      = scalar("SELECT COALESCE(SUM(total), 0) FROM bills").toDouble();
    s.taxPercent   = taxPercent();
    return s;
}
