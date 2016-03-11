#include "viewconstr.h"

viewConstr::viewConstr(QWidget *parent) : count(720), step(80), nmOfBoardsOnDetector(6),  QWidget(parent)
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
            qDebug() << "Square n%";
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
    qDebug() << "ToJson";

    QVariantMap map;
    QVariantMap device_map;

    foreach(QComboBox* cmb, listComboBox)
    {
        device_map.insert("Number of MT48", cmb->currentText());
        qDebug() << cmb->currentText();
    }

    foreach(QSpinBox* spinbx_numb, ListSpinBox)
    {
        map.insert("Number", spinbx_numb->value());
    }

    foreach(QComboBox* cmb_coord, ListCoordComboBox)
    {
        map.insert("Coordiante", cmb_coord->currentText());
    }

    deviceObject = QJsonObject::fromVariantMap(device_map);

    obj = QJsonObject::fromVariantMap(map);
    obj["Device"] = QJsonObject::fromVariantMap(device_map);
    detectorObject["Detector"] = obj;

    //detectorObject["Device"] = deviceObject;
    //detectorObject["Device"] = QJsonObject::fromVariantMap(device_map);

    document.setObject(detectorObject);

    //qDebug() << document.toJson();

    QFile jsonFile("blocks.json");
    jsonFile.open(QFile::Append);
    QTextStream out(&jsonFile);
    out << document.toJson();
    jsonFile.close();

}




