#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QTcpSocket>
#include <QResizeEvent>
#include <math.h>
#include <qcustomplot.h>

#include "dialog.h"
#include "settings.h"
#include "viewconstr.h"

class viewConstr;

namespace Ui
{

class MainWindow;

}

class MainWindow : public QMainWindow
{

    Q_OBJECT

public:
    explicit MainWindow();
    int numberOfBrokenDevice;
    ~MainWindow();

     //QVector<int> *NOBD;

private:
    Ui::MainWindow *ui;
    QString strHost;
    int nPort;
    QCPDataMap  *mapData;
    QCPGraph    *graph1;
    QCustomPlot *customPlot;
    QTcpSocket* m_pTcpSocket;
    quint32     m_nNextBlockSize;
    int LinesCount;                         // ...
    IPDialog* ip_dialog;
    settings* settings_dialog;
    bool fVisibleLabels;

    const unsigned char ChannelsOnBoard;
    quint8 value_threshold;
    viewConstr* vw;

    void CreateLines();
    //void CreatePlot(QVector<InfoChannel> *arrData);
    void CreatePlot(QVector<quint32> *arrData);
    void CreateConnections();
    void resizeEvent(QResizeEvent* event);
    void changeEvent(QEvent *event);

private slots:
    void StopServer();
    void StartServer();
    void ServerControl(quint8 status);
    void ScaleChanged();
    void mousePress();
    void mouseWheel();
    void slotReadyRead();
    void slotConnected();
    void on_actionConnect_to_triggered();
    void connectToHost(QString str);
    void xAxisChanged(QCPRange newRange);
    void yAxisChanged(QCPRange newRange);
    void on_pb_ZoomIn_clicked();
    void on_pb_ZoomOut_clicked();
    void on_pb_ResetRange_clicked();
    void MousePress(QCPAbstractItem* item, QMouseEvent* event);
    void tabSelected();
    void on_actionSettings_triggered();
    void get_threshold(int threshold);
};

#endif // MAINWINDOW_H
