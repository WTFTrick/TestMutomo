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
                   foreach( QComboBox* cmb, ComboBoxList)
                   {
                       cmb = new QComboBox;
                       cmb->setFixedSize(FixedSize);
                       cmb->addItem(rand);
                       cmb->addItem("0");
                       //cmb->setLayout(bx);
                       //cmb->backgroundRole();
                       gpw = scene->addWidget(cmb);
                       cmb->move(i+7,j+7);
                   }

                   //QList< QSpinBox* > SpinBoxList;
                   SpinBoxList << new QSpinBox();
                   foreach( QSpinBox* sp, SpinBoxList)
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
                   foreach( QComboBox* cmb2, ComboBoxList2)
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

    /*QVariantMap map;
    map.insert("1:", ComboBoxList);
    object = QJsonObject::fromVariantMap(map);
    document.setObject(object);
    qDebug() << document.toJson();*/
}




