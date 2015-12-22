#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QTcpSocket>
#include <qcustomplot.h>
#include <dialog.h>
#include <qfile.h>

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
    QCPGraph    *graph1;
    QCustomPlot *customPlot;
    QTcpSocket* m_pTcpSocket;
    quint32     m_nNextBlockSize;

private slots:
    void slotReadyRead   ();
    void slotConnected   ();
    void on_actionConnect_to_triggered();
};

#endif // MAINWINDOW_H
