#include "Bill.h"
#include <iostream>

Bill::Bill(){
    roomTotal = 0;
    servicesTotal = 0;
    tax = 0;
}

Bill::Bill(double roomTotal, double servicesTotal, double tax){
    this ->roomTotal = roomTotal;
    this ->servicesTotal = servicesTotal;
    this->tax = tax;
}

double Bill::getRoomTotal() const{
    return roomTotal;
}

double Bill::getServicesTotal() const{
    return servicesTotal;
}
double Bill::getTax() const{
    return tax;
}
double Bill::calculateTotal() const {
    return roomTotal + servicesTotal + tax;
}
void Bill::display() const{
    std::cout << "\n========== BILL ==========\n";

    std::cout << "Room:      "
              << roomTotal << " EUR\n";

    std::cout << "Services:  "
              << servicesTotal << " EUR\n";

    std::cout << "Tax:       "
              << tax << " EUR\n";

    std::cout << "--------------------------\n";

    std::cout << "TOTAL:     "
              << calculateTotal()
              << " EUR\n";

    std::cout << "==========================\n";

}
