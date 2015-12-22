#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QWidget>
#include "mainwindow.h"

namespace Ui
{
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
    void on_pb_close_clicked();

private:
    Ui::Dialog *ui;
    QLineEdit *ip_lineEdit;
};

#endif // DIALOG_H
