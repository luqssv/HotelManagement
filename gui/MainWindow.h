#pragma once

#include <QList>
#include <QMainWindow>

#include "HotelService.h"

class QLabel;
class QPushButton;
class QTableWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(HotelService& service, const QString& dbPath, QWidget* parent = nullptr);

private slots:
    void refreshAll();
    void onAddSampleRooms();
    void onAddRoom();
    void onAddCustomer();
    void onNewReservation();
    void onCheckIn();
    void onCheckOut();
    void onGenerateBill();
    void onAbout();

private:
    QTableWidget* makeTable();
    QPushButton* makeButton(const QString& text, void (MainWindow::*slot)());
    QWidget* makeTab(QTableWidget* table, const QList<QPushButton*>& buttons);

    void fillTable(QTableWidget* table, QSqlQuery query, const QStringList& headers);
    int selectedReservationId(const QString& title);

    HotelService& m_service;

    QLabel* m_stats = nullptr;
    QTableWidget* m_rooms = nullptr;
    QTableWidget* m_customers = nullptr;
    QTableWidget* m_reservations = nullptr;
    QTableWidget* m_bills = nullptr;
};