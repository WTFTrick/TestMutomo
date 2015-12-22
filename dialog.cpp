#include "dialog.h"
#include "ui_dialog.h"
#include <QtNetwork>
#include <QtGui>

Dialog::Dialog(QWidget *parent) : QDialog(parent), ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->ip_lineEdit->setInputMask("000.000.000.000;_");

    QFile file("/home/kopylov/projects/TMutomo-develope/IPAdress.txt");
    if(file.open(QIODevice::ReadOnly |QIODevice::Text))
        {
            while(!file.atEnd())
            {
                //читаем строку
                QString str = file.readLine();
                //Делим строку на слова разделенные пробелом
                QStringList lst = str.split(" ");
                  // выводим первых три слова
                qDebug() << str;


                QListWidgetItem *pIt;
                pIt = new QListWidgetItem(str, ui->listWidget);
            }

        }
        else
        {
            qDebug()<< "Не смог открыть файл";
        }
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

   QFile file("/home/kopylov/projects/TMutomo-develope/IPAdress.txt");
   if (file.open(QIODevice::Append))
   {
      file.write(text);
   }
   file.close();
}
