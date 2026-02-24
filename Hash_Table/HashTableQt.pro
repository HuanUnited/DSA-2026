QT += core gui widgets

TARGET = HashTableQt
TEMPLATE = app

CONFIG += c++17

SOURCES += \
    main_qt.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h \
    hashtable_qt.h

# Дополнительные флаги компилятора
QMAKE_CXXFLAGS += -std=c++17
