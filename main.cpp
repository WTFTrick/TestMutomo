#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w("192.168.10.4", 2323);
    w.show();

    return a.exec();
}
