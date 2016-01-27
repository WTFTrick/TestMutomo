#include "dialog.h"
#include "ui_dialog.h"
#include <QtNetwork>
#include <QtGui>

IPDialog::IPDialog(QWidget *parent) : QDialog(parent), ui(new Ui::IPDialog)
{
    ui->setupUi(this);
    QRegExp rx("\\d+");
    ui->ip_lineEdit->setValidator( new QIntValidator(0, 255, this) );
    ui->ip_lineEdit->setInputMask("000.000.000.000;_");
    ipFile.setFileName("./IPAdress.txt");
    ReadFromFile();
    //ui->listWidget->clear();
    if (ui->listWidget->count() == 0)
    {
        ui->pb_connect->setDisabled(true);
        ui->pb_del->setDisabled(true);
    }
    else
    {
        ui->listWidget->setCurrentRow(0);
    }
}

IPDialog::~IPDialog()
{
    ipFile.close();
    delete ui;
}

void IPDialog::on_pb_close_clicked()
{
    ipFile.close();
    close();
}

void IPDialog::on_pb_add_clicked()
{
    QString text = ui->ip_lineEdit->text();

    ui->ip_lineEdit->clear();
    if(ipFile.open(QIODevice::WriteOnly |QIODevice::Text | QIODevice::Append))
    {
        if (text == "...")
        {
            ui->listWidget->setCurrentRow(0);
        }
        else
        {
            QTextStream out(&ipFile);
            out << text << "\n";
            QListWidgetItem *pIt;
            pIt = new QListWidgetItem(text, ui->listWidget);
            ui->listWidget->setCurrentItem(pIt);
        }
    }
    ipFile.close();

    if (ui->listWidget->count() >= 1)
    {
        ui->pb_connect->setEnabled(true);
        ui->pb_del->setEnabled(true);
    }
}

void IPDialog::ReadFromFile()
{
        QString tempstr;
        QStringList list;
        QTextStream stream;
        stream.setDevice(&ipFile);

        if(ipFile.open(QIODevice::ReadWrite|QIODevice::Text))
        {
            /*QString s;
            QTextStream t(&ipFile);
            while(!t.atEnd())
            {
                QString line = t.readLine();
                if(!line.contains("..."))
                    s.append(line + "\n");
            }
            ipFile.resize(0);
            t << s;*/

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

    if (ui->listWidget->count() < 0)
    {
        ui->pb_connect->setDisabled(true);
    }
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

    if (ui->listWidget->count() == 0)
    {
        ui->pb_connect->setDisabled(true);
        ui->pb_del->setDisabled(true);
    }

}

void IPDialog::on_pb_connect_clicked()
{

    QString ip = "0.0.0.0";
    ip = ui->listWidget->item(ui->listWidget->currentRow())->text();
    qDebug() << "Modal dialog ip:" << ip;
    emit sendData(ip);
    close();


}
