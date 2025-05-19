#include "Menu.h"
#include "ui_Menu.h"
#include "Product.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include "CustomerWindow.h"

MainMenu::MainMenu(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainMenu)
{
    ui->setupUi(this);
    setupUI();
}

MainMenu::~MainMenu()
{
    delete ui;
}

void MainMenu::setupUI()
{
    setFixedSize(300, 350);  // ограничиваем размер окна7
    setWindowTitle("Авторизация");
    // Применяем стили только к этому окну
    setStyleSheet(R"(
        QWidget {
            background-color: #f0f4f8;
            font-family: Segoe UI, sans-serif;
            font-size: 14px;
        }
        QLabel {
            font-size: 20px;
            font-weight: bold;
            color: #333;
        }
        QLineEdit {
            border: 1px solid #ccc;
            border-radius: 8px;
            padding: 8px;
            background-color: #fff;
        }
        QComboBox {
            border: 1px solid #ccc;
            border-radius: 8px;
            padding: 6px;
            background-color: #fff;
        }
        QPushButton {
            background-color: #0078d7;
            color: white;
            padding: 10px;
            border: none;
            border-radius: 8px;
        }
        QPushButton:hover {
            background-color: #005a9e;
        }
    )");
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(15);
    QLabel *title = new QLabel("Авторизация", this);
    title->setAlignment(Qt::AlignCenter);
    lineEditLogin = new QLineEdit(this);
    lineEditLogin->setPlaceholderText("Логин");
    lineEditPassword = new QLineEdit(this);
    lineEditPassword->setPlaceholderText("Пароль");
    lineEditPassword->setEchoMode(QLineEdit::Password);
    comboRole = new QComboBox(this);
    comboRole->addItem("Покупатель");
    comboRole->addItem("Админ");
    lineEditAdminCode = new QLineEdit(this);
    lineEditAdminCode->setPlaceholderText("Код администратора");
    lineEditAdminCode->setEchoMode(QLineEdit::Password);
    lineEditAdminCode->setVisible(false);
    btnLogin = new QPushButton("Войти", this);
    btnRegister = new QPushButton("Зарегистрироваться", this);
    layout->addWidget(title);
    layout->addWidget(lineEditLogin);
    layout->addWidget(lineEditPassword);
    layout->addWidget(comboRole);
    layout->addWidget(lineEditAdminCode);
    layout->addWidget(btnLogin);
    layout->addWidget(btnRegister);
    connect(comboRole, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainMenu::onRoleChanged);
    connect(btnLogin, &QPushButton::clicked, this, &MainMenu::onLoginClicked);
    connect(btnRegister, &QPushButton::clicked, this, &MainMenu::onRegisterClicked);
}

void MainMenu::onRoleChanged(int index)
{
    lineEditAdminCode->setVisible(index == 1);
}

void MainMenu::onLoginClicked()
{
    QString login = lineEditLogin->text();
    QString password = lineEditPassword->text();
    QString role = comboRole->currentText();
    QString code = lineEditAdminCode->text();
    if (role == "Админ" && code != "1234") {
        QMessageBox::warning(this, "Ошибка", "Неверный код администратора");
        return;
    }
    if (validateLogin(login, password, role)) {
        QMessageBox::information(this, "Успешно", "Добро пожаловать, " + login);
        if (role == "Админ") {
            MainWindow *productWindow = new MainWindow(this);
            productWindow->show();
        } else {
            CustomerWindow *customerWindow = new CustomerWindow(nullptr);
            customerWindow->setAttribute(Qt::WA_DeleteOnClose);  // чтобы автоматически удалялось
            customerWindow->show();
        }
        this->hide();
    } else {
        QMessageBox::warning(this, "Ошибка", "Неверный логин или пароль");
    }
}

void MainMenu::onRegisterClicked()
{
    QString login = lineEditLogin->text();
    QString password = lineEditPassword->text();
    QString role = comboRole->currentText();
    QString code = lineEditAdminCode->text();
    if (role == "Админ" && code != "1234") {
        QMessageBox::warning(this, "Ошибка", "Неверный код администратора");
        return;
    }
    if (registerUser(login, password, role)) {
        QMessageBox::information(this, "Успешно", "Регистрация завершена");
        onLoginClicked();
    } else {
        QMessageBox::warning(this, "Ошибка", "Такой пользователь уже существует");
    }
}

bool MainMenu::validateLogin(const QString &login, const QString &password, const QString &role)
{
    QFile file("users.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QStringList parts = in.readLine().split(";");
        if (parts.size() == 3 && parts[0] == login && parts[1] == password && parts[2] == role)
            return true;
    }
    return false;
}

bool MainMenu::registerUser(const QString &login, const QString &password, const QString &role)
{
    QString filePath = "users.txt";
    QFile file(filePath);
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return false;
        QTextStream in(&file);
        while (!in.atEnd()) {
            QStringList parts = in.readLine().split(";");
            if (parts.size() >= 1 && parts[0] == login) {
                file.close();
                return false;
            }
        }
        file.close();
    }
    if (!file.open(QIODevice::Append | QIODevice::Text))
        return false;
    QTextStream out(&file);
    out << login << ";" << password << ";" << role << "\n";
    file.close();
    return true;
}


