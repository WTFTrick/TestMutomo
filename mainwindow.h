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
    IPDialog* ip_dialog;

    void CreatePlot(QVector<InfoChannel> *arrData);
    void CreatePlot(QVector<double> *arrData);
    void CreateConnections();



private slots:
    void slotReadyRead();
    void slotConnected();
    void on_actionConnect_to_triggered();
    void connectToHost(QString str);

};

#endif // MAINWINDOW_H
