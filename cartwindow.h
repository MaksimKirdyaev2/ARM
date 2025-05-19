#include <QMainWindow>
#include <QMap>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

struct ProductInfo {
    double price;
    QString imagePath;
};

class CartWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CartWindow(const QMap<QString, int> &cartData, QWidget *parent = nullptr);
    void updateCart(const QMap<QString, int>& newCartData);

signals:
    void orderCompleted();

private slots:
    void handleCheckout();

private:
    QMap<QString, int> cartData;
    QMap<QString, ProductInfo> productDetails;
    QLabel *totalLabel;
    QScrollArea *scrollArea;
    QWidget *productsContainer;
    QVBoxLayout *productsLayout;
    QPushButton *checkoutButton;
};
