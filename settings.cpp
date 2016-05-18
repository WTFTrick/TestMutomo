#include "settings.h"
#include "ui_settings.h"

settings::settings(QWidget *parent) :
    QDialog(parent),
    qsettings("settings.conf", QSettings::NativeFormat)
  /*, ui(new Ui::settings)*/
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
    setWindowTitle("Настройки");
    label_threshold->setAlignment(Qt::AlignCenter);     //Set label at center
    label_yUpperBound->setAlignment(Qt::AlignCenter);

    //sp_thresholdValue->setValidator( new QIntValidator(20, 350, this) );
    //sp_yUpperBound->   setValidator( new QIntValidator(0, 500, this)  );

    sp_thresholdValue->setFocus();
}

void settings::CreateConnections()
{
    connect(pb_OK, SIGNAL(clicked(bool)), this, SLOT(ok_clicked()));
    connect(pb_cancel, SIGNAL(clicked(bool)), this, SLOT(cancel_clicked()));
}

void settings::ok_clicked()
{
    if ((sp_thresholdValue->value() == 0) || (sp_yUpperBound->value() == 0) )
        qDebug() << "Edit line is empty! Enter some value!";

    else
    {
        quint16 threshold = sp_thresholdValue->value();
        quint16 yUpperBound = sp_yUpperBound->value();
        emit sendThreshold(threshold, yUpperBound);
        accept();
    }
}

void settings::cancel_clicked()
{
    reject();
}

