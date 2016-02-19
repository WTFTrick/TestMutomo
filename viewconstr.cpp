#include "viewconstr.h"

viewConstr::viewConstr(QWidget *parent) : count(450), step(50), QWidget(parent)
{
    CreateView();
}

void viewConstr::CreateView()
{
    QLabel *fileNameLabel = new QLabel(tr("viewConstr Class:"));
    scene = new QGraphicsScene(this);
    gv = new QGraphicsView();
    gv->setScene(scene);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(fileNameLabel);
    mainLayout->addWidget(gv);
    setLayout(mainLayout);
    QBrush TBrush(Qt::transparent);
    QPen outlinePen(Qt::black);
    QPen TPen(Qt::transparent);
    outlinePen.setWidth(1);

    quint16 width = 300;
    quint8 height = 30;
    quint8 empty_area = count / 2.25;

    for (int i = 0; i < count; i+=step)
    {
        if (i < 300)
        {
        QComboBox* cmb = new QComboBox;
        cmb->addItem("10");
        gpw = scene->addWidget(cmb);
        cmb->move(i+2,2);
        }
        if (i == empty_area)
        {
            rectangle = scene->addRect(0, i, width, height, TPen, TBrush);
        }
        else
        {
            rectangle = scene->addRect(0, i, width, height, outlinePen,TBrush);
        }
    }
}


