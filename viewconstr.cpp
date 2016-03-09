#include "viewconstr.h"

viewConstr::viewConstr(QWidget *parent) : count(720), step(80), QWidget(parent)
{
    CreateView();
    CreateConnections();
}

void viewConstr::CreateView()
{
    //JSON
    //Layers
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
       outlinePen.setWidth(1);

       int width = step * 5;
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

       for (int i = 0; i < count; i = i + step)
       {
           if (i != empty_area)
           {
               rectangle = scene->addRect(0, i, width, height, outlinePen,TBrush);
           }
       }


       for (int i = 0; i < count; i = i + step)
       {
           for (int j = 0; j < count; j = j + step)
           {

               if ((i < width) && (j != empty_area))
               {
                   quint8 rnd = qrand() % 50;
                   QString rand = QString::number(rnd);

                   //QList< QComboBox* > ComboBoxList;
                   //ComboBoxList << new QComboBox();
                   ComboBoxList << new QComboBox();
                   foreach(cmb, ComboBoxList)
                   {
                       cmb = new QComboBox;
                       cmb->setFixedSize(FixedSize);
                       cmb->addItem(rand);
                       //cmb->setLayout(bx);
                       //cmb->backgroundRole();
                       gpw = scene->addWidget(cmb);
                       cmb->move(i+7,j+7);
                   }

                   //QList< QSpinBox* > SpinBoxList;
                   SpinBoxList << new QSpinBox();
                   foreach(sp, SpinBoxList)
                   {
                       sp = new QSpinBox;
                       sp->setRange(0,50);
                       //sp->setSuffix("");
                       sp->setFixedSize(SpinBoxSize);
                       gpw = scene->addWidget(sp);
                       sp->move(xsb,j+7);
                   }

                   //QList< QComboBox* > ComboBoxList2;
                   ComboBoxList2 << new QComboBox();
                   foreach(cmb2, ComboBoxList2)
                   {
                       cmb2 = new QComboBox;
                       cmb2->setFixedSize(FixedSize);
                       cmb2->addItem("X");
                       cmb2->addItem("Y");
                       gpw = scene->addWidget(cmb2);
                       cmb2->move(xcb,j+7);
                   }
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

    QVariantMap nmb_map;
    QVariantMap xy_cmb2;
    QVariantMap sp_map;

    foreach(cmb, ComboBoxList)
    {
        nmb_map.insert("Number of board", cmb->itemData(cmb->currentIndex()));
    }

    foreach(sp, SpinBoxList)
    {
        sp_map.insert("SpinBox", sp->value());
    }

    foreach(cmb2, ComboBoxList2)
    {
        xy_cmb2.insert("Coordiante", cmb2->itemData(cmb2->currentIndex()));
    }

    numberOfBoardsObject = QJsonObject::fromVariantMap(nmb_map);
    detectorObject["Detector"] = deviceObject;
    detectorObject["Sp"] = QJsonObject::fromVariantMap(sp_map);
    detectorObject["Coordinate"] = QJsonObject::fromVariantMap(xy_cmb2);
    deviceObject["Device"] = numberOfBoardsObject;

    document.setObject(detectorObject);

    //qDebug() << document.toJson();

    QFile jsonFile("blocks.json");
    jsonFile.open(QFile::Append);
    QTextStream out(&jsonFile);
    //out << document.toJson();
    jsonFile.close();

}




