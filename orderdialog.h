#ifndef ORDERDIALOG_H
#define ORDERDIALOG_H
#include <QDialog>
#include <QJsonObject>
#include <QList>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>

class OrderDialog : public QDialog {
    Q_OBJECT

public:
    explicit OrderDialog(QWidget *parent = nullptr);
    void setOrderList(const QList<QJsonObject> &orders);
    QList<QJsonObject> getConfirmedOrders() const;

private:
    QTableWidget *tableWidget;
    QPushButton *confirmButton;
    QList<QJsonObject> confirmedOrders;
};

#endif // ORDERDIALOG_H
