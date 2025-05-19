#ifndef PRODUCT_H
#define PRODUCT_H
#include <QMainWindow>
#include <QJsonArray>
#include <QJsonObject>
#include <QTableWidgetItem>
#include <QDate>
#include <QSystemTrayIcon>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onDeleteProductClicked();
    void onSearchChanged(const QString &text);

private:
    Ui::MainWindow *ui;
    QJsonArray products;
    void updateTable();
    void saveToFile();
    void loadFromFile();

    void updateExpiringTable(const QJsonArray &expiringProducts);
    void onDeleteExpiringProductClicked();

    void openAddProductDialog();
    int findNextAvailableUid() const;
    void showBatchDetails(int row, int column);
    void onDeleteExpiredClicked();
    void removeExpiredBatchesForProduct(int index);
    void simulateRobotRemoval(const QString &productName, const QDate &expiryDate);
    void onOrderButtonClicked();
    bool simulateRobotReceive(const QString &productName);
    bool simulateRobotTransport(const QString &productName, int cellNumber);
    bool simulateRobotUnload(const QString &productName, int cellNumber);
    QTimer *orderCheckTimer;
    void checkAndOrderProducts();
    QSystemTrayIcon* trayIcon;
    QTimer* expirationTimer;
    void checkAndRemoveExpiredBatches();
    void setupExpirationTimer();
};

#endif // PRODUCT_H
