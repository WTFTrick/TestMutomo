#include "settings.h"
#include "ui_settings.h"

settings::settings(QWidget *parent) : QDialog(parent), /*ui(new Ui::settings),*/
    qsettings("settings.conf", QSettings::NativeFormat)
{
    setupUi(this);
    InterfaceSettings();
    CreateConnections();
}

settings::~settings()
{
    close();
    //delete ui;
}

void settings::InterfaceSettings()
{
    label_threshold->setAlignment(Qt::AlignCenter);     //Set label at center
    label_yUpperBound->setAlignment(Qt::AlignCenter);


    le_thresholdValue->setValidator( new QIntValidator(0, 350, this)    );
    le_yUpperBound->setValidator(   new QIntValidator(0, 500, this)     );

    le_thresholdValue->setFocus();
}

void settings::CreateConnections()
{
    connect(pb_OK, SIGNAL(clicked(bool)), this, SLOT(ok_clicked()));
    connect(pb_cancel, SIGNAL(clicked(bool)), this, SLOT(cancel_clicked()));
}

void settings::ok_clicked()
{
    if ((le_thresholdValue->text() == "") || (le_yUpperBound->text() == "") )
        qDebug() << "Edit line is empty! Enter some value!";

    else
    {
        quint16 threshold= le_thresholdValue->text().toInt();
        quint16 yUpperBound = le_yUpperBound->text().toInt();
        emit sendThreshold(threshold, yUpperBound);
        accept();
    }
}

void settings::cancel_clicked()
{
    reject();
}

