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
    qsettings.setValue("settings/ComboBoxForGrid", checkbxForGrid->isChecked());
    //close();
    //delete ui;
}

void settings::InterfaceSettings()
{
    setWindowTitle("Настройки");
    label_threshold->setAlignment(Qt::AlignCenter);     //Set label at center
    label_yUpperBound->setAlignment(Qt::AlignCenter);
    label_threshold->setFocus();

    if ( QFile::exists("settings.conf") )
        checkbxForGrid->setChecked(qsettings.value("settings/ComboBoxForGrid").toBool());
    else
        checkbxForGrid->setChecked(true);
}

void settings::CreateConnections()
{
    connect(pb_OK, SIGNAL(clicked(bool)), this, SLOT(ok_clicked()));
    connect(pb_cancel, SIGNAL(clicked(bool)), this, SLOT(cancel_clicked()));
    connect(checkbxForGrid, SIGNAL(clicked(bool)), this, SLOT(checkbox_checked()));
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

void settings::checkbox_checked()
{
    if (checkbxForGrid->checkState() == Qt::CheckState(Qt::Checked))
        emit checkbox_grid(true);
    else
        emit checkbox_grid(false);
}

