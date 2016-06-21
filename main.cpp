#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    Qt::AA_EnableHighDpiScaling;
    QApplication a(argc, argv);
    QApplication::setStyle("fusion");

#ifdef ANDROID

    QFile f(":qstyle/StyleSheet.qss");
    if (f.exists())
    {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }
    else
    {
        qDebug() << "Cannot open stylesheet file";
    }

#endif


    MainWindow w;
    w.show();

    return a.exec();
}
