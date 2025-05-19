#ifndef CUSTOMERWINDOW_H
#define CUSTOMERWINDOW_H
#include <QMainWindow>
#include <QTableWidget>
#include <QSpinBox>
#include <QPushButton>
#include <QMainWindow>
#include <QScrollArea>
#include <QGridLayout>
#include <QMap>
#include <QString>
#include "CartWindow.h"

class CustomerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CustomerWindow(QWidget *parent = nullptr);
    ~CustomerWindow();

private:
    QTableWidget *tableProducts;  // Таблица с продуктами
    QSpinBox *spinQuantity;       // Поле ввода количества
    QPushButton *btnAddToCart;    // Кнопка добавления в корзину
    QMap<QString, int> cartData;
    void loadProductsFromJson();  // Загрузка продуктов из JSON
    CartWindow *cartWin = nullptr;

private slots:
    void addToCart(const QString &productName, int quantity);            // Слот добавления в корзину
    void removeFromCart(const QString &productName, int quantity);  // Слот удаления из корзины
    void openCartWindow();
    double calculateTotalAmount(QJsonArray &batches);
signals:
    void cartUpdated(const QMap<QString, int> &cartData);

};

#endif // CUSTOMERWINDOW_H
