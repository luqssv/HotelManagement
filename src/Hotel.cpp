#include "Hotel.h"
#include <iostream>
#include <string>
#include <limits>
#include <cmath>
#include <regex>
#include <ctime>
#include <cstdio>
#include <iomanip>
#include <sqlite3.h>

Hotel::Hotel(){}
bool Hotel::initialize(){
    if(!database.open("hotel.db")){
        return false;
    }
    database.execute(R"(
        CREATE TABLE IF NOT EXISTS rooms (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            room_number INTEGER UNIQUE NOT NULL,
            type TEXT NOT NULL,
            price REAL NOT NULL,
            status TEXT NOT NULL
        );
    )");
    database.execute(R"(
        CREATE TABLE IF NOT EXISTS customers (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            phone TEXT,
            email TEXT
        );
    )");
    database.execute(R"(
        CREATE TABLE IF NOT EXISTS reservations (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            customer_id INTEGER NOT NULL,
            room_id INTEGER NOT NULL,
            check_in TEXT NOT NULL,
            check_out TEXT NOT NULL,
            status TEXT NOT NULL
        );
    )");
    database.execute(R"(
        CREATE TABLE IF NOT EXISTS bills (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            reservation_id INTEGER NOT NULL,
            room_total REAL NOT NULL,
            services_total REAL NOT NULL,
            tax REAL NOT NULL,
            total REAL NOT NULL
        );
    )");

    return true;
}

void Hotel::addSampleRooms(){
    database.execute(R"(
        INSERT OR IGNORE INTO rooms
        (room_number, type, price, status)
        VALUES
        (101, 'SINGLE', 60, 'AVAILABLE');
    )");
    database.execute(R"(
        INSERT OR IGNORE INTO rooms
        (room_number, type, price, status)
        VALUES
        (102, 'SINGLE', 60, 'AVAILABLE');
    )");
    database.execute(R"(
        INSERT OR IGNORE INTO rooms
        (room_number, type, price, status)
        VALUES
        (201, 'DOUBLE', 90, 'AVAILABLE');
    )");

    database.execute(R"(
        INSERT OR IGNORE INTO rooms
        (room_number, type, price, status)
        VALUES
        (202, 'DOUBLE', 90, 'AVAILABLE');
    )");

    database.execute(R"(
        INSERT OR IGNORE INTO rooms
        (room_number, type, price, status)
        VALUES
        (301, 'SUITE', 160, 'AVAILABLE');
    )");

    std::cout << "Sample rooms added.\n";
}
void Hotel::showRooms()
{
    const char* sql =
        "SELECT id, room_number, type, price, status "
        "FROM rooms;";

    sqlite3_stmt* statement;

    if (sqlite3_prepare_v2(
            database.getConnection(),
            sql,
            -1,
            &statement,
            nullptr
        ) != SQLITE_OK)
    {
        std::cerr << "Failed to read rooms.\n";
        return;
    }

    std::cout << "\n========== ROOMS ==========\n";

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        int id =
            sqlite3_column_int(statement, 0);

        int roomNumber =
            sqlite3_column_int(statement, 1);

        const char* type =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 2)
            );

        double price =
            sqlite3_column_double(statement, 3);

        const char* status =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 4)
            );

        std::cout
            << "ID: " << id
            << " | Room: " << roomNumber
            << " | Type: " << type
            << " | Price: " << price << " EUR"
            << " | Status: " << status
            << '\n';
    }

    sqlite3_finalize(statement);

    std::cout << "===========================\n";
}
void Hotel::addCustomer()
{
    std::string name;
    std::string phone;
    std::string email;

    std::cin.ignore();

    std::cout << "\nCustomer name: ";
    std::getline(std::cin, name);

    std::cout << "Phone: ";
    std::getline(std::cin, phone);

    std::cout << "Email: ";
    std::getline(std::cin, email);

    std::string sql =
        "INSERT INTO customers "
        "(name, phone, email) VALUES ('" +
        name + "', '" +
        phone + "', '" +
        email + "');";

    if (database.execute(sql))
    {
        std::cout << "Customer added successfully.\n";
    }
}
void Hotel::showCustomers()
{
    const char* sql =
        "SELECT id, name, phone, email "
        "FROM customers;";

    sqlite3_stmt* statement;

    if (sqlite3_prepare_v2(
            database.getConnection(),
            sql,
            -1,
            &statement,
            nullptr
        ) != SQLITE_OK)
    {
        std::cerr << "Failed to read customers.\n";
        return;
    }

    std::cout << "\n======= CUSTOMERS =======\n";

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        int id =
            sqlite3_column_int(statement, 0);

        const char* name =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 1)
            );

        const char* phone =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 2)
            );

        const char* email =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 3)
            );

        std::cout
            << id << " | "
            << name << " | "
            << phone << " | "
            << email << '\n';
    }

    sqlite3_finalize(statement);

    std::cout << "=========================\n";
}
// Check that a date is a real calendar date in YYYY-MM-DD format.
bool isValidDate(const std::string& date)
{
    if (!std::regex_match(
            date,
            std::regex(R"(\d{4}-\d{2}-\d{2})")))
    {
        return false;
    }

    int year = std::stoi(date.substr(0, 4));
    int month = std::stoi(date.substr(5, 2));
    int day = std::stoi(date.substr(8, 2));

    std::tm time = {};
    time.tm_year = year - 1900;
    time.tm_mon = month - 1;
    time.tm_mday = day;
    time.tm_isdst = -1;

    std::mktime(&time);

    char normalized[11];
    std::strftime(
        normalized,
        sizeof(normalized),
        "%Y-%m-%d",
        &time
    );

    return date == normalized;
}

int readInteger(const std::string& prompt)
{
    int value;

    while (true)
    {
        std::cout << prompt;

        if (std::cin >> value)
        {
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(),
                '\n'
            );

            return value;
        }

        std::cout << "Please enter a valid number.\n";
        std::cin.clear();

        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(),
            '\n'
        );
    }
}

std::string readText(const std::string& prompt)
{
    std::string value;

    std::cout << prompt;
    std::getline(std::cin, value);

    return value;
}

void Hotel::makeReservation()
{
    sqlite3* db = database.getConnection();

    std::cout << "\n===== MAKE RESERVATION =====\n";

    // 1. Display customers
    showCustomers();

    int customerId = readInteger(
        "Enter customer ID: "
    );

    // 2. Verify the customer exists
    sqlite3_stmt* statement = nullptr;

    const char* customerSQL =
        "SELECT id FROM customers WHERE id = ?;";

    if (sqlite3_prepare_v2(
            db, customerSQL, -1,
            &statement, nullptr) != SQLITE_OK)
    {
        std::cerr << "Could not query customers.\n";
        return;
    }

    sqlite3_bind_int(statement, 1, customerId);

    bool customerExists =
        sqlite3_step(statement) == SQLITE_ROW;

    sqlite3_finalize(statement);

    if (!customerExists)
    {
        std::cout << "Customer not found.\n";
        return;
    }

    // 3. Display rooms
    showRooms();

    int roomNumber = readInteger(
        "Enter room number: "
    );

    // 4. Find the room and check its status
    const char* roomSQL =
        "SELECT id, status FROM rooms "
        "WHERE room_number = ?;";

    if (sqlite3_prepare_v2(
            db, roomSQL, -1,
            &statement, nullptr) != SQLITE_OK)
    {
        std::cerr << "Could not query rooms.\n";
        return;
    }

    sqlite3_bind_int(statement, 1, roomNumber);

    if (sqlite3_step(statement) != SQLITE_ROW)
    {
        sqlite3_finalize(statement);
        std::cout << "Room not found.\n";
        return;
    }

    int roomId = sqlite3_column_int(statement, 0);

    const unsigned char* statusText =
        sqlite3_column_text(statement, 1);

    std::string status =
        statusText
            ? reinterpret_cast<const char*>(statusText)
            : "";

    sqlite3_finalize(statement);

    if (status != "AVAILABLE")
    {
        std::cout << "This room is not available.\n";
        return;
    }

    // 5. Ask for the dates
    std::string checkIn = readText(
        "Check-in date (YYYY-MM-DD): "
    );

    std::string checkOut = readText(
        "Check-out date (YYYY-MM-DD): "
    );

    if (!isValidDate(checkIn) ||
        !isValidDate(checkOut) ||
        checkOut <= checkIn)
    {
        std::cout
            << "Invalid dates. Check-out must be "
            << "after check-in.\n";
        return;
    }

    // 6. Check for overlapping reservations
    const char* overlapSQL = R"(
        SELECT COUNT(*)
        FROM reservations
        WHERE room_id = ?
          AND status IN ('RESERVED', 'CHECKED_IN')
          AND check_in < ?
          AND check_out > ?;
    )";

    if (sqlite3_prepare_v2(
            db, overlapSQL, -1,
            &statement, nullptr) != SQLITE_OK)
    {
        std::cerr << "Could not check reservations.\n";
        return;
    }

    sqlite3_bind_int(statement, 1, roomId);
    sqlite3_bind_text(
        statement, 2, checkOut.c_str(),
        -1, SQLITE_TRANSIENT
    );
    sqlite3_bind_text(
        statement, 3, checkIn.c_str(),
        -1, SQLITE_TRANSIENT
    );

    bool overlapping =
        sqlite3_step(statement) == SQLITE_ROW &&
        sqlite3_column_int(statement, 0) > 0;

    sqlite3_finalize(statement);

    if (overlapping)
    {
        std::cout
            << "The room is already booked "
            << "for those dates.\n";
        return;
    }

    // 7. Save the reservation and update the room
    if (!database.execute("BEGIN IMMEDIATE;"))
    {
        std::cerr << "Could not start transaction.\n";
        return;
    }

    const char* insertSQL = R"(
        INSERT INTO reservations
        (customer_id, room_id, check_in, check_out, status)
        VALUES (?, ?, ?, ?, 'RESERVED');
    )";

    bool success = false;

    if (sqlite3_prepare_v2(
            db, insertSQL, -1,
            &statement, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, customerId);
        sqlite3_bind_int(statement, 2, roomId);

        sqlite3_bind_text(
            statement, 3, checkIn.c_str(),
            -1, SQLITE_TRANSIENT
        );

        sqlite3_bind_text(
            statement, 4, checkOut.c_str(),
            -1, SQLITE_TRANSIENT
        );

        success =
            sqlite3_step(statement) == SQLITE_DONE;

        sqlite3_finalize(statement);
    }

    if (success)
    {
        const char* updateSQL = R"(
            UPDATE rooms
            SET status = 'RESERVED'
            WHERE id = ? AND status = 'AVAILABLE';
        )";

        if (sqlite3_prepare_v2(
                db, updateSQL, -1,
                &statement, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_int(statement, 1, roomId);

            success =
                sqlite3_step(statement) == SQLITE_DONE &&
                sqlite3_changes(db) == 1;

            sqlite3_finalize(statement);
        }
        else
        {
            success = false;
        }
    }

    if (success)
    {
        success = database.execute("COMMIT;");
    }

    if (!success)
    {
        database.execute("ROLLBACK;");
        std::cout
            << "Reservation failed. No changes saved.\n";
        return;
    }

    std::cout << "\nReservation created successfully!\n";
    std::cout << "Customer ID: " << customerId << '\n';
    std::cout << "Room number: " << roomNumber << '\n';
    std::cout << "Check-in:    " << checkIn << '\n';
    std::cout << "Check-out:   " << checkOut << '\n';
    std::cout << "Status:      RESERVED\n";
}

void Hotel::showReservations()
{
    sqlite3* db = database.getConnection();
    sqlite3_stmt* statement = nullptr;

    const char* sql = R"(
        SELECT
            reservations.id,
            customers.name,
            rooms.room_number,
            reservations.check_in,
            reservations.check_out,
            reservations.status
        FROM reservations
        JOIN customers
            ON customers.id = reservations.customer_id
        JOIN rooms
            ON rooms.id = reservations.room_id
        ORDER BY reservations.id;
    )";

    if (sqlite3_prepare_v2(
            db, sql, -1,
            &statement, nullptr) != SQLITE_OK)
    {
        std::cerr << "Could not load reservations.\n";
        return;
    }

    std::cout << "\n========== RESERVATIONS ==========\n";

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(statement, 0);

        auto getText = [&](int column) -> std::string
        {
            const unsigned char* value =
                sqlite3_column_text(statement, column);

            return value
                ? reinterpret_cast<const char*>(value)
                : "";
        };

        std::cout
            << "Reservation ID: " << id
            << " | Customer: " << getText(1)
            << " | Room: " << sqlite3_column_int(statement, 2)
            << "\n  Check-in: " << getText(3)
            << " | Check-out: " << getText(4)
            << " | Status: " << getText(5)
            << "\n----------------------------------\n";
    }

    sqlite3_finalize(statement);
}
// Days between two dates (YYYY-MM-DD). Works on Windows and Linux.
static long daysFromCivil(int y, unsigned m, unsigned d)
{
    y -= m <= 2;
    const long era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<long>(doe) - 719468;
}

int calculateNights(const std::string& checkIn, const std::string& checkOut)
{
    int y1, m1, d1, y2, m2, d2;
    char extra1, extra2;

    if (std::sscanf(checkIn.c_str(), "%4d-%2d-%2d%c", &y1, &m1, &d1, &extra1) != 3 ||
        std::sscanf(checkOut.c_str(), "%4d-%2d-%2d%c", &y2, &m2, &d2, &extra2) != 3)
    {
        return -1;
    }

    return static_cast<int>(daysFromCivil(y2, m2, d2) - daysFromCivil(y1, m1, d1));
}
void Hotel::checkIn()
{
    sqlite3* db = database.getConnection();

    std::cout << "\n========== CHECK-IN ==========\n";

    int reservationId;

    std::cout << "Reservation ID: ";

    if (!(std::cin >> reservationId))
    {
        std::cin.clear();
        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n'
        );
        std::cout << "Invalid reservation ID.\n";
        return;
    }

    sqlite3_stmt* stmt = nullptr;

    const char* sql = R"(
        SELECT room_id
        FROM reservations
        WHERE id = ? AND status = 'RESERVED';
    )";

    if (sqlite3_prepare_v2(
            db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Database error.\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, reservationId);

    if (sqlite3_step(stmt) != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        std::cout
            << "Reservation not found or already checked in.\n";
        return;
    }

    int roomId = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    if (!database.execute("BEGIN IMMEDIATE;"))
    {
        std::cerr << "Could not start transaction.\n";
        return;
    }

    const char* updateReservation = R"(
        UPDATE reservations
        SET status = 'CHECKED_IN'
        WHERE id = ? AND status = 'RESERVED';
    )";

    bool success = false;

    if (sqlite3_prepare_v2(
            db, updateReservation, -1,
            &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, reservationId);

        success =
            sqlite3_step(stmt) == SQLITE_DONE &&
            sqlite3_changes(db) == 1;

        sqlite3_finalize(stmt);
    }

    if (success)
    {
        const char* updateRoom = R"(
            UPDATE rooms
            SET status = 'OCCUPIED'
            WHERE id = ? AND status = 'RESERVED';
        )";

        if (sqlite3_prepare_v2(
                db, updateRoom, -1,
                &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_int(stmt, 1, roomId);

            success =
                sqlite3_step(stmt) == SQLITE_DONE &&
                sqlite3_changes(db) == 1;

            sqlite3_finalize(stmt);
        }
        else
        {
            success = false;
        }
    }

    if (success)
    {
        success = database.execute("COMMIT;");
    }

    if (!success)
    {
        database.execute("ROLLBACK;");
        std::cout << "Check-in failed.\n";
        return;
    }

    std::cout
        << "Check-in successful! Room is now OCCUPIED.\n";
}

void Hotel::checkOut()
{
    sqlite3* db = database.getConnection();
    sqlite3_stmt* stmt = nullptr;

    std::cout << "\n========== CHECK-OUT ==========\n";

    int reservationId;

    std::cout << "Reservation ID: ";

    if (!(std::cin >> reservationId))
    {
        std::cin.clear();
        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n'
        );
        std::cout << "Invalid reservation ID.\n";
        return;
    }

    // Find an active stay.
    const char* findSQL = R"(
        SELECT room_id
        FROM reservations
        WHERE id = ? AND status = 'CHECKED_IN';
    )";

    if (sqlite3_prepare_v2(
            db, findSQL, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Database error.\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, reservationId);

    if (sqlite3_step(stmt) != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        std::cout
            << "Active reservation not found. "
            << "Check-in may not have been completed.\n";
        return;
    }

    int roomId = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    // Ensure the bill has been saved.
    const char* billSQL =
        "SELECT id FROM bills WHERE reservation_id = ?;";

    if (sqlite3_prepare_v2(
            db, billSQL, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Could not check bill.\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, reservationId);

    bool hasBill = sqlite3_step(stmt) == SQLITE_ROW;

    sqlite3_finalize(stmt);

    if (!hasBill)
    {
        std::cout
            << "Generate the bill before checking out.\n";
        return;
    }

    if (!database.execute("BEGIN IMMEDIATE;"))
    {
        std::cerr << "Could not start transaction.\n";
        return;
    }

    bool success = false;

    const char* reservationSQL = R"(
        UPDATE reservations
        SET status = 'CHECKED_OUT'
        WHERE id = ? AND status = 'CHECKED_IN';
    )";

    if (sqlite3_prepare_v2(
            db, reservationSQL, -1,
            &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, reservationId);

        success =
            sqlite3_step(stmt) == SQLITE_DONE &&
            sqlite3_changes(db) == 1;

        sqlite3_finalize(stmt);
    }

    if (success)
    {
        // Release the room only if no other active
        // reservation is using it.
        const char* roomSQL = R"(
            UPDATE rooms
            SET status = 'AVAILABLE'
            WHERE id = ?
              AND NOT EXISTS (
                  SELECT 1
                  FROM reservations
                  WHERE room_id = rooms.id
                    AND status IN ('RESERVED', 'CHECKED_IN')
              );
        )";

        if (sqlite3_prepare_v2(
                db, roomSQL, -1,
                &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_int(stmt, 1, roomId);

            success =
                sqlite3_step(stmt) == SQLITE_DONE &&
                sqlite3_changes(db) == 1;

            sqlite3_finalize(stmt);
        }
        else
        {
            success = false;
        }
    }

    if (success)
    {
        success = database.execute("COMMIT;");
    }

    if (!success)
    {
        database.execute("ROLLBACK;");
        std::cout << "Check-out failed.\n";
        return;
    }

    std::cout
        << "Check-out successful! Thank you for staying with us.\n";
    std::cout << "Room status updated when available.\n";
}

void Hotel::generateBill()
{
    sqlite3* db = database.getConnection();
    sqlite3_stmt* stmt = nullptr;

    std::cout << "\n========== GENERATE BILL ==========\n";

    int reservationId;

    std::cout << "Reservation ID: ";

    if (!(std::cin >> reservationId))
    {
        std::cin.clear();
        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n'
        );
        std::cout << "Invalid reservation ID.\n";
        return;
    }

    // Find reservation dates, room price and status.
    const char* sql = R"(
        SELECT
            reservations.check_in,
            reservations.check_out,
            reservations.status,
            rooms.price
        FROM reservations
        JOIN rooms ON rooms.id = reservations.room_id
        WHERE reservations.id = ?;
    )";

    if (sqlite3_prepare_v2(
            db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Database error.\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, reservationId);

    if (sqlite3_step(stmt) != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        std::cout << "Reservation not found.\n";
        return;
    }

    auto getText = [&](int column) -> std::string
    {
        const unsigned char* value =
            sqlite3_column_text(stmt, column);

        return value
            ? reinterpret_cast<const char*>(value)
            : "";
    };

    std::string checkIn = getText(0);
    std::string checkOut = getText(1);
    std::string status = getText(2);

    double pricePerNight = sqlite3_column_double(stmt, 3);

    sqlite3_finalize(stmt);

    if (status != "CHECKED_IN" &&
        status != "CHECKED_OUT")
    {
        std::cout
            << "The reservation must be checked in first.\n";
        return;
    }

    // Prevent duplicate bills.
    const char* existingSQL =
        "SELECT id FROM bills WHERE reservation_id = ?;";

    if (sqlite3_prepare_v2(
            db, existingSQL, -1,
            &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Could not check existing bills.\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, reservationId);

    bool alreadyBilled =
        sqlite3_step(stmt) == SQLITE_ROW;

    sqlite3_finalize(stmt);

    if (alreadyBilled)
    {
        std::cout << "A bill already exists for this reservation.\n";
        return;
    }

    // Calculate room cost.
    int nights = calculateNights(checkIn, checkOut);

    if (nights <= 0)
    {
        std::cout << "Invalid reservation dates.\n";
        return;
    }

    double roomTotal = pricePerNight * nights;

    // Ask for services.
    double servicesTotal;

    std::cout << "Number of nights: " << nights << '\n';
    std::cout << "Price per night: " << pricePerNight << " EUR\n";
    std::cout << "Room total: " << roomTotal << " EUR\n";
    std::cout << "Extra services total (EUR): ";

    if (!(std::cin >> servicesTotal) ||
        !std::isfinite(servicesTotal) ||
        servicesTotal < 0)
    {
        std::cin.clear();
        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n'
        );
        std::cout << "Invalid services amount.\n";
        return;
    }

        // VAT rate (percent) is stored in the settings table (the GUI can change it).
    // Falls back to 10% if no value is stored.
    double taxPercent = 10.0;
    {
        sqlite3_stmt* taxStmt = nullptr;

        if (sqlite3_prepare_v2(
                db,
                "SELECT value FROM settings WHERE key = 'tax_percent';",
                -1, &taxStmt, nullptr) == SQLITE_OK)
        {
            if (sqlite3_step(taxStmt) == SQLITE_ROW)
            {
                taxPercent = sqlite3_column_double(taxStmt, 0);
            }
            sqlite3_finalize(taxStmt);
        }
    }

    double subtotal = roomTotal + servicesTotal;
    double tax = subtotal * (taxPercent / 100.0);
    double total = subtotal + tax;

    // Save the bill using a prepared statement.
    const char* insertSQL = R"(
        INSERT INTO bills
        (reservation_id, room_total, services_total, tax, total)
        VALUES (?, ?, ?, ?, ?);
    )";

    if (sqlite3_prepare_v2(
            db, insertSQL, -1,
            &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Could not prepare bill.\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, reservationId);
    sqlite3_bind_double(stmt, 2, roomTotal);
    sqlite3_bind_double(stmt, 3, servicesTotal);
    sqlite3_bind_double(stmt, 4, tax);
    sqlite3_bind_double(stmt, 5, total);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;

    sqlite3_finalize(stmt);

    if (!success)
    {
        std::cerr << "Could not save bill.\n";
        return;
    }

    // Print receipt.
    std::cout << std::fixed << std::setprecision(2);

    std::cout << "\n========== HOTEL BILL ==========\n";
    std::cout << "Reservation: " << reservationId << '\n';
    std::cout << "Nights: " << nights << '\n';
    std::cout << "Room: " << roomTotal << " EUR\n";
    std::cout << "Services: " << servicesTotal << " EUR\n";
    std::cout << "Subtotal: " << subtotal << " EUR\n";
    std::cout << "VAT (" << taxPercent << "%): " << tax << " EUR\n";
    std::cout << "--------------------------------\n";
    std::cout << "TOTAL: " << total << " EUR\n";
    std::cout << "================================\n";
    std::cout << "Bill saved successfully.\n";
}

void Hotel::showReports()
{
    sqlite3* db = database.getConnection();

    std::cout << "\n========== HOTEL REPORTS ==========\n";

    // Helper to execute a query that returns one number.
    auto getCount = [&](const char* sql) -> double
    {
        sqlite3_stmt* stmt = nullptr;
        double result = 0;

        if (sqlite3_prepare_v2(
                db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "Report query failed.\n";
            return 0;
        }

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            result = sqlite3_column_double(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return result;
    };

    double totalRooms = getCount(
        "SELECT COUNT(*) FROM rooms;"
    );

    double availableRooms = getCount(
        "SELECT COUNT(*) FROM rooms "
        "WHERE status = 'AVAILABLE';"
    );

    double occupiedRooms = getCount(
        "SELECT COUNT(*) FROM rooms "
        "WHERE status = 'OCCUPIED';"
    );

    double reservedRooms = getCount(
        "SELECT COUNT(*) FROM rooms "
        "WHERE status = 'RESERVED';"
    );

    double maintenanceRooms = getCount(
        "SELECT COUNT(*) FROM rooms "
        "WHERE status = 'MAINTENANCE';"
    );

    double totalCustomers = getCount(
        "SELECT COUNT(*) FROM customers;"
    );

    double totalReservations = getCount(
        "SELECT COUNT(*) FROM reservations;"
    );

    double activeReservations = getCount(
        "SELECT COUNT(*) FROM reservations "
        "WHERE status IN ('RESERVED', 'CHECKED_IN');"
    );

    double completedReservations = getCount(
        "SELECT COUNT(*) FROM reservations "
        "WHERE status = 'CHECKED_OUT';"
    );

    double totalBills = getCount(
        "SELECT COUNT(*) FROM bills;"
    );

    double totalRevenue = getCount(
        "SELECT COALESCE(SUM(total), 0) FROM bills;"
    );

    std::cout << std::fixed << std::setprecision(2);

    std::cout << "\n--- ROOM STATISTICS ---\n";
    std::cout << "Total rooms:       " << totalRooms << '\n';
    std::cout << "Available rooms:   " << availableRooms << '\n';
    std::cout << "Occupied rooms:    " << occupiedRooms << '\n';
    std::cout << "Reserved rooms:    " << reservedRooms << '\n';
    std::cout << "Under maintenance: " << maintenanceRooms << '\n';

    std::cout << "\n--- CUSTOMER STATISTICS ---\n";
    std::cout << "Total customers:   " << totalCustomers << '\n';

    std::cout << "\n--- RESERVATIONS ---\n";
    std::cout << "Total reservations:     "
              << totalReservations << '\n';

    std::cout << "Active reservations:    "
              << activeReservations << '\n';

    std::cout << "Completed reservations: "
              << completedReservations << '\n';

    std::cout << "\n--- FINANCIAL REPORT ---\n";
    std::cout << "Bills generated: " << totalBills << '\n';

    std::cout << "Recorded revenue: "
              << totalRevenue << " EUR\n";

    std::cout << "===================================\n";
}
void Hotel::menu()
{
    int choice;

    do
    {
        std::cout << "\n";
        std::cout << "=================================\n";
        std::cout << "     HOTEL MANAGEMENT SYSTEM\n";
        std::cout << "=================================\n";

        std::cout << "1. Add sample rooms\n";
        std::cout << "2. Show rooms\n";
        std::cout << "3. Add customer\n";
        std::cout << "4. Show customers\n";
        std::cout << "5. Make reservation\n";
        std::cout << "6. Show reservations\n";
        std::cout << "7. Check in\n";
        std::cout << "8. Check out\n";
        std::cout << "9. Generate bill\n";
        std::cout << "10. Reports\n";
        std::cout << "0. Exit\n";

        std::cout << "\nChoice: ";
        std::cin >> choice;

        switch (choice)
        {
            case 1:
                addSampleRooms();
                break;

            case 2:
                showRooms();
                break;

            case 3:
                addCustomer();
                break;

            case 4:
                showCustomers();
                break;

            case 5:
                makeReservation();
                break;

            case 6:
                showReservations();
                break;

            case 7:
                checkIn();
                break;

            case 8:
                checkOut();
                break;

            case 9:
                generateBill();
                break;

            case 10:
                showReports();
                break;

            case 0:
                std::cout << "Goodbye!\n";
                break;

            default:
                std::cout << "Invalid choice.\n";
        }

    } while (choice != 0);
}