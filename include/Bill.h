#ifndef BILL_H
#define BILL_H

class Bill
{
private:
    double roomTotal;
    double servicesTotal;
    double tax;

public:
    Bill();

    Bill(
        double roomTotal,
        double servicesTotal,
        double tax
    );

    double getRoomTotal() const;
    double getServicesTotal() const;
    double getTax() const;

    double calculateTotal() const;

    void display() const;
};

#endif