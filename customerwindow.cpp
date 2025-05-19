#include "CustomerWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QHeaderView>
#include <QLabel>
#include <QScrollArea>
#include <QGridLayout>
#include <QDebug>
#include "Product.h"

CustomerWindow::CustomerWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    // Заголовок
    QLabel *titleLabel = new QLabel("Выберите товары:");
    titleLabel->setProperty("title", true);
    mainLayout->addWidget(titleLabel);
    // Прокручиваемая область с товарами
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet(R"(
        QScrollArea {
            border: 2px solid #B0BEC5;
            border-radius: 12px;
            background: #FFFFFF;
            margin: 10px;
        }

        /* Вертикальный скроллбар */
        QScrollBar:vertical {
            border: 1px solid #b0bec5;
            background: #ffffff;
            width: 12px;
            margin: 2px;
            border-radius: 4px;
        }

        /* Ползунок скроллбара */
        QScrollBar::handle:vertical {
            background: #607D8B;
            min-height: 30px;
            border: 1px solid #455A64;
            border-radius: 4px;
        }

        /* Стрелки скроллбара */
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            border: none;
            background: none;
        }

        /* Область за ползунком */
        QScrollBar::add-page:vertical,
        QScrollBar::sub-page:vertical {
            background: none;
        }
    )");
    // Затемнение фона с плитками
    QWidget *productsContainer = new QWidget(scrollArea);
    productsContainer->setStyleSheet(R"(
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
            stop:0 #e0e0e0,
            stop:1 #f5f5f5);
        padding: 5px;
    )");
    QGridLayout *productsLayout = new QGridLayout(productsContainer);
    productsLayout->setAlignment(Qt::AlignTop);
    scrollArea->setWidget(productsContainer);
    mainLayout->addWidget(scrollArea);
    // Кнопка "Корзина"
    QPushButton *btnViewCart = new QPushButton("Корзина", this);
    mainLayout->addWidget(btnViewCart);
    connect(btnViewCart, &QPushButton::clicked, this, &CustomerWindow::openCartWindow);
    // Глобальные стили
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f5f5f5;
            font-family: 'Segoe UI', Arial, sans-serif;
        }
        QPushButton {
            background-color: #2196F3;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-size: 14px;
            min-width: 100px;
        }
        QPushButton:hover {
            background-color: #42A5F5;
        }
        QPushButton:pressed {
            background-color: #1976D2;
        }
        QSpinBox {
            padding: 5px;
            border: 2px solid #B0BEC5;
            border-radius: 8px;
            background: white;
            color: #333;
        }
        QLabel {
            font-size: 14px;
            color: #424242;
        }
        QLabel[title="true"] {
            font-size: 16px;
            font-weight: 500;
            color: #212121;
        }
        QFrame, QWidget, QLabel {
            border: none;
        }
    )");
    // Загрузка карточек товаров
    loadProductsFromJson();
    connect(this, &CustomerWindow::cartUpdated, cartWin, &CartWindow::updateCart, Qt::QueuedConnection);
    // Фиксируем размер окна
    setFixedSize(820, 600);
}

CustomerWindow::~CustomerWindow() {}

void CustomerWindow::loadProductsFromJson()
{
    QString filePath = "products.json";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл продуктов");
        return;
    }
    QByteArray data = file.readAll();
    file.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        QMessageBox::warning(this, "Ошибка", "Неверный формат JSON");
        return;
    }
    QJsonArray array = doc.array();
    QWidget *productsContainer = findChild<QScrollArea*>()->widget();
    productsContainer->setStyleSheet(R"(
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
            stop:0 #f8f9fa,
            stop:1 #e9ecef);
    )");
    QGridLayout *productsLayout = qobject_cast<QGridLayout*>(productsContainer->layout());
    productsLayout->setHorizontalSpacing(10);
    productsLayout->setVerticalSpacing(15);
    const int columns = 2;
    const int cardWidth = 370;
    const int cardHeight = 400;
    int windowWidth = columns * cardWidth + (columns + 1) * 10 + 20;
    this->setFixedWidth(windowWidth);
    for (int i = 0; i < array.size(); ++i) {
        QJsonObject obj = array[i].toObject();
        QString name = obj.value("Название").toString();
        double price = obj.value("Цена").toDouble(0);
        QString priceStr = QString::number(price, 'f', 2);
        QString displayName = QString("%1 - %2 руб.").arg(name).arg(priceStr);
        QString imagePath = obj.value("Изображение").toString();
        QJsonArray batches = obj.value("Партии").toArray();
        double totalQuantity = calculateTotalAmount(batches);
        QWidget *productCard = new QWidget(productsContainer);
        productCard->setFixedSize(cardWidth, cardHeight);
        productCard->setStyleSheet(R"(
            background: white;
            border-radius: 10px;
            padding: 12px;
            border: 2px solid #B0BEC5;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        )");
        QVBoxLayout *cardLayout = new QVBoxLayout(productCard);
        cardLayout->setAlignment(Qt::AlignHCenter);
        cardLayout->setContentsMargins(8, 8, 8, 8);
        cardLayout->setSpacing(4);
        QWidget *imageFrame = new QWidget(productCard);
        imageFrame->setFixedSize(353, 280);
        imageFrame->setStyleSheet("border: none;");
        QHBoxLayout *frameLayout = new QHBoxLayout(imageFrame);
        frameLayout->setContentsMargins(0, 0, 0, 0);
        frameLayout->setAlignment(Qt::AlignCenter);
        QLabel *imageLabel = new QLabel(imageFrame);
        QPixmap pixmap(imagePath);
        if (pixmap.isNull()) pixmap = QPixmap("images/default.png");
        imageLabel->setPixmap(pixmap.scaled(260, 260, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        imageLabel->setAlignment(Qt::AlignCenter);
        frameLayout->addWidget(imageLabel);
        QLabel *nameLabel = new QLabel(displayName, productCard);
        nameLabel->setStyleSheet(R"(
            font-weight: bold;
            font-size: 18px;
            margin: 10px 0;
            padding: 0 10px;
            qproperty-wordWrap: true;
            border: none;
        )");
        nameLabel->setFixedHeight(60);
        nameLabel->setAlignment(Qt::AlignCenter);
        QWidget *controlsContainer = new QWidget(productCard);
        QHBoxLayout *controlsLayout = new QHBoxLayout(controlsContainer);
        controlsLayout->setContentsMargins(0, 0, 0, 0);
        controlsLayout->setSpacing(10);
        const int controlWidth = 60;
        const int controlHeight = 40;
        QPushButton *removeButton = new QPushButton("-", controlsContainer);
        removeButton->setFixedSize(controlWidth, controlHeight);
        removeButton->setStyleSheet(R"(
            QPushButton {
                background-color: #f44336;
                color: white;
                border: none;
                border-radius: 5px;
                font-size: 16px;
            }
            QPushButton:hover { background-color: #d32f2f; }
        )");
        QLabel *quantityLabel = new QLabel("1", controlsContainer);
        quantityLabel->setFixedSize(controlWidth + 20, controlHeight);
        quantityLabel->setStyleSheet(R"(
            font-size: 14px;
            color: #333;
            background: #f5f5f5;
            border-radius: 5px;
            padding: 0 5px;
        )");
        quantityLabel->setAlignment(Qt::AlignCenter);
        QPushButton *addButton = new QPushButton("+", controlsContainer);
        addButton->setFixedSize(controlWidth, controlHeight);
        addButton->setStyleSheet(R"(
            QPushButton {
                background-color: #4CAF50;
                color: white;
                border: none;
                border-radius: 5px;
                font-size: 16px;
            }
            QPushButton:hover { background-color: #45a049; }
        )");
        QPushButton *mainAddButton = new QPushButton("Добавить", controlsContainer);
        mainAddButton->setFixedHeight(controlHeight);
        mainAddButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        mainAddButton->setStyleSheet(R"(
            QPushButton {
                background-color: #4CAF50;
                color: white;
                border: none;
                border-radius: 5px;
                font-size: 14px;
            }
            QPushButton:hover { background-color: #45a049; }
        )");
        if (totalQuantity > 0) {
            controlsLayout->addWidget(mainAddButton);
        } else {
            QLabel *outOfStockLabel = new QLabel("Нет в наличии", controlsContainer);
            outOfStockLabel->setAlignment(Qt::AlignCenter);
            outOfStockLabel->setStyleSheet(R"(
        font-size: 14px;
        color: red;
        background: #fff3f3;
        border: 1px solid #e57373;
        border-radius: 5px;
        padding: 10px;
    )");
            controlsLayout->addWidget(outOfStockLabel);
        }
        cardLayout->addWidget(imageFrame);
        cardLayout->addWidget(nameLabel);
        cardLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
        cardLayout->addWidget(controlsContainer);
        connect(mainAddButton, &QPushButton::clicked, this, [=]() {
            mainAddButton->hide();
            controlsLayout->removeWidget(mainAddButton);
            controlsLayout->addWidget(removeButton);
            controlsLayout->addWidget(quantityLabel);
            controlsLayout->addWidget(addButton);
            removeButton->show();
            quantityLabel->show();
            addButton->show();
            addToCart(name, 1);
        });
        connect(addButton, &QPushButton::clicked, this, [=]() {
            int current = quantityLabel->text().toInt() + 1;
            quantityLabel->setText(QString::number(current));
            addToCart(name, 1);
        });
        connect(removeButton, &QPushButton::clicked, this, [=]() {
            int current = quantityLabel->text().toInt();
            if (current > 1) {
                quantityLabel->setText(QString::number(current - 1));
                removeFromCart(name, 1);
            } else {
                controlsLayout->removeWidget(removeButton);
                controlsLayout->removeWidget(quantityLabel);
                controlsLayout->removeWidget(addButton);
                controlsLayout->addWidget(mainAddButton);
                removeButton->hide();
                quantityLabel->hide();
                addButton->hide();
                mainAddButton->show();
                removeFromCart(name, 1);
            }
        });
        productsLayout->addWidget(productCard, i / columns, i % columns, Qt::AlignTop);
    }
}


void CustomerWindow::removeFromCart(const QString &productName, int quantity)
{
    if (cartData.contains(productName)) {
        cartData[productName] -= quantity;
        if (cartData[productName] <= 0)
            cartData.remove(productName);
    }
    emit cartUpdated(cartData);
}

void CustomerWindow::addToCart(const QString &productName, int quantity)
{
    cartData[productName] += quantity;
    emit cartUpdated(cartData);
}

void CustomerWindow::openCartWindow()
{
    if (cartWin && cartWin->isVisible()) {
        cartWin->raise();
        cartWin->activateWindow();
        return;
    }
    cartWin = new CartWindow(cartData, nullptr);
    cartWin->setAttribute(Qt::WA_DeleteOnClose);
    cartWin->setWindowModality(Qt::NonModal);
    cartWin->setWindowFlag(Qt::Window);
    cartWin->setWindowTitle("Корзина");
    cartWin->resize(400, 300);
    connect(cartWin, &QObject::destroyed, this, [this]() {
        cartWin = nullptr;
    });
    connect(this, &CustomerWindow::cartUpdated, cartWin, &CartWindow::updateCart);
    cartWin->show();
}
double CustomerWindow::calculateTotalAmount(QJsonArray &batches) {
    double total = 0;
    for (const QJsonValue &batch : batches) {
        QJsonObject obj = batch.toObject();
        total += obj.value("Количество").toDouble();
    }
    return total;
}

