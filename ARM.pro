QT += core gui widgets
CONFIG += c++17
TEMPLATE = app
TARGET = ARM

# Исходные файлы
SOURCES += \
    addbatchdialog.cpp \
    addproductdialog.cpp \
    cartwindow.cpp \
    customerwindow.cpp \
    main.cpp \
    Menu.cpp \
    Product.cpp \
    orderconfirmationwindow.cpp \
    orderdialog.cpp

# Заголовочные файлы
HEADERS += \
    Menu.h \
    Product.h \
    addbatchdialog.h \
    addproductdialog.h \
    cartwindow.h \
    customerwindow.h \
    orderconfirmationwindow.h \
    orderdialog.h

# UI-файлы
FORMS += \
    Menu.ui \
    Product.ui \
    addbatchdialog.ui \
    addproductdialog.ui \
    cartwindow.ui \
    customerwindow.ui \
    orderconfirmationwindow.ui \
    orderdialog.ui
