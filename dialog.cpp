#include "dialog.h"
#include "ui_dialog.h"
#include <QtNetwork>
#include <QtGui>

IPDialog::IPDialog(QWidget *parent) : QDialog(parent), ui(new Ui::IPDialog)
{
    ui->setupUi(this);
    ui->ip_lineEdit->setInputMask("000.000.000.000;_");

    ReadFromFile();
}

IPDialog::~IPDialog()
{
    delete ui;
}

void IPDialog::on_pb_close_clicked()
{
    close();
}

void IPDialog::on_pb_add_clicked()
{
   QString text = ui->ip_lineEdit->text();

   QFile ipFile("/home/kopylov/projects/TMutomo-develope/IPAdress.txt");
   if(ipFile.open(QIODevice::WriteOnly |QIODevice::Text | QIODevice::Append))
   {
   QTextStream out(&ipFile);
   out << text << "\n";
   QListWidgetItem *pIt;
   pIt = new QListWidgetItem(text, ui->listWidget);
   }
   ipFile.close();
}

void IPDialog::ReadFromFile()
{
        QString tempstr;
        QStringList list;
        QTextStream stream;
        QFile ipFile("/home/kopylov/projects/TMutomo-develope/IPAdress.txt");
        stream.setDevice(&ipFile);

        if(ipFile.open(QIODevice::ReadWrite|QIODevice::Text))
        {
        while(!stream.atEnd())
        {
            tempstr = stream.readLine();
            list.append(tempstr);
            QListWidgetItem *pIt;
            pIt = new QListWidgetItem(tempstr, ui->listWidget);
        }
        }
        ipFile.close();
}

void IPDialog::on_pb_del_clicked()
{
    QListWidgetItem *it = ui->listWidget->item(ui->listWidget->currentRow());
    QString st1 = ui->listWidget->item(ui->listWidget->currentRow())->text();

    QFile ipFile("/home/kopylov/projects/TMutomo-develope/IPAdress.txt");
    if(ipFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QString s;
        QTextStream t(&ipFile);
        while(!t.atEnd())
        {
            QString line = t.readLine();
            if(!line.contains(st1))
                s.append(line + "\n");
        }
        ipFile.resize(0);
        t << s;
        ipFile.close();
    }
    delete it;
}

void IPDialog::on_pb_connect_clicked()
{
    QString ip = "0.0.0.0";
    ip = ui->listWidget->item(ui->listWidget->currentRow())->text();
    qDebug() << "Modal dialog ip:" << ip;
    emit sendData(ip);
}
