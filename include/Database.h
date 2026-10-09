#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>

class Database
{
private:
    sqlite3* db;

public:
    Database();
    ~Database();

    bool open(const std::string& filename);
    void close();

    bool execute(const std::string& sql);

    sqlite3* getConnection();
};

#endif