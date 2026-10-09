#include "Database.h"
#include <iostream>

Database::Database()
{
    db = nullptr;
}

Database::~Database()
{
    close();
}

bool Database::open(const std::string& filename)
{
    int result = sqlite3_open(filename.c_str(), &db);

    if (result != SQLITE_OK)
    {
        std::cerr
            << "Error opening database: "
            << sqlite3_errmsg(db)
            << std::endl;

        return false;
    }

    std::cout << "Database connected successfully.\n";

    return true;
}

void Database::close()
{
    if (db != nullptr)
    {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool Database::execute(const std::string& sql)
{
    char* errorMessage = nullptr;

    int result = sqlite3_exec(
        db,
        sql.c_str(),
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK)
    {
        std::cerr
            << "SQL Error: "
            << errorMessage
            << std::endl;

        sqlite3_free(errorMessage);

        return false;
    }

    return true;
}

sqlite3* Database::getConnection()
{
    return db;
}