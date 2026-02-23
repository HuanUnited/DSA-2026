#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Binary Tree Visualizer");

    MainWindow w;
    w.show();
    return app.exec();
}
