#include <iostream>
#include <QApplication>
#include "ui/mainwindow.h"

int main(int argc, char **argv) {
    QApplication application(argc, argv);

    qRegisterMetaType<QFileInfo>("QFileInfo");

    MainWindow mainWindow;
    mainWindow.show();

    application.exec();

    return 0;
}