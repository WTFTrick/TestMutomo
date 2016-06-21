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
    Ui::settings *ui;
    explicit settings(QWidget *parent = 0);
    ~settings();
    void InterfaceSettings();
    void CreateConnections();

private slots:
    void ok_clicked();
    void cancel_clicked();
    void checkboxDetec_checked();
    void checkboxDevice_checked();

signals:
    void sendThreshold(quint16 threshold, quint16 yUpperBound);
    void checkboxDetec_grid(bool checked);
    void checkboxDevice_grid(bool checked);
};

#endif // SETTINGS_H
