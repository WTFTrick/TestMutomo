#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QDebug>
#include <mainwindow.h>

#include "ui_settings.h"

class MainWindow;

namespace Ui {
class settings;
}

class settings : public QDialog, public Ui::settings
{
    Q_OBJECT

public:
    //Ui::settings *ui;
    explicit settings(QWidget *parent = 0);
    ~settings();
    void InterfaceSettings();
    void CreateConnections();

private slots:
    void ok_clicked();
    void cancel_clicked();

signals:
    void sendThreshold(quint16 threshold, quint16 yUpperBound);

private:
    QSettings qsettings;
};

#endif // SETTINGS_H
