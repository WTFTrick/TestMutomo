#include "viewconstr.h"

viewConstr::viewConstr(QWidget *parent) : count(720), step(80), nmOfBoardsOnDetector(7),  QWidget(parent)
{
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

    QFile file("blocks.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        file.write("");
    }

    file.close();

    for (quint16 i = 0; i < count; i = i + step)
    {
        if (i != empty_area)
        {
            rectangle = scene->addRect(0, i, width, height, outlinePen,TBrush);
            //qDebug() << "Square n%";
        }
    }

    for (quint16 i = 0; i < count; i = i + step)
    {

        for (quint16 j = 0; j < count; j = j + step)
        {

            if ((i < width) && (j != empty_area))
            {
                quint8 numberOfBoardMT = qrand() % 50;
                QString s_numberOfBoardMT = QString::number( numberOfBoardMT);

                QComboBox* cmb = new QComboBox();
                cmb->setFixedSize( FixedSize );
                cmb->addItem(  s_numberOfBoardMT );
                gpw = scene->addWidget( cmb );
                cmb->move(i+7,j+7);
                listComboBox << cmb;

                QComboBox* cmb_coord = new QComboBox();
                cmb_coord->setFixedSize( FixedSize );
                cmb_coord->addItem( "X" );
                cmb_coord->addItem( "Y" );
                gpw = scene->addWidget(cmb_coord);
                cmb_coord->move(xcb,j+7);
                ListCoordComboBox << cmb_coord;

                QSpinBox* spinbx_numb = new QSpinBox();
                spinbx_numb->setRange(0,50);
                spinbx_numb->setFixedSize(SpinBoxSize);
                gpw = scene->addWidget(spinbx_numb);
                spinbx_numb->move(xsb,j+7);
                ListSpinBox << spinbx_numb;

                //Доделать комбобоксы и спин бокс, переделать циклы!
                //Разобраться со структурой JSON файла!

            }

        }
    }
}

void viewConstr::CreateConnections()
{
    connect(pb_toJson, SIGNAL(clicked(bool)), this, SLOT(ToJson()));
}

void viewConstr::ToJson()
{
    qDebug() << "ToJson btn clicked";

    QJsonDocument docJSON;
    QJsonObject jsonDevice;
    QJsonObject jsonDetector;
    QJsonObject jsonTempInDetector;

    const short nmDevicesOnDetec = 6;
    for (int indDetector = 0; indDetector < 8; ++indDetector){
        for(int indDevice = 0; indDevice < nmDevicesOnDetec; ++indDevice )
        {
            QString nameField("Number of MT48: ");
            nameField += QString::number(indDevice);
            jsonDevice[nameField] = listComboBox[(indDetector*nmDevicesOnDetec) + indDevice]->currentText();
        }
        jsonTempInDetector["Device "] = jsonDevice;

        QString coordField("Cooridnate ");
        coordField += QString::number(indDetector);
        jsonTempInDetector[coordField] = ListCoordComboBox[indDetector]->currentText();

        ListSpinBox[0];
        QString numbField("Number ");
        numbField += QString::number(indDetector);
        jsonTempInDetector[numbField] = ListSpinBox[0]->value();

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




