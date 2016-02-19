#include "viewconstr.h"

viewConstr::viewConstr(QWidget *parent) : QWidget(parent)
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
    for (int i = 0; i < 450; i+=50)
    {
        if (i == 200)
        {
            rectangle = scene->addRect(0, i, 300, 30, TPen, TBrush);
        }
        else
            rectangle = scene->addRect(0, i, 300, 30, outlinePen,TBrush);
    }
}


