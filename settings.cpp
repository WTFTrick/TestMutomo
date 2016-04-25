#include "settings.h"
#include "ui_settings.h"

settings::settings(QWidget *parent) : QDialog(parent), ui(new Ui::settings), qsettings("settings.conf", QSettings::NativeFormat)
{
    ui->setupUi(this);
    InterfaceSettings();
    CreateConnections();
}

settings::~settings()
{
    close();
    delete ui;
}

void settings::InterfaceSettings()
{
    ui->label_threshold->setAlignment(Qt::AlignCenter);     //Set label at center
    ui->label_yUpperBound->setAlignment(Qt::AlignCenter);

    ui->le_thresholdValue->setValidator( new QIntValidator(0, 100, this)    );
    ui->le_yUpperBound->setValidator(   new QIntValidator(0, 500, this)     );

    if ( QFile::exists("settings.conf") )
    {
        ui->le_thresholdValue->setText( qsettings.value("settings/threshold").toString()    );
        ui->le_yUpperBound->setText(    qsettings.value("settings/yUpperBound").toString()  );
    }
    else
    {
        ui->le_thresholdValue->setText("100");
        ui->le_yUpperBound->setText("150");
    }

    ui->le_thresholdValue->setFocus();
}

void settings::CreateConnections()
{
    connect(ui->pb_OK, SIGNAL(clicked(bool)), this, SLOT(ok_clicked()));
    connect(ui->pb_cancel, SIGNAL(clicked(bool)), this, SLOT(cancel_clicked()));
}

void settings::ok_clicked()
{
    if ((ui->le_thresholdValue->text() == "") || (ui->le_yUpperBound->text() == "") )
        qDebug() << "Edit line is empty! Enter some value!";

    else
    {
        quint16 threshold= ui->le_thresholdValue->text().toInt();
        quint16 yUpperBound = ui->le_yUpperBound->text().toInt();
        emit sendThreshold(threshold, yUpperBound);
        close();
    }
}

void settings::cancel_clicked()
{
    close();
}

