cmake_minimum_required(VERSION 3.16)
project(HotelManagement LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_CLI "Build the console version" ON)
option(BUILD_GUI "Build the Qt6 GUI version" ON)

# ---------- Console version ----------
if(BUILD_CLI)
    find_package(SQLite3 REQUIRED)

    add_executable(HotelManagement
        src/main.cpp
        src/Database.cpp
        src/Room.cpp
        src/Customer.cpp
        src/Reservation.cpp
        src/Bill.cpp
        src/Hotel.cpp
    )
    target_include_directories(HotelManagement PRIVATE include)
    target_link_libraries(HotelManagement PRIVATE SQLite::SQLite3)
endif()

# ---------- Qt6 GUI version ----------
if(BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

    set(CMAKE_AUTOMOC ON)

    qt_add_executable(HotelManagementGUI
        gui/main.cpp
        gui/MainWindow.cpp
        gui/MainWindow.h
        gui/HotelService.cpp
        gui/HotelService.h
    )
    target_link_libraries(HotelManagementGUI PRIVATE Qt6::Widgets Qt6::Sql)

    set_target_properties(HotelManagementGUI PROPERTIES
        WIN32_EXECUTABLE ON   # no console window on Windows
    )
endif()