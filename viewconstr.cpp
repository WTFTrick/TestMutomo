#include "viewconstr.h"

viewConstr::viewConstr(QWidget *parent) : count(450), step(50), QWidget(parent)
{
    CreateView();
}

void viewConstr::CreateView()
{
       QLabel *fileNameLabel = new QLabel(tr("Graphical View:"));
       fileNameLabel->setAlignment(Qt::AlignCenter);
       scene = new QGraphicsScene(this);
       gv = new QGraphicsView();
       gv->setScene(scene);
       QVBoxLayout *mainLayout = new QVBoxLayout;
       mainLayout->addWidget(fileNameLabel);
       mainLayout->addWidget(gv);
       setLayout(mainLayout);
       QBrush TBrush(Qt::transparent);
       QPen outlinePen(Qt::black);
       outlinePen.setWidth(1);

       int width = step * 6;
       quint8 height = 40;
       int empty_area = count / 2.25;
       int xcb = width + 15;
       int xsb = width + 75;
       QSize FixedSize(44,24);

       for (int i = 0; i < count; i+=step)
       {

           for (int j = 0; j < count; j+=step)
           {

               if ((i < width) && (j != empty_area))
               {

                   quint8 rnd = qrand() % 50;
                   QString rand = QString::number(rnd);

                   QList< QComboBox* > ComboBoxList;
                   ComboBoxList << new QComboBox();
                   foreach( QComboBox* cmb, ComboBoxList)
                   {
                       cmb->setFixedSize(FixedSize);
                       cmb->addItem(rand);
                       gpw = scene->addWidget(cmb);
                       cmb->move(i+4,j+7);
                   }

                   QList< QSpinBox* > SpinBoxList;
                   SpinBoxList << new QSpinBox();
                   foreach( QSpinBox* sp, SpinBoxList)
                   {
                       sp->setRange(0,50);
                       sp->setFixedSize(FixedSize);
                       gpw = scene->addWidget(sp);
                       sp->move(xsb,j+7);
                   }

                   QList< QComboBox* > ComboBoxList2;
                   ComboBoxList2 << new QComboBox();
                   foreach( QComboBox* cmb2, ComboBoxList2)
                   {
                       cmb2->setFixedSize(FixedSize);
                       cmb2->addItem("X");
                       cmb2->addItem("Y");
                       gpw = scene->addWidget(cmb2);
                       cmb2->move(xcb,j+7);
                   }


               }

               if (i == empty_area)
               {
                   //qDebug() << "Empty_Area=" << empty_area;
               }

               else
               {
                   rectangle = scene->addRect(0, i, width, height, outlinePen,TBrush);

               }

           }
       }

}




