#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <string>

class Customer
{
private:
    int id;
    std::string name;
    std::string phone;
    std::string email;

public:
    Customer();

    Customer(
        int id,
        const std::string& name,
        const std::string& phone,
        const std::string& email
    );

    int getId() const;
    std::string getName() const;
    std::string getPhone() const;
    std::string getEmail() const;

    void display() const;
};

#endif