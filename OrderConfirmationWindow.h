#include <QWidget>
#include <QMap>
#include <QTimer>
#include <QLabel>

class OrderConfirmationWindow : public QWidget {
    Q_OBJECT
public:
    OrderConfirmationWindow(const QString& orderNumber,
                            const QMap<QString, int>& items,
                            double totalCost,
                            QWidget* parent = nullptr);

signals:
    void orderCompleted(const QString& orderNumber);

private slots:
    void updateOrderStatus();

private:
    void saveToDatabase();
    void updateInventory();
    QString generateStorageCell();
    double m_totalCost;
    QString m_orderNumber;
    QMap<QString, int> m_items;
    QString m_storageCell;
    QTimer* m_statusTimer;
    QLabel* m_statusLabel;
    QString generateNextOrderNumber();
};
