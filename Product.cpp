#include "Product.h"
#include "ui_Product.h"
#include <QFile>
#include <QJsonDocument>
#include <QMessageBox>
#include <QHeaderView>
#include <QDate>
#include "AddProductDialog.h"
#include <QFileInfo>
#include "OrderDialog.h"
#include <QTimer>
#include <QSystemTrayIcon>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    trayIcon(new QSystemTrayIcon(this))
{
    ui->setupUi(this);
    setFixedSize(1000, 600);
    // Создаём главный layout
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    // Настроим таблицу
    ui->tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // Сделаем кнопки одинакового размера
    QSize buttonSize(213, 40);
    ui->pushButtonAddProduct->setFixedSize(buttonSize);
    ui->pushButtonDelete->setFixedSize(buttonSize);
    ui->pushButtonCreate->setFixedSize(buttonSize);
    ui->pushButtonOrder->setFixedSize(buttonSize);
    // Кнопки в одну строку по центру
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(ui->pushButtonAddProduct);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(ui->pushButtonDelete);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(ui->pushButtonCreate);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(ui->pushButtonOrder);
    buttonLayout->addStretch();
    // Собираем основной макет
    mainLayout->addStretch();
    mainLayout->addWidget(ui->tableWidget, 6);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();
    central->setLayout(mainLayout);
    setCentralWidget(central);
    // Стиль
    this->setStyleSheet(R"(
    QMainWindow {
        background-color: #f0f4f8;
        font-family: "Segoe UI", sans-serif;
        font-size: 14px;
    }

    QPushButton {
        background-color: #0078d7;
        color: white;
        border-radius: 8px;
        padding: 8px 16px;
        font-weight: bold;
    }
    QPushButton:hover {
        background-color: #005a9e;
    }
    QPushButton:pressed {
        background-color: #003f6f;
    }

    QLineEdit {
        border: 1px solid #ccc;
        border-radius: 6px;
        padding: 6px;
        background-color: white;
    }

    QTableWidget {
        background-color: white;             /* фон таблицы */
        gridline-color: black;               /* цвет линий сетки */
        selection-background-color: #d6ccb9; /* бежево-серый цвет выделения */
        selection-color: black;
        border: 1px solid #ccc;
    }

    QHeaderView::section {
        background-color: #0078d7;
        color: white;
        padding: 6px;
        border: none;
    }

    QLabel {
        font-weight: bold;
    }
)");
    // Подключение сигналов
    connect(ui->tableWidget, &QTableWidget::cellDoubleClicked, this, &MainWindow::showBatchDetails);
    connect(ui->pushButtonCreate, &QPushButton::clicked, this, &MainWindow::onDeleteExpiredClicked);
    connect(ui->pushButtonDelete, &QPushButton::clicked, this, &MainWindow::onDeleteProductClicked);
    connect(ui->lineEditSearch, &QLineEdit::textChanged, this, &MainWindow::onSearchChanged);
    connect(ui->pushButtonAddProduct, &QPushButton::clicked, this, &MainWindow::openAddProductDialog);
    connect(ui->pushButtonOrder, &QPushButton::clicked, this, &MainWindow::onOrderButtonClicked);
    orderCheckTimer = new QTimer(this);
    expirationTimer=new QTimer(this);
    connect(orderCheckTimer, &QTimer::timeout, this, &MainWindow::checkAndOrderProducts);
    connect(expirationTimer, &QTimer::timeout, this, &MainWindow::checkAndRemoveExpiredBatches);
    // Проверяем каждые 24 часа (можно изменить интервал)
    expirationTimer->start(50000);
    orderCheckTimer->start(50000);
    // Первая проверка при запуске
    //QTimer::singleShot(0, this, &MainWi7ndow::checkAndRemoveExpiredBatches);
    loadFromFile();
    updateTable();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onDeleteProductClicked() {
    QList<QTableWidgetItem*> items = ui->tableWidget->selectedItems();
    if (items.isEmpty()) return;
    QSet<QString> namesToDelete;
    for (QTableWidgetItem* item : std::as_const(items)) {
        int row = item->row();
        QString name = ui->tableWidget->item(row, 0)->text();
        namesToDelete.insert(name);
    }
    for (int i = products.size() - 1; i >= 0; --i) {
        QJsonObject obj = products[i].toObject();
        if (namesToDelete.contains(obj["Название"].toString())) {
            products.removeAt(i);
        }
    }
    saveToFile();
    updateTable();
}

void MainWindow::onSearchChanged(const QString &text) {
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QTableWidgetItem *item = ui->tableWidget->item(row, 0);
        ui->tableWidget->setRowHidden(row, !item->text().contains(text, Qt::CaseInsensitive));
    }
}


void MainWindow::showBatchDetails(int row, int column) {
    Q_UNUSED(column);
    QJsonObject obj = products[row].toObject();
    QString name = obj["Название"].toString();
    QJsonArray batches = obj["Партии"].toArray();
    QString details = QString("Партии товара \"%1\":\n\n").arg(name);
    QDate currentDate = QDate::currentDate();
    for (const QJsonValue &batchVal : batches) {
        QJsonObject batch = batchVal.toObject();
        double amount = batch["Количество"].toDouble();
        QString manufactureDate = batch["Дата изготовления"].toString();
        QString expiryDateStr = batch["Истечение срока годности"].toString();
        QDate expiryDate = QDate::fromString(expiryDateStr, "yyyy-MM-dd");
        QString status;
        if (expiryDate.isValid()) {
            if (expiryDate <= currentDate) {
                status = "Просрочена";
            } else if (expiryDate <= currentDate.addDays(1)) {
                status = "Скоро истечет";
            } else {
                status = "В порядке";
            }
        } else {
            status = "Дата не указана";
        }
        details += QString("Кол-во: %1, Изготовлено: %2, Годна до: %3 — %4\n")
                       .arg(amount).arg(manufactureDate).arg(expiryDateStr).arg(status);
    }
    QMessageBox::information(this, "Детали партий", details);
}

void MainWindow::updateTable() {
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(products.size());
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setHorizontalHeaderLabels({"Название", "Общее кол-во", "Статус"});
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    QDate currentDate = QDate::currentDate();
    for (int i = 0; i < products.size(); ++i) {
        QJsonObject obj = products[i].toObject();
        QString name = obj["Название"].toString();
        QJsonArray batches = obj["Партии"].toArray();
        double total=0;
        bool hasExpired = false;
        bool hasExpiringSoon = false;
        bool allExpired = true;
        for (const QJsonValue &batchVal : batches) {
            QJsonObject batch = batchVal.toObject();
            double amount = batch["Количество"].toDouble();
            total += amount;
            QDate expiryDate = QDate::fromString(batch["Истечение срока годности"].toString(), "yyyy-MM-dd");
            if (expiryDate.isValid()) {
                if (expiryDate <= currentDate) {
                    hasExpired = true;
                }
                if (expiryDate > currentDate) {
                    allExpired = false;
                    if (expiryDate <= currentDate.addDays(1)) {
                        hasExpiringSoon = true;
                    }
                }
            } else {
                allExpired = false; // если даты нет, считаем, что партия не просрочена
            }
        }
        // Создаем элементы ячеек
        QTableWidgetItem *nameItem = new QTableWidgetItem(name);
        QTableWidgetItem *totalItem = new QTableWidgetItem(QString::number(total, 'f', 2));
        QString statusText;
        if (allExpired && hasExpired) {
            statusText = "Все просрочено";
        } else if (hasExpired || hasExpiringSoon) {
            statusText = "Частично просрочено";
        } else {
            statusText = "В порядке";
        }
        QTableWidgetItem *statusItem = new QTableWidgetItem(statusText);
        // Устанавливаем элементы в таблицу
        ui->tableWidget->setItem(i, 0, nameItem);
        ui->tableWidget->setItem(i, 1, totalItem);
        ui->tableWidget->setItem(i, 2, statusItem);
        // Окрашиваем строку в зависимости от статуса
        QColor bgColor;
        QColor fgColor = Qt::black;
        if (allExpired && hasExpired) {
            bgColor = Qt::red;
            fgColor = Qt::white;
        } else if (hasExpired || hasExpiringSoon) {
            bgColor = QColor(255, 165, 0); // оранжевый
        } else {
            bgColor = QColor(144, 238, 144); // светло-зеленый
        }
        for (int col = 0; col < 3; ++col) {
            QTableWidgetItem *item = ui->tableWidget->item(i, col);
            if (item) {
                item->setBackground(bgColor);
                item->setForeground(fgColor);
            }
        }
    }
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->setSortingEnabled(true);
}

void MainWindow::saveToFile() {
    QFile file("products.json");
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл products.json для записи");
        return;
    }
    qDebug() << "Saving to:" << QFileInfo(file).absoluteFilePath();
    QByteArray data = QJsonDocument(products).toJson();
    qDebug() << "Data size:" << data.size();
    file.write(data);
    file.close();
}

void MainWindow::loadFromFile() {
    QFile file("products.json");
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isArray()) {
            products = doc.array();
        }
        file.close();
    }
}
void MainWindow::openAddProductDialog() {
    AddProductDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getName();
        int shelfLifeDays = dialog.getShelfLifeDays();
        double price = dialog.getPrice();  // <-- Получаем цену
        int minThreshold = dialog.getMinStockThreshold();
        int optimalStock = dialog.getOptimalStockLevel();
        QString imagePath = dialog.getImagePath();
        // Проверка уникальности по названию
        for (const QJsonValue &value : products) {
            if (value.toObject()["Название"].toString().compare(name, Qt::CaseInsensitive) == 0) {
                QMessageBox::warning(this, "Ошибка", "Такой продукт уже существует.");
                return;
            }
        }
        int newUid = findNextAvailableUid();
        QJsonObject newProduct;
        newProduct["UID"] = newUid;
        newProduct["Название"] = name;
        newProduct["Срок хранения"] = shelfLifeDays;
        newProduct["Цена"] = price;         // <-- Добавляем цену
        newProduct["Партии"] = QJsonArray();
        newProduct["Изображение"] = imagePath;
        newProduct["Мин. порог"] = minThreshold;
        newProduct["Оптимальное кол-во"] = optimalStock;
        products.append(newProduct);
        saveToFile();
        updateTable();

    }
}

int MainWindow::findNextAvailableUid() const {
    QSet<int> usedUids;
    for (const QJsonValue &value : products) {
        QJsonObject obj = value.toObject();
        int uid = obj["UID"].toInt(0);
        usedUids.insert(uid);
    }

    int candidate = 1;
    while (usedUids.contains(candidate)) {
        candidate++;
    }
    return candidate;
}
void MainWindow::onDeleteExpiredClicked() {
    int row = ui->tableWidget->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Ошибка", "Выберите товар в таблице");
        return;
    }

    removeExpiredBatchesForProduct(row);
}
void MainWindow::removeExpiredBatchesForProduct(int index) {
    if (index < 0 || index >= products.size()) return;

    QJsonObject product = products[index].toObject();
    QString productName = product["Название"].toString();
    QJsonArray batches = product["Партии"].toArray();
    QJsonArray updatedBatches;
    QDate currentDate = QDate::currentDate();

    // Загружаем архив списанных партий
    QFile archiveFile("expired_archive.json");
    QJsonArray expiredBatchesArchive;

    if (archiveFile.open(QIODevice::ReadOnly)) {
        QJsonDocument archiveDoc = QJsonDocument::fromJson(archiveFile.readAll());
        expiredBatchesArchive = archiveDoc.array();
        archiveFile.close();
    }

    for (const QJsonValue &batchVal : batches) {
        QJsonObject batch = batchVal.toObject();
        QDate expiry = QDate::fromString(batch["Истечение срока годности"].toString(), "yyyy-MM-dd");

        if (expiry > currentDate) {
            updatedBatches.append(batch);
        } else {
            // Добавляем дополнительную информацию при архивировании
            QJsonObject expiredBatch = batch;
            expiredBatch["Название товара"] = productName;
            expiredBatch["Дата списания"] = currentDate.toString("yyyy-MM-dd");
            expiredBatch["UID товара"] = product["UID"].toInt();

            expiredBatchesArchive.append(expiredBatch);
            simulateRobotRemoval(productName, expiry);
        }
    }
    // Сохраняем архив списанных партий
    if (archiveFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QJsonDocument archiveDoc(expiredBatchesArchive);
        archiveFile.write(archiveDoc.toJson());
        archiveFile.close();
    }
    // Обновляем основной список
    product["Партии"] = updatedBatches;
    products[index] = product;

    saveToFile();
    updateTable();
}
void MainWindow::simulateRobotRemoval(const QString &productName, const QDate &expiryDate) {
    QMessageBox::information(
        this,
        "Робот в пути",
        QString("Робот забирает просроченную партию товара \"%1\" с датой %2...")
            .arg(productName)
            .arg(expiryDate.toString("yyyy-MM-dd"))
        );
}
void MainWindow::onOrderButtonClicked() {
    QDate currentDate = QDate::currentDate();
    QList<QJsonObject> orders;

    for (int i = 0; i < products.size(); ++i) {
        QJsonObject product = products[i].toObject();
        QString name = product["Название"].toString().trimmed();
        int minThreshold = product["Мин. порог"].toInt(0);
        int optimalAmount = product["Оптимальное кол-во"].toInt(0);

        QJsonArray batches = product["Партии"].toArray();
        double currentAmount = 0;

        for (const QJsonValue &val : batches) {
            QJsonObject batch = val.toObject();
            QDate expiry = QDate::fromString(batch["Истечение срока годности"].toString(), "yyyy-MM-dd");
            if (expiry > currentDate) {
                currentAmount += batch["Количество"].toDouble();
            }
        }

        if (currentAmount < minThreshold) {
            double toOrder = optimalAmount - currentAmount;
            if (toOrder > 0) {
                QJsonObject order;
                order["Index"] = i;
                order["Название"] = name;
                order["Остаток"] = currentAmount;
                order["Заказать"] = toOrder;
                orders.append(order);
            }
        }
    }

    if (orders.isEmpty()) {
        QMessageBox::information(this, "Заказ", "Нет продуктов для заказа.");
        return;
    }

    QString orderText = "Вы собираетесь заказать следующие продукты:\n\n";
    for (const QJsonObject &order : orders) {
        QString name = order["Название"].toString();
        double amount = order["Заказать"].toDouble();
        orderText += QString("- %1 : %2 шт.\n").arg(name).arg(amount);
    }
    orderText += "\nПодтвердить заказ?";

    if (QMessageBox::question(this, "Подтверждение заказа", orderText) == QMessageBox::Yes) {
        for (const QJsonObject &order : orders) {
            int index = order["Index"].toInt();
            double amount = order["Заказать"].toDouble();

            QJsonObject product = products[index].toObject();
            int shelfLifeDays = product["Срок хранения"].toInt(30);
            int uid = product["UID"].toInt();
            QString name = product["Название"].toString();

            // Последовательно выполняем все шаги, проверяя каждый
            bool received = simulateRobotReceive(name);
            if (!received) continue;

            bool transported = simulateRobotTransport(name, uid);
            if (!transported) continue;

            bool unloaded = simulateRobotUnload(name, uid);
            if (!unloaded) continue;
            // Все флаги прошли — добавляем партию
            QDate manufactureDate = QDate::currentDate();
            QDate expiry = manufactureDate.addDays(shelfLifeDays);
            QJsonObject newBatch;
            newBatch["Количество"] = amount;
            newBatch["Дата изготовления"] = manufactureDate.toString("yyyy-MM-dd");
            newBatch["Истечение срока годности"] = expiry.toString("yyyy-MM-dd");
            QJsonArray batches = product["Партии"].toArray();
            batches.append(newBatch);
            product["Партии"] = batches;
            products[index] = product;
        }

        saveToFile();
        updateTable();
        QMessageBox::information(this, "Заказ", "Партии успешно добавлены после доставки роботом.");
    }
}


bool MainWindow::simulateRobotReceive(const QString &productName) {
    // Здесь можно заменить на случайный отказ или проверку условий
    QMessageBox::information(this, "Робот",
                             QString("Робот принял заказ на продукт \"%1\".").arg(productName));
    return true; // можно заменить на false для отладки
}

bool MainWindow::simulateRobotTransport(const QString &productName, int cellNumber) {
    QMessageBox::information(this, "Робот",
                             QString("Робот в пути с продуктом \"%1\" к ячейке #%2...").arg(productName).arg(cellNumber));
    return true;
}

bool MainWindow::simulateRobotUnload(const QString &productName, int cellNumber) {
    QMessageBox::information(this, "Робот",
                             QString("Робот выгрузил продукт \"%1\" в ячейку #%2.").arg(productName).arg(cellNumber));
    return true;
}
void MainWindow::checkAndOrderProducts()
{
    QDate currentDate = QDate::currentDate();
    QList<QJsonObject> orders;

    // Анализ продуктов для заказа
    for (int i = 0; i < products.size(); ++i) {
        QJsonObject product = products[i].toObject();
        QString name = product["Название"].toString().trimmed();
        int minThreshold = product["Мин. порог"].toInt(0);
        int optimalAmount = product["Оптимальное кол-во"].toInt(0);

        QJsonArray batches = product["Партии"].toArray();
        double currentAmount = 0;

        for (const QJsonValue &val : batches) {
            QJsonObject batch = val.toObject();
            QDate expiry = QDate::fromString(batch["Истечение срока годности"].toString(), "yyyy-MM-dd");
            if (expiry > currentDate) {
                currentAmount += batch["Количество"].toDouble();
            }
        }

        if (currentAmount < minThreshold) {
            double toOrder = optimalAmount - currentAmount;
            if (toOrder > 0) {
                QJsonObject order;
                order["Index"] = i;
                order["Название"] = name;
                order["Остаток"] = currentAmount;
                order["Заказать"] = toOrder;
                orders.append(order);
            }
        }
    }

    if (orders.isEmpty()) {
        qDebug() << QDateTime::currentDateTime().toString() << "Нет продуктов для заказа.";
        return;
    }

    // Логирование информации о заказе
    QString orderText = "Автоматически заказаны продукты:\n\n";
    for (const QJsonObject &order : orders) {
        QString name = order["Название"].toString();
        double amount = order["Заказать"].toDouble();
        orderText += QString("- %1 : %2 шт.\n").arg(name).arg(amount);
    }
    qDebug() << orderText;

    // Автоматическое выполнение заказа
    for (const QJsonObject &order : orders) {
        int index = order["Index"].toInt();
        double amount = order["Заказать"].toDouble();

        QJsonObject product = products[index].toObject();
        int shelfLifeDays = product["Срок хранения"].toInt(30);
        int uid = product["UID"].toInt();
        QString name = product["Название"].toString();

        // Последовательно выполняем все шаги
        bool received = simulateRobotReceive(name);
        if (!received) {
            qDebug() << "Ошибка получения товара:" << name;
            continue;
        }

        bool transported = simulateRobotTransport(name, uid);
        if (!transported) {
            qDebug() << "Ошибка транспортировки товара:" << name;
            continue;
        }

        bool unloaded = simulateRobotUnload(name, uid);
        if (!unloaded) {
            qDebug() << "Ошибка выгрузки товара:" << name;
            continue;
        }

        // Добавляем новую партию
        QDate manufactureDate = QDate::currentDate();
        QDate expiry = manufactureDate.addDays(shelfLifeDays);

        QJsonObject newBatch;
        newBatch["Количество"] = amount;
        newBatch["Дата изготовления"] = manufactureDate.toString("yyyy-MM-dd");
        newBatch["Истечение срока годности"] = expiry.toString("yyyy-MM-dd");

        QJsonArray batches = product["Партии"].toArray();
        batches.append(newBatch);
        product["Партии"] = batches;

        products[index] = product;
    }

    saveToFile();
    updateTable();

    // Можно добавить уведомление
    QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(QSystemTrayIcon::Information);
    trayIcon->showMessage("Автоматический заказ",
                          "Были автоматически заказаны продукты",
                          QSystemTrayIcon::Information,
                          5000);
}


void MainWindow::checkAndRemoveExpiredBatches()
{
    qDebug() << "[" << QTime::currentTime().toString() << "] Проверка просроченных партий...";

    bool changesMade = false;
    QDate currentDate = QDate::currentDate();
    QJsonArray expiredBatchesArchive;

    // Загрузка существующего архива
    QFile archiveFile("expired_archive.json");
    if (archiveFile.open(QIODevice::ReadOnly)) {
        expiredBatchesArchive = QJsonDocument::fromJson(archiveFile.readAll()).array();
        archiveFile.close();
    }

    // Создаем копию products для работы
    QJsonArray updatedProducts = products;

    // Проверка всех продуктов
    for (int i = 0; i < updatedProducts.size(); ++i) {
        QJsonObject product = updatedProducts[i].toObject();
        QJsonArray batches = product["Партии"].toArray();
        QJsonArray updatedBatches;

        for (const QJsonValue &batchVal : batches) {
            QJsonObject batch = batchVal.toObject();
            QDate expiry = QDate::fromString(batch["Истечение срока годности"].toString(), "yyyy-MM-dd");

            if (expiry > currentDate) {
                updatedBatches.append(batch);
            } else {
                // Формируем запись для архива
                QJsonObject expiredBatch = batch;
                expiredBatch["Название товара"] = product["Название"].toString();
                expiredBatch["Дата списания"] = currentDate.toString("yyyy-MM-dd");
                expiredBatch["UID товара"] = product["UID"].toInt();
                expiredBatchesArchive.append(expiredBatch);

                simulateRobotRemoval(product["Название"].toString(), expiry);
                changesMade = true;
                qDebug() << "Партия удалена:" << product["Название"].toString()
                         << "| Срок годности:" << expiry.toString("yyyy-MM-dd")
                         << "| Количество:" << batch["Количество"].toInt();
            }
        }

        // Обновляем партии только если есть изменения
        if (batches.size() != updatedBatches.size()) {
            product["Партии"] = updatedBatches;
            updatedProducts[i] = product;
        }
    }

    if (changesMade) {
        // Сохранение архива
        if (archiveFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            archiveFile.write(QJsonDocument(expiredBatchesArchive).toJson());
            archiveFile.close();
            qDebug() << "Архив списанных партий обновлен";
        }

        // Обновление основных данных
        products = updatedProducts; // Важно: обновляем основной массив
        saveToFile();
        updateTable();
        qDebug() << "Данные успешно обновлены. Удалено партий:"
                 << (changesMade ? "да" : "нет");
    } else {
        qDebug() << "Просроченных партий не обнаружено";
    }
}

