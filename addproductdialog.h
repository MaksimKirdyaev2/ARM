#ifndef ADDPRODUCTDIALOG_H
#define ADDPRODUCTDIALOG_H
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QSpinBox>

class AddProductDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddProductDialog(QWidget *parent = nullptr);

    QString getName() const;
    QString getProductName() const;
    int getShelfLifeDays() const;
    double getPrice() const;
    QString getImagePath() const;
    int getMinStockThreshold() const;
    int getOptimalStockLevel() const;


private:
    QLineEdit *nameLineEdit;
    QDialogButtonBox *buttonBox;
    QSpinBox *shelfLifeSpinBox;
    QDoubleSpinBox *priceSpinBox;
    QString imagePath;
    QSpinBox *minThresholdSpinBox;
    QSpinBox *optimalStockSpinBox;


};

#endif // ADDPRODUCTDIALOG_H
