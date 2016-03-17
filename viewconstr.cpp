#include "viewconstr.h"

viewConstr::viewConstr(QWidget *parent) : count(720), step(80), nmOfBoardsOnDetector(6),  QWidget(parent)
{
    ClearJSONFile();
    CreateView();
    CreateConnections();
}

void viewConstr::CreateView()
{
    QLabel *fileNameLabel = new QLabel(tr("Graphical View:"));
    pb_toJson = new QPushButton("To JSON");
    fileNameLabel->setAlignment(Qt::AlignCenter);
    scene = new QGraphicsScene(this);
    gv = new QGraphicsView();
    gv->setScene(scene);
    gv->setAlignment(Qt::AlignCenter);
    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(fileNameLabel);
    mainLayout->addWidget(gv);
    mainLayout->addWidget(pb_toJson);
    setLayout(mainLayout);
    QBrush TBrush(Qt::transparent);
    QPen outlinePen(Qt::black);
    outlinePen.setWidth(0.5);

    int width = step * nmOfBoardsOnDetector;
    quint8 height = 60;
    int empty_area = count / 2.25;
    int xcb = width + 15;
    int xsb = width + 100;
    QSize FixedSize(60,50);
    QSize SpinBoxSize(85, 50);

    for (quint16 i = 0; i < count; i = i + step)
    {
        if (i != empty_area)
        {
            rectangle = scene->addRect(0, i, width, height, outlinePen,TBrush);
        }
    }

    int checkNumberOfSpinBox = 0;
    for (quint16 i = 0; i < count; i = i + step)
    {

        for (quint16 j = 0; j < count; j = j + step)
        {

            if ((j < width) && (i != empty_area))
            {
                quint8 numberOfBoardMT = qrand() % 50;
                QString s_numberOfBoardMT = QString::number( numberOfBoardMT );

                QComboBox* cmb = new QComboBox();
                listComboBox << cmb;
                cmb->setFixedSize( FixedSize );
                cmb->addItem(  s_numberOfBoardMT );
                gpw = scene->addWidget( cmb );
                cmb->move(j+7,i+7);
            }
        }
    }

    for (int k = 0; k < count; k += step)
    {
        if (k != empty_area)
        {
            QComboBox* cmb_coord = new QComboBox();
            ListCoordComboBox << cmb_coord;
            cmb_coord->setFixedSize( FixedSize );
            cmb_coord->addItem( "X" );
            cmb_coord->addItem( "Y" );
            gpw = scene->addWidget( cmb_coord );
            cmb_coord->move(xcb,k+7);

            QSpinBox* spinbx_numb = new QSpinBox();
            ListSpinBox << spinbx_numb;
            spinbx_numb->setRange(0,50);
            spinbx_numb->setFixedSize(SpinBoxSize);
            gpw = scene->addWidget(spinbx_numb);
            spinbx_numb->move(xsb,k+7);
            checkNumberOfSpinBox++;
            spinbx_numb->setValue(checkNumberOfSpinBox);
        }
    }

}

void viewConstr::CreateConnections()
{
    connect(pb_toJson, SIGNAL(clicked(bool)), this, SLOT(ToJson()));
}

void viewConstr::ClearJSONFile()
{
    QFile file("blocks.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        file.write("");
    }
}

void viewConstr::ToJson()
{
    ClearJSONFile();

    qDebug() << "ToJson btn clicked";

    QJsonDocument docJSON;
    QJsonObject jsonDevice;
    QJsonObject jsonDetector;
    QJsonObject jsonTempInDetector;

    const short nmDevicesOnDetec = 6;
    const short nmDetectors = 8;

    for (int indDetector = 0; indDetector < nmDetectors; ++indDetector){
        for(int indDevice = 0; indDevice < nmDevicesOnDetec; ++indDevice )
        {
            QString nameField("Number of MT48: ");
            nameField += QString::number(indDevice);
            jsonDevice[nameField] = listComboBox[(indDetector*nmDevicesOnDetec) + indDevice]->currentText();
        }
        jsonTempInDetector["Device "] = jsonDevice;

        jsonTempInDetector["Coordinate"] = ListCoordComboBox[indDetector]->currentText();
        qDebug() << "Index of ComboBox:"<< indDetector << "; Value in ComboBox:" << ListCoordComboBox[indDetector]->currentText();

        jsonTempInDetector["Number "] = ListSpinBox[indDetector]->value();
        qDebug() << "Index of SpinBox:"<< indDetector << "; Value in SpinBox:" << ListSpinBox[indDetector]->value();

        QString nameField("Detector ");
        nameField += QString::number(indDetector);

        jsonDetector[nameField] = jsonTempInDetector;
    }
    docJSON.setObject(jsonDetector);

    QFile jsnFile("blocks.json");
    jsnFile.open(QFile::Append);
    QTextStream outJson(&jsnFile);
    outJson << docJSON.toJson();
    jsnFile.close();

    return;
}




