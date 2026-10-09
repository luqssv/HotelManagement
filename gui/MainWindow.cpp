#include "MainWindow.h"

#include <QAction>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>

static QString money(double value)
{
    return QString::number(value, 'f', 2) + " DA";
}

MainWindow::MainWindow(HotelService& service, const QString& dbPath, QWidget* parent)
    : QMainWindow(parent), m_service(service)
{
    setWindowTitle("Hotel Management");
    resize(980, 620);

    m_rooms = makeTable();
    m_customers = makeTable();
    m_reservations = makeTable();
    m_bills = makeTable();

    m_stats = new QLabel;
    m_stats->setTextFormat(Qt::RichText);
    m_stats->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_stats->setMargin(16);

    auto* tabs = new QTabWidget;
    tabs->addTab(m_stats, "Dashboard");

    tabs->addTab(makeTab(m_rooms, {
        makeButton("Add room", &MainWindow::onAddRoom),
        makeButton("Add sample rooms", &MainWindow::onAddSampleRooms)
    }), "Rooms");

    tabs->addTab(makeTab(m_customers, {
        makeButton("Add customer", &MainWindow::onAddCustomer)
    }), "Customers");

    tabs->addTab(makeTab(m_reservations, {
        makeButton("New reservation", &MainWindow::onNewReservation),
        makeButton("Check in", &MainWindow::onCheckIn),
        makeButton("Generate bill", &MainWindow::onGenerateBill),
        makeButton("Check out", &MainWindow::onCheckOut)
    }), "Reservations");

    tabs->addTab(makeTab(m_bills, {}), "Bills");

    setCentralWidget(tabs);
    statusBar()->showMessage("Database: " + dbPath);

    // Menus
    QMenu* fileMenu = menuBar()->addMenu("&File");

    QAction* refresh = fileMenu->addAction("&Refresh");
    refresh->setShortcut(QKeySequence::Refresh);
    connect(refresh, &QAction::triggered, this, &MainWindow::refreshAll);

    QAction* quit = fileMenu->addAction("&Quit");
    quit->setShortcut(QKeySequence::Quit);
    connect(quit, &QAction::triggered, this, &QWidget::close);

    QMenu* helpMenu = menuBar()->addMenu("&Help");
    QAction* about = helpMenu->addAction("&About");
    connect(about, &QAction::triggered, this, &MainWindow::onAbout);

    refreshAll();
}

// ---------- UI helpers ----------

QTableWidget* MainWindow::makeTable()
{
    auto* table = new QTableWidget;
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(true);
    return table;
}

QPushButton* MainWindow::makeButton(const QString& text, void (MainWindow::*slot)())
{
    auto* button = new QPushButton(text);
    connect(button, &QPushButton::clicked, this, slot);
    return button;
}

QWidget* MainWindow::makeTab(QTableWidget* table, const QList<QPushButton*>& buttons)
{
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);

    auto* row = new QHBoxLayout;
    for (QPushButton* b : buttons)
        row->addWidget(b);
    row->addStretch();

    layout->addLayout(row);
    layout->addWidget(table);
    return page;
}

void MainWindow::fillTable(QTableWidget* table, QSqlQuery query, const QStringList& headers)
{
    table->clear();
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->setRowCount(0);

    int row = 0;
    while (query.next())
    {
        table->insertRow(row);
        for (int col = 0; col < headers.size(); ++col)
        {
            const QVariant v = query.value(col);
            const QString text = (v.typeId() == QMetaType::Double)
                                     ? QString::number(v.toDouble(), 'f', 2)
                                     : v.toString();
            table->setItem(row, col, new QTableWidgetItem(text));
        }
        ++row;
    }
    table->resizeColumnsToContents();
    table->horizontalHeader()->setStretchLastSection(true);
}

int MainWindow::selectedReservationId(const QString& title)
{
    const int row = m_reservations->currentRow();
    if (row < 0 || !m_reservations->item(row, 0))
    {
        QMessageBox::information(this, title, "Select a reservation in the table first.");
        return -1;
    }
    return m_reservations->item(row, 0)->text().toInt();
}

// ---------- Refresh ----------

void MainWindow::refreshAll()
{
    fillTable(m_rooms, m_service.roomsQuery(),
              {"ID", "Room", "Type", "Price / night (DA)", "Status"});
    fillTable(m_customers, m_service.customersQuery(),
              {"ID", "Name", "Phone", "Email"});
    fillTable(m_reservations, m_service.reservationsQuery(),
              {"ID", "Customer", "Room", "Check-in", "Check-out", "Status"});
    fillTable(m_bills, m_service.billsQuery(),
              {"Bill", "Reservation", "Customer", "Room",
               "Room total", "Services", "Tax", "Total (DA)"});

    const Stats s = m_service.stats();
    m_stats->setText(
        QString("<h2>Overview</h2>"
                "<h3>Rooms</h3>"
                "<p>Total: %1<br>Available: %2<br>Occupied: %3<br>"
                "Reserved: %4<br>Maintenance: %5</p>"
                "<h3>Customers</h3><p>Total: %6</p>"
                "<h3>Reservations</h3>"
                "<p>Total: %7<br>Active: %8<br>Completed: %9</p>"
                "<h3>Finance</h3>"
                "<p>Bills generated: %10<br>Recorded revenue: %11</p>")
            .arg(s.totalRooms).arg(s.available).arg(s.occupied)
            .arg(s.reserved).arg(s.maintenance)
            .arg(s.customers)
            .arg(s.reservations).arg(s.active).arg(s.completed)
            .arg(s.bills).arg(money(s.revenue)));
}

// ---------- Rooms ----------

void MainWindow::onAddSampleRooms()
{
    m_service.addSampleRooms();
    refreshAll();
}

void MainWindow::onAddRoom()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Add room");
    auto* form = new QFormLayout(&dlg);

    auto* number = new QSpinBox;
    number->setRange(1, 9999);
    number->setValue(101);

    auto* type = new QComboBox;
    type->addItems({"SINGLE", "DOUBLE", "SUITE"});

    auto* price = new QDoubleSpinBox;
    price->setRange(0, 10000000);
    price->setDecimals(2);
    price->setValue(3000);
    price->setSuffix(" DA");

    form->addRow("Room number:", number);
    form->addRow("Type:", type);
    form->addRow("Price per night:", price);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted)
        return;

    QString error;
    if (!m_service.addRoom(number->value(), type->currentText(), price->value(), error))
        QMessageBox::warning(this, "Add room", error);

    refreshAll();
}

// ---------- Customers ----------

void MainWindow::onAddCustomer()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Add customer");
    auto* form = new QFormLayout(&dlg);

    auto* name = new QLineEdit;
    auto* phone = new QLineEdit;
    auto* email = new QLineEdit;

    form->addRow("Name:", name);
    form->addRow("Phone:", phone);
    form->addRow("Email:", email);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted)
        return;

    QString error;
    if (!m_service.addCustomer(name->text(), phone->text(), email->text(), error))
        QMessageBox::warning(this, "Add customer", error);

    refreshAll();
}

// ---------- Reservations ----------

void MainWindow::onNewReservation()
{
    auto* customerBox = new QComboBox;
    QSqlQuery cq = m_service.customersQuery();
    while (cq.next())
    {
        customerBox->addItem(
            QString("%1 (#%2)").arg(cq.value(1).toString()).arg(cq.value(0).toInt()),
            cq.value(0).toInt());
    }

    auto* roomBox = new QComboBox;
    QSqlQuery rq = m_service.roomsQuery();
    while (rq.next())
    {
        if (rq.value(4).toString() != "AVAILABLE")
            continue;

        roomBox->addItem(
            QString("Room %1 - %2 - %3 DA/night")
                .arg(rq.value(1).toInt())
                .arg(rq.value(2).toString())
                .arg(rq.value(3).toDouble(), 0, 'f', 0),
            rq.value(0).toInt());
    }

    if (customerBox->count() == 0 || roomBox->count() == 0)
    {
        delete customerBox;
        delete roomBox;
        QMessageBox::information(this, "New reservation",
                                 "You need at least one customer and one available room first.");
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle("New reservation");
    auto* form = new QFormLayout(&dlg);

    auto* checkIn = new QDateEdit(QDate::currentDate());
    checkIn->setCalendarPopup(true);
    checkIn->setDisplayFormat("yyyy-MM-dd");

    auto* checkOut = new QDateEdit(QDate::currentDate().addDays(1));
    checkOut->setCalendarPopup(true);
    checkOut->setDisplayFormat("yyyy-MM-dd");

    form->addRow("Customer:", customerBox);
    form->addRow("Room:", roomBox);
    form->addRow("Check-in:", checkIn);
    form->addRow("Check-out:", checkOut);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted)
        return;

    QString error;
    if (!m_service.makeReservation(customerBox->currentData().toInt(),
                                   roomBox->currentData().toInt(),
                                   checkIn->date(), checkOut->date(), error))
    {
        QMessageBox::warning(this, "New reservation", error);
    }
    refreshAll();
}

void MainWindow::onCheckIn()
{
    const int id = selectedReservationId("Check in");
    if (id < 0)
        return;

    QString error;
    if (m_service.checkIn(id, error))
        QMessageBox::information(this, "Check in", "Check-in successful. The room is now OCCUPIED.");
    else
        QMessageBox::warning(this, "Check in", error);

    refreshAll();
}

void MainWindow::onGenerateBill()
{
    const int id = selectedReservationId("Generate bill");
    if (id < 0)
        return;

    bool ok = false;
    const double services = QInputDialog::getDouble(
        this, "Generate bill", "Extra services total (DA):",
        0, 0, 100000000, 2, &ok);
    if (!ok)
        return;

    BillResult bill;
    QString error;
    if (!m_service.generateBill(id, services, bill, error))
    {
        QMessageBox::warning(this, "Generate bill", error);
        return;
    }

    QMessageBox::information(
        this, "Bill saved",
        QString("Reservation #%1\n\n"
                "Nights: %2 x %3\n"
                "Room: %4\n"
                "Services: %5\n"
                "Subtotal: %6\n"
                "Tax (10 percent): %7\n"
                "-----------------------\n"
                "TOTAL: %8")
            .arg(id).arg(bill.nights).arg(money(bill.pricePerNight))
            .arg(money(bill.roomTotal)).arg(money(bill.services))
            .arg(money(bill.subtotal)).arg(money(bill.tax)).arg(money(bill.total)));

    refreshAll();
}

void MainWindow::onCheckOut()
{
    const int id = selectedReservationId("Check out");
    if (id < 0)
        return;

    QString error;
    if (m_service.checkOut(id, error))
        QMessageBox::information(this, "Check out", "Check-out successful. Thank you for staying with us!");
    else
        QMessageBox::warning(this, "Check out", error);

    refreshAll();
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About Hotel Management",
                       "A small hotel management app written in C++ with Qt6 and SQLite.\n\n"
                       "This is a learning project, not production software.");
}