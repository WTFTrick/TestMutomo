#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile styleF;

    styleF.setFileName("/home/kopylov/projects/TMutomo-develope/StyleSheet.qss");
    styleF.open(QFile::ReadOnly);
    QString qssStr = styleF.readAll();

    //qApp->setStyleSheet(qssStr);

    MainWindow w;
    w.show();

    return a.exec();
}
