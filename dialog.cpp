#include "dialog.h"
#include "ui_dialog.h"
#include <QtNetwork>
#include <QtGui>

Dialog::Dialog(QWidget *parent) : QDialog(parent), ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->ip_lineEdit->setInputMask("000.000.000.000;_");

    ReadFromFile();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pb_close_clicked()
{
    close();
}

void Dialog::on_pb_add_clicked()
{
   QString text = ui->ip_lineEdit->text();

   QFile file1("/home/kopylov/projects/TMutomo-develope/IPAdress.txt");
   if(file1.open(QIODevice::WriteOnly |QIODevice::Text | QIODevice::Append))
   {
   QTextStream out(&file1);
   out << text << "\n";
   QListWidgetItem *pIt;
   pIt = new QListWidgetItem(text, ui->listWidget);
   }
   file1.close();
}

void Dialog::ReadFromFile()
{
        QString tempstr;
        QStringList list;
        QTextStream stream;
        QFile fileProtocol("/home/kopylov/projects/TMutomo-develope/IPAdress.txt");
        stream.setDevice(&fileProtocol);

        if(fileProtocol.open(QIODevice::ReadWrite|QIODevice::Text))
        {
            qDebug()<<"Мы зашли сюда";
        while(!stream.atEnd())
        {
            tempstr = stream.readLine();
            list.append(tempstr);
            QListWidgetItem *pIt;
            pIt = new QListWidgetItem(tempstr, ui->listWidget);
        }
        }
        fileProtocol.close();
}
