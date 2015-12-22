#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w("10.162.1.110", 2323);
    //MainWindow w("localhost", 2323);
    w.show();

    return a.exec();
}
