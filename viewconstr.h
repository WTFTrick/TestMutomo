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
#include <QRect>

#include "mainwindow.h"

class MainWindow;

class viewConstr : public QWidget
{
    Q_OBJECT

public:
    explicit viewConstr(QWidget *parent = 0);
    ~viewConstr();

    QJsonDocument docJSON;

    QList< QComboBox* > listComboBox;
    QList< QSpinBox* > ListSpinBox;
    QList< QComboBox* > ListCoordComboBox;

    QVector<bool> VectorOfbadBoards;
    QVector<QGraphicsRectItem*> VectorOfRectanglesOverComboBoxes;

    QGraphicsRectItem* RectanglesForComboBoxes;
    QScreen * screen;

    QGraphicsView * gv;
    QGraphicsScene *scene;
    QGraphicsRectItem *rectangle;
    QGraphicsProxyWidget* gpw;
    QTransform transform;
    QPushButton* pb_toJson;
    quint16 count;                  //count need for drawing, area from 0 to count, where will be draw a rectangles
    quint8 step;                    //step for cycle, which will be draw a rectangles
    quint8 nmOfBoardsOnDetector;    //Number of devices on detector
    quint8 countOfBoards;

    QVBoxLayout *mainLayout;
    QWidget *window;
    QTcpSocket* m_pTcpSocket;
    MainWindow* mw;

    void ClearJSONFile();
    void BrokenDevice();
    void CreateView();
    void CreateConnections();
    void ClearVectorOfBrokenDevices();
    void resizeEvent(QResizeEvent *event);
    bool event(QEvent *event);

public slots:
    void ToJson();
    void onRotate(Qt::ScreenOrientation);

signals:
    void messg(QString str);

};

#endif // VIEWCONSTR_H
