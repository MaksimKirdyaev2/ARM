#include "OrderConfirmationWindow.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFile>
#include <QJsonDocument>

OrderConfirmationWindow::OrderConfirmationWindow(const QString& orderNumber,
                                                 const QMap<QString, int>& items,
                                                 double totalCost,
                                                 QWidget* parent)
    : QWidget(parent),
    m_orderNumber(orderNumber),
    m_items(items),
    m_totalCost(totalCost),
    m_storageCell(generateStorageCell())
{
    setWindowTitle("Ожидание заказа");
    setFixedSize(400, 200);
    setStyleSheet(R"(
        QWidget {
            background-color: #f9fafb;
            font-family: Segoe UI, sans-serif;
            font-size: 14px;
            color: #333;
        }
        QLabel {
            padding: 6px;
        }
        QLabel#titleLabel {
            font-size: 18px;
            font-weight: bold;
            color: #2c3e50;
        }
        QLabel#statusLabel {
            font-style: italic;
            color: #555;
        }
    )");
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);
    // Заголовок
    QLabel* orderLabel = new QLabel(
        QString("Номер вашего заказа: %1").arg(orderNumber), this);
    orderLabel->setObjectName("titleLabel");
    orderLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(orderLabel);
    // Ячейка хранения
    QLabel* cellLabel = new QLabel(
        QString("Заказ будет храниться в ячейке: <b>%1</b>").arg(m_storageCell), this);
    cellLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(cellLabel);
    // Статус заказа
    m_statusLabel = new QLabel("Статус: Начало сборки заказа...", this);
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_statusLabel);
    layout->addStretch();
    // Запись в БД и инвентарь
    saveToDatabase();
    updateInventory();
    // Таймер сборки
    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &OrderConfirmationWindow::updateOrderStatus);
    m_statusTimer->start(5000); // Обновление каждые 5 секунд
}

void OrderConfirmationWindow::updateOrderStatus()
{
    static int progress = 0;
    progress += 20;
    if(progress >= 100) {
        m_statusTimer->stop();
        m_statusLabel->setText("Статус: Заказ собран и готов к выдаче!");
        QMessageBox::information(this, "Готово", "Заказ собран!");
        emit orderCompleted(m_orderNumber);
        this->deleteLater();
    }
    else {
        m_statusLabel->setText(
            QString("Статус: Идет сборка заказа... (%1%)").arg(progress));
    }
}

QString OrderConfirmationWindow::generateStorageCell()
{
    // Генерация случайного номера ячейки формата A-XXX
    int cellNumber = QRandomGenerator::global()->bounded(100, 999);
    return QString("A-%1").arg(cellNumber);
}

void OrderConfirmationWindow::saveToDatabase()
{
    const QString filename = "orders.json";
    // Создаем объект заказа
    QJsonObject orderObject;
    orderObject["Номер заказа"] = m_orderNumber;
    orderObject["Дата"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    orderObject["Общая стоимость"] = m_totalCost;
    orderObject["Ячейка"] = m_storageCell;
    // Формируем объект с товарами
    QJsonObject itemsObject;
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        itemsObject[it.key()] = it.value();
    }
    orderObject["Товары"] = itemsObject;
    // Читаем существующие данные
    QJsonArray ordersArray;
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        ordersArray = doc.array();
        file.close();
    }
    // Добавляем новый заказ
    ordersArray.append(orderObject);
    // Записываем обратно в файл
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(ordersArray);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    } else {
        qWarning() << "Не удалось сохранить заказ в файл!";
    }
}




void OrderConfirmationWindow::updateInventory()
{
    // Открываем и читаем файл products.json
    QFile file("products.json");
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "Не удалось открыть файл products.json";
        return;
    }
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray productsArray = doc.array();
    file.close();
    // Для каждого товара в заказе
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        QString productName = it.key();
        int quantityToDeduct = it.value();
        qDebug() << "Списание товара:" << productName << "Количество:" << quantityToDeduct;
        // Находим товар в массиве
        for (int i = 0; i < productsArray.size(); ++i) {
            QJsonObject product = productsArray[i].toObject();
            if (product["Название"].toString() == productName) {
                QJsonArray batches = product["Партии"].toArray();
                int remainingQuantity = quantityToDeduct;
                // Проходим по партиям в исходном порядке (первая - самая старая)
                for (int j = 0; j < batches.size() && remainingQuantity > 0; ++j) {
                    QJsonObject batch = batches[j].toObject();
                    int batchQuantity = batch["Количество"].toInt();
                    int deductAmount = qMin(remainingQuantity, batchQuantity);
                    batch["Количество"] = batchQuantity - deductAmount;
                    remainingQuantity -= deductAmount;
                    batches[j] = batch;
                    qDebug() << "Списано" << deductAmount << "из партии с истечением"
                             << batch["Истечение срока годности"].toString();
                }
                if (remainingQuantity > 0) {
                    qDebug() << "Внимание! Недостаточно товара" << productName
                             << "на складе. Осталось списать:" << remainingQuantity;
                }
                // Обновляем партии в товаре
                product["Партии"] = batches;
                productsArray[i] = product;
                break;
            }
        }
    }
    // Записываем обновленные данные обратно в файл
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qDebug() << "Не удалось открыть файл для записи";
        return;
    }
    doc.setArray(productsArray);
    file.write(doc.toJson());
    file.close();
    qDebug() << "Инвентарь успешно обновлен";
}
