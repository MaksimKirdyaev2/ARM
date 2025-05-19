#include "AddBatchDialog.h"
#include <QLabel>
#include <QCompleter>

AddBatchDialog::AddBatchDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Добавить партию");
    // Создаем выпадающий список для выбора продукта с возможностью редактирования
    productComboBox = new QComboBox(this);
    productComboBox->setEditable(true); // Можно вводить текст вручную
    amountSpinBox = new QDoubleSpinBox(this);
    amountSpinBox->setMinimum(0.01);
    amountSpinBox->setMaximum(1000000);
    amountSpinBox->setValue(1.0);
    manufactureDateEdit = new QDateEdit(QDate::currentDate(), this);
    manufactureDateEdit->setCalendarPopup(true);
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Название продукта:", this));
    layout->addWidget(productComboBox);
    layout->addWidget(new QLabel("Количество:", this));
    layout->addWidget(amountSpinBox);
    layout->addWidget(new QLabel("Дата изготовления:", this));
    layout->addWidget(manufactureDateEdit);
    layout->addWidget(buttonBox);
    setLayout(layout);
}

void AddBatchDialog::setProductList(const QStringList &productNames)
{
    productComboBox->clear();
    productComboBox->addItems(productNames);
    // Настройка автодополнения по списку
    QCompleter *completer = new QCompleter(productNames, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    productComboBox->setCompleter(completer);
}

QString AddBatchDialog::getProductName() const
{
    return productComboBox->currentText();
}

double AddBatchDialog::getAmount() const {
    return amountSpinBox->value();
}

QDate AddBatchDialog::getExpiryDate() const {
    return manufactureDateEdit->date();
}
QDate AddBatchDialog::getManufactureDate() const {
    return manufactureDateEdit->date();
}
