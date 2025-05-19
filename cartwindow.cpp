#include "CartWindow.h"
#include "OrderConfirmationWindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>
#include <QPixmap>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QMessageBox>
#include <QJsonarray>

static int orderCounter = 0;

CartWindow::CartWindow(const QMap<QString, int> &cartData, QWidget *parent)
    : QMainWindow(parent), cartData(cartData)
{
    setWindowTitle("Корзина");
    resize(600, 400);
    // Загружаем product.json
    QFile file("products.json");
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть product.json");
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonArray productsArray = doc.array();
    for (const QJsonValue &productVal : productsArray) {
        QJsonObject productObj = productVal.toObject();
        QString productName = productObj["Название"].toString();

        ProductInfo info;
        info.price = productObj["Цена"].toDouble();
        info.imagePath = productObj["Изображение"].toString();
        if(info.imagePath.isEmpty()) {
            info.imagePath = "images/default.png";
        }

        productDetails[productName] = info;
    }
    // Центральный виджет и основной макет
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    // Прокручиваемая область (должна быть первой)
    scrollArea = new QScrollArea(this);
    QWidget *scrollContent = new QWidget();
    productsLayout = new QVBoxLayout(scrollContent);
    scrollContent->setLayout(productsLayout);
    scrollArea->setWidget(scrollContent);
    scrollArea->setWidgetResizable(true);
    mainLayout->addWidget(scrollArea);
    // Блок общей стоимости
    QWidget *totalWidget = new QWidget();
    QHBoxLayout *totalLayout = new QHBoxLayout(totalWidget);
    QLabel *totalTextLabel = new QLabel("Общая стоимость:");
    totalLabel = new QLabel();
    totalTextLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    totalLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #2c3e50;");
    totalLayout->addWidget(totalTextLabel, 0, Qt::AlignRight);
    totalLayout->addWidget(totalLabel, 0, Qt::AlignLeft);
    totalLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->addWidget(totalWidget);
    // Кнопка оформления (добавляется последней)
    checkoutButton = new QPushButton("Оформить заказ", this);
    checkoutButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #4CAF50;"
        "  color: white;"
        "  border: none;"
        "  padding: 10px 20px;"
        "  border-radius: 5px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #45a049; }"
        );
    mainLayout->addWidget(checkoutButton);
    connect(checkoutButton, &QPushButton::clicked, this, &CartWindow::handleCheckout);
    setCentralWidget(centralWidget);
    scrollContent->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    updateCart(cartData);
}

void CartWindow::updateCart(const QMap<QString, int>& newCartData)
{
    cartData = newCartData;
    double total = 0.0;
    // Очистка предыдущих элементов
    QLayoutItem *child;
    while ((child = productsLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    for (auto it = cartData.begin(); it != cartData.end(); ++it) {
        QString name = it.key();
        int quantity = it.value();
        if(!productDetails.contains(name)) continue;
        ProductInfo info = productDetails.value(name);
        total += info.price * quantity;
        QWidget *tile = new QWidget(this);
        tile->setFixedHeight(100);
        tile->setStyleSheet("background-color: #f8f9fa; border-radius: 8px;");
        QHBoxLayout *tileLayout = new QHBoxLayout(tile);
        tileLayout->setContentsMargins(15, 10, 15, 10);
        // Добавление изображения
        QLabel *imageLabel = new QLabel();
        QPixmap pix(info.imagePath);
        if(pix.isNull()) {
            pix.load("images/default.png");
        }
        imageLabel->setPixmap(pix.scaled(100, 80, Qt::KeepAspectRatio));
        tileLayout->addWidget(imageLabel);
        // Текстовая информация
        QVBoxLayout *textLayout = new QVBoxLayout();
        QLabel *nameLabel = new QLabel(name);
        nameLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
        QLabel *priceLabel = new QLabel(QString("Цена: %1 ₽").arg(info.price));
        QLabel *qtyLabel = new QLabel(QString("Количество: %1").arg(quantity));
        textLayout->addWidget(nameLabel);
        textLayout->addWidget(priceLabel);
        textLayout->addWidget(qtyLabel);
        tileLayout->addLayout(textLayout);
        // Кнопка удаления
        QPushButton *removeButton = new QPushButton("Удалить");
        removeButton->setStyleSheet(
            "QPushButton {"
            "  background-color: #e74c3c;"
            "  color: white;"
            "  border: none;"
            "  padding: 8px 15px;"
            "  border-radius: 5px;"
            "}"
            "QPushButton:hover { background-color: #c0392b; }"
            );
        connect(removeButton, &QPushButton::clicked, [=]() {
            cartData.remove(name);
            updateCart(cartData);
        });
        tileLayout->addWidget(removeButton);
        productsLayout->addWidget(tile);
    }
    // Обновление общей стоимости
    totalLabel->setText(QString("<b>%1 ₽</b>").arg(total, 0, 'f', 2));
    // Добавляем растягивающийся элемент в конец
    productsLayout->addStretch();
    scrollArea->widget()->adjustSize();
}

QString generateNextOrderNumber() {
    QFile file("orders.json");
    int maxNumber = -1;
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isArray()) {
            QJsonArray array = doc.array();
            for (const QJsonValue& val : array) {
                if (val.isObject()) {
                    QJsonObject obj = val.toObject();
                    QString numStr = obj.value("Номер заказа").toString();
                    bool ok;
                    int num = numStr.toInt(&ok);
                    if (ok && num > maxNumber) {
                        maxNumber = num;
                    }
                }
            }
        }
    }

    int nextNumber = maxNumber + 1;
    return QString("%1").arg(nextNumber, 6, 10, QChar('0'));
}
void CartWindow::handleCheckout()
{
    QString orderNumber = generateNextOrderNumber();
    double totalCost = 0.0;
    OrderConfirmationWindow* confirmWindow = new OrderConfirmationWindow(orderNumber, cartData, totalCost);
    confirmWindow->show();
    emit orderCompleted();
    this->close();
}
