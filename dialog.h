#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QWidget>
#include <QListWidget>

#include <qfile.h>

namespace Ui
{
    class IPDialog;
}

class IPDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IPDialog(QWidget *parent = 0);
    ~IPDialog();

private slots:
    void on_pb_close_clicked();
    void on_pb_add_clicked();
    void ReadFromFile();
    void on_pb_del_clicked();
    void on_pb_connect_clicked();

signals:
    void sendData(QString str);

private:
    Ui::IPDialog *ui;
    QLineEdit *ip_lineEdit;
    QFile ipFile;


};

#endif // DIALOG_H
