#include "settings.h"
#include "ui_settings.h"

settings::settings(QWidget *parent) : QDialog(parent), ui(new Ui::settings)
{
    ui->setupUi(this);
    InterfaceSettings();
    CreateConnections();
}

settings::~settings()
{
    delete ui;
    close();
}

void settings::InterfaceSettings()
{
    ui->label->setAlignment(Qt::AlignCenter); //Set label at center
    ui->le_value->setValidator( new QIntValidator(0, 100, this) );
    ui->le_value->setText("100");
    ui->pb_cancel->setFocus();
    //setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
}

void settings::CreateConnections()
{
    connect(ui->pb_OK, SIGNAL(clicked(bool)), this, SLOT(ok_clicked()));
    connect(ui->pb_cancel, SIGNAL(clicked(bool)), this, SLOT(cancel_clicked()));
}

void settings::ok_clicked()
{
    if (ui->le_value->text() == "")
    {
        qDebug() << "Edit line is empty! Enter some value!";
    }
    else
    {
        QString threshold_val;
        threshold_val = ui->le_value->text();
        quint8 threshold = threshold_val.toInt();
        emit sendThreshold(threshold);
        close();
    }
}

void settings::cancel_clicked()
{
    close();
}

