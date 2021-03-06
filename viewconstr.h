#ifndef VIEWCONSTR_H
#define VIEWCONSTR_H

#include <QGuiApplication>
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QRect>

#include <QScreen>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QPushButton>
#include <QLayout>
#include <QTcpSocket>


class viewConstr : public QWidget
{
    Q_OBJECT

public:
    explicit viewConstr(QWidget *parent = 0);
    ~viewConstr();



    QJsonDocument docJSON;

    bool OrientationChange;

    QList< QComboBox* > listComboBox;
    QList< QComboBox* > ListCoordComboBox;

    QVector<bool> VectorOfbadBoards;
    QVector<QGraphicsRectItem*> VectorOfRectanglesOverComboBoxes;

    QScreen* screen;

    QGraphicsView * gv;
    QGraphicsScene *scene;
    QGraphicsRectItem *rectangle;
    QGraphicsProxyWidget* gpw;
    QTransform transform;
    QPushButton* pbSaveConfig;
    quint16 count;                  //count need for drawing, area from 0 to count, where will be draw a rectangles
    quint8 step;                    //step for cycle, which will be draw a rectangles
    quint8 nmOfBoardsOnDetector;    //Number of devices on detector
    quint8 countOfBoards;

    QVBoxLayout* mainLayout;

    QTcpSocket* m_pTcpSocket;

    bool event(QEvent *event);
    void ClearJSONFile();
    void BrokenDevice();
    void CreateView();
    void CreateConnections();
    void ClearVectorOfBrokenDevices();
    void resizeEvent(QResizeEvent *event);

public slots:
    void createJSONConfiguration();
    void onRotate(Qt::ScreenOrientation);

signals:
    void messg(QString str);
    void sendJson(QByteArray JsonDoc);

};

#endif // VIEWCONSTR_H
