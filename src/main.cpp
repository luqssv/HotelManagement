#include "Hotel.h"
#include <iostream>

int main()
{
    Hotel hotel;

    if (!hotel.initialize())
    {
        std::cerr << "Failed to initialize hotel system.\n";
        return 1;
    }

    hotel.menu();

    return 0;
}