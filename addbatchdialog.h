#ifndef ADDBATCHDIALOG_H
#define ADDBATCHDIALOG_H
#include <QDialog>
#include <QDate>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QVBoxLayout>

class AddBatchDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddBatchDialog(QWidget *parent = nullptr);

    void setProductList(const QStringList &productNames);

    QString getProductName() const;
    double getAmount() const;
    QDate getExpiryDate() const;
    QDate getManufactureDate() const;

private:
    QComboBox *productComboBox;
    QDoubleSpinBox *amountSpinBox;
    QDateEdit *expiryDateEdit;
    QDialogButtonBox *buttonBox;
    QDateEdit *manufactureDateEdit;

};

#endif // ADDBATCHDIALOG_H
