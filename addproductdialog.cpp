#include "AddProductDialog.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QFileDialog>
#include <QFileInfo>

AddProductDialog::AddProductDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Добавить продукт");
    QLabel *nameLabel = new QLabel("Название:", this);
    nameLineEdit = new QLineEdit(this);
    QLabel *shelfLifeLabel = new QLabel("Срок хранения (в днях):", this);
    shelfLifeSpinBox = new QSpinBox(this);
    shelfLifeSpinBox->setMinimum(1);
    shelfLifeSpinBox->setMaximum(3650); // до 10 лет
    shelfLifeSpinBox->setValue(30);     // значение по умолчанию
    QLabel *priceLabel = new QLabel("Цена:", this);
    priceSpinBox = new QDoubleSpinBox(this);
    priceSpinBox->setMinimum(0);
    priceSpinBox->setMaximum(1000000);
    priceSpinBox->setDecimals(2);
    priceSpinBox->setPrefix("₽ ");
    priceSpinBox->setValue(0.0);
    QPushButton *imageButton = new QPushButton("Выбрать изображение", this);
    connect(imageButton, &QPushButton::clicked, this, [=]() {
        QString fileName = QFileDialog::getOpenFileName(this, "Выберите изображение", "", "Изображения (*.png *.jpg *.jpeg *.bmp)");
        if (!fileName.isEmpty()) {
            imagePath = fileName;
            if (!fileName.isEmpty()) {
                imagePath = fileName;
                imageButton->setText("✔ " + QFileInfo(fileName).fileName());
            }
        }
    });
    QLabel *minThresholdLabel = new QLabel("Мин. порог для заказа:", this);
    minThresholdSpinBox = new QSpinBox(this);
    minThresholdSpinBox->setMinimum(0);
    minThresholdSpinBox->setMaximum(100000);
    minThresholdSpinBox->setValue(10);  // Значение по умолчанию
    QLabel *optimalStockLabel = new QLabel("Оптимальное количество:", this);
    optimalStockSpinBox = new QSpinBox(this);
    optimalStockSpinBox->setMinimum(0);
    optimalStockSpinBox->setMaximum(100000);
    optimalStockSpinBox->setValue(100);
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(nameLabel);
    layout->addWidget(nameLineEdit);
    layout->addWidget(shelfLifeLabel);
    layout->addWidget(shelfLifeSpinBox);
    layout->addWidget(priceLabel);
    layout->addWidget(priceSpinBox);
    layout->addWidget(imageButton);
    layout->addWidget(minThresholdLabel);
    layout->addWidget(minThresholdSpinBox);
    layout->addWidget(optimalStockLabel);
    layout->addWidget(optimalStockSpinBox);
    layout->addWidget(buttonBox);  // Кнопки всегда внизу
    setLayout(layout);
}

QString AddProductDialog::getName() const {
    return nameLineEdit->text();
}

int AddProductDialog::getShelfLifeDays() const {
    return shelfLifeSpinBox->value();
}

double AddProductDialog::getPrice() const {
    return priceSpinBox->value();
}
QString AddProductDialog::getImagePath() const {
    return imagePath;
}
int AddProductDialog::getMinStockThreshold() const {
    return minThresholdSpinBox->value();
}

int AddProductDialog::getOptimalStockLevel() const {
    return optimalStockSpinBox->value();
}
