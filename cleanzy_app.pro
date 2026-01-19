QT       += core gui sql charts printsupport serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    arduino.cpp \
    client.cpp \
    connection.cpp \
    equipement.cpp \
    mailing.cpp \
    main.cpp \
    mainwindow.cpp \
    qrcode.cpp \
    smsmanager.cpp

HEADERS += \
    arduino.h \
    client.h \
    connection.h \
    equipement.h \
    mailing.h \
    mainwindow.h \
    qrcode.h \
    qrcode.hpp \
    smsmanager.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    rescource.qrc
