#ifndef MENU_H
#define MENU_H
#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui { class MainMenu; }
QT_END_NAMESPACE

class MainMenu : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainMenu(QWidget *parent = nullptr);
    ~MainMenu();

private slots:
    void onRoleChanged(int index);
    void onLoginClicked();
    void onRegisterClicked();

private:
    Ui::MainMenu *ui;
    QWidget *centralWidget;
    QLineEdit *lineEditLogin;
    QLineEdit *lineEditPassword;
    QLineEdit *lineEditAdminCode;
    QComboBox *comboRole;
    QPushButton *btnLogin;
    QPushButton *btnRegister;
    void setupUI();
    bool validateLogin(const QString &login, const QString &password, const QString &role);
    bool registerUser(const QString &login, const QString &password, const QString &role);
};

#endif // MENU_H
