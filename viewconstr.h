#ifndef VIEWCONSTR_H
#define VIEWCONSTR_H

#include <QMainWindow>
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QComboBox>
#include <QSpinBox>

#include "mainwindow.h"

class viewConstr : public QWidget
{
    Q_OBJECT
public:
    explicit viewConstr(QWidget *parent = 0);
    void CreateView();
    void CreateConnections();
    quint8 nmOfBoardsOnDetector;
    QList< QComboBox* > listComboBox;
    QList< QSpinBox* > ListSpinBox;
    QList< QComboBox* > ListCoordComboBox;


    QPushButton* pb_toJson;
    QGraphicsView * gv;
    QGraphicsScene *scene;
    QGraphicsRectItem *rectangle;
    QGraphicsProxyWidget* gpw;
    QTransform transform;
    quint16 count;
    quint8 step;
    QVBoxLayout *mainLayout;
    QWidget *window ;

public slots:
    void ToJson();

signals:

private:

public slots:
};

#endif // VIEWCONSTR_H
