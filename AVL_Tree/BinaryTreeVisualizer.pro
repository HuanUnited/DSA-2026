QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET   = BinaryTreeVisualizer
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    treewidget.cpp \
    treetests.cpp \
    binarytree.cpp \
    binarysearchtree.cpp \
    avltree.cpp

HEADERS += \
    mainwindow.h \
    treewidget.h \
    treetests.h \
    binarytree.h \
    binarysearchtree.h \
    avltree.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
