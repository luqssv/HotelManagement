# Hotel Management

A small hotel management system written in **C++17** with **SQLite**, available as a console app and as a **Qt6 desktop GUI**.

> **This is a learning project.** I built it to practice C++, SQL, and Qt, and I keep improving it as I learn. It is not production software, so don't use it to manage a real hotel or store real customer data.

## Features

- Manage rooms (single, double, suite) and customers
- Make reservations with date validation and overlap checking
- Check in and check out guests
- Generate bills (nights x price + services + 10% tax)
- Dashboard and reports: room status, reservations, revenue
- Two interfaces sharing the same database schema:
  - `HotelManagement` - console menu
  - `HotelManagementGUI` - Qt6 graphical interface
  - Delete rooms, customers, and reservations (with safety checks so records that are still in use can't be removed)

## Project structure

```
include/   headers for the console version
src/       console version (Hotel, Room, Customer, Reservation, Bill, Database)
gui/       Qt6 GUI (MainWindow + HotelService database layer)
CMakeLists.txt
```

## Requirements

- A C++17 compiler
- CMake 3.16+
- Qt 6 (Widgets and Sql modules) for the GUI
- SQLite3 development files for the console version (optional)

## Build on Fedora

```bash
sudo dnf install gcc-c++ cmake ninja-build qt6-qtbase-devel sqlite-devel

git clone https://github.com/luqssv/HotelManagement.git
cd HotelManagement

cmake -S . -B build -G Ninja
cmake --build build

./build/HotelManagementGUI     # GUI
./build/HotelManagement        # console version
```

## Build on Windows

1. Install **Qt 6** with the [Qt Online Installer](https://www.qt.io/download-qt-installer-oss). Select a kit (e.g. *MinGW 64-bit*) plus *CMake* and *Ninja* under Developer and Designer Tools.
2. Easiest: open `CMakeLists.txt` in **Qt Creator**, pick the kit, and press Run.
3. Or from the command line (adjust the Qt path and version):

```bat
cmake -S . -B build -G Ninja -DBUILD_CLI=OFF -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/mingw_64
cmake --build build
build\HotelManagementGUI.exe
```

To run the `.exe` on a PC without Qt installed, run `windeployqt build\HotelManagementGUI.exe` first.

`-DBUILD_CLI=OFF` skips the console version, which needs a separate SQLite install on Windows (for example via vcpkg).

## Where is the data stored?

- **GUI**: `hotel.db` in your user data folder (Qt's `AppDataLocation`), or any path you pass as the first argument: `./HotelManagementGUI /path/to/hotel.db`
- **Console**: `hotel.db` in the folder you run it from

Both use the same tables, so you can point the GUI at the console's database.

## Known limitations / ideas

- Prices are in euros. VAT differs per country, so the rate is a setting (GUI: Settings > VAT rate) stored in the database and also used by the console version. Check the rate that applies to hotel stays where you operate.
- A room must be `AVAILABLE` to be booked, so a room reserved for next month can't take another booking for dates before it
- No user accounts or login
- The console version builds some SQL by string concatenation; the GUI uses prepared statements
- No unit tests yet

## License

Choose a license (for example MIT) and add a `LICENSE` file.