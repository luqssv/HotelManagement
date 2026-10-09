#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>

#include "HotelService.h"
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("HotelManagement");
    QApplication::setOrganizationName("HotelManagementLearning");

    // Optional: pass a database path as the first argument,
    // e.g. ./HotelManagementGUI ../hotel.db  (shares data with the console version)
    QString dbPath;
    const QStringList args = QCoreApplication::arguments();

    if (args.size() > 1)
    {
        dbPath = args.at(1);
    }
    else
    {
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dir);
        dbPath = dir + "/hotel.db";
    }

    HotelService service;
    QString error;
    if (!service.open(dbPath, error))
    {
        QMessageBox::critical(nullptr, "Database error",
                              "Could not open the database:\n" + error);
        return 1;
    }

    MainWindow window(service, dbPath);
    window.show();

    return app.exec();
}