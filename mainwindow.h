#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QTcpSocket>
#include <qcustomplot.h>


#include "dialog.h"

struct InfoChannel
{
    quint16 nm_channel;
    quint16 freq;
};

namespace Ui
{

class MainWindow;

}

class MainWindow : public QMainWindow
{

    Q_OBJECT

public:
    explicit MainWindow();
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QString strHost;
    int nPort = 2323;
    QCPDataMap  *mapData;
    QCPGraph    *graph1;
    QCustomPlot *customPlot;
    QTcpSocket* m_pTcpSocket;
    quint32     m_nNextBlockSize;
    int LinesCount;
    IPDialog* ip_dialog;
    QCPItemText *NumberOfBoard;

    void CreatePlot(QVector<InfoChannel> *arrData);
    void CreatePlot(QVector<quint32> *arrData);
    void CreateConnections();



private slots:
    void mousePress();
    void mouseWheel();
    void slotReadyRead();
    void slotConnected();
    void on_actionConnect_to_triggered();
    void connectToHost(QString str);

    void AxisChanged(QCPRange newRange);
    void xAxisChanged(QCPRange newRange);
    void yAxisChanged(QCPRange newRange);
    void on_pb_ZoomIn_clicked();
    void on_pb_ZoomOut_clicked();
    void on_pb_ResetRange_clicked();
};

#endif // MAINWINDOW_H
