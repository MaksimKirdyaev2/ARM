#include "OrderDialog.h"

OrderDialog::OrderDialog(QWidget *parent)
    : QDialog(parent),
    tableWidget(new QTableWidget(this)),
    confirmButton(new QPushButton("Подтвердить заказ", this)) {
    setWindowTitle("Подтверждение заказа");
    resize(500, 400);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tableWidget);
    layout->addWidget(confirmButton);
    connect(confirmButton, &QPushButton::clicked, this, [this]() {
        this->accept();
    });
}

void OrderDialog::setOrderList(const QList<QJsonObject> &orders) {
    confirmedOrders = orders;
    tableWidget->setColumnCount(3);
    tableWidget->setHorizontalHeaderLabels({"Название", "Текущее", "Заказать"});
    tableWidget->setRowCount(orders.size());
    for (int i = 0; i < orders.size(); ++i) {
        const QJsonObject &obj = orders[i];
        tableWidget->setItem(i, 0, new QTableWidgetItem(obj["Название"].toString()));
        tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(obj["Остаток"].toDouble(), 'f', 2)));
        tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(obj["Заказать"].toDouble(), 'f', 2)));
    }
    tableWidget->resizeColumnsToContents();
}

QList<QJsonObject> OrderDialog::getConfirmedOrders() const {
    return confirmedOrders;
}
