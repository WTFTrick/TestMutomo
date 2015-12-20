#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QTcpSocket>
#include <qcustomplot.h>

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
    explicit MainWindow(const QString& strHost, int nPort);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    void CreateGraph();
    void CreatePlot(QVector<InfoChannel> *arrData);
    QCPDataMap  *mapData;
    QCPGraph *graph1;
    QCustomPlot *customPlot;
    QTcpSocket* m_pTcpSocket;
    quint16     m_nNextBlockSize;
    int arraySize;

private slots:
    void slotReadyRead   ();
    void slotConnected   ();
};

#endif // MAINWINDOW_H
