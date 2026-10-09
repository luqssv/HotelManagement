#include "Customer.h"
#include <iostream>

Customer::Customer(){
    id = 0;
    name = "";
    phone = "";
    email = "";
}
Customer::Customer(int id, const std::string& name, const std::string& phone, const std::string& email){
    this->id = id;
    this->name = name;
    this->phone = phone;
    this->email = email;

}
int Customer::getId() const{
    return id;
}

std::string Customer::getName() const{
    return name;
}
std::string Customer::getPhone() const{
    return phone;
}

std::string Customer::getEmail() const{
    return email;
}

void Customer::display() const{
    std::cout << "ID: " << id << " | Name: " << name << " | Phone: " << phone << " | Email: "<< std::endl;
    
}