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
    explicit MainWindow(const QString& strHost, int nPort, QWidget* pwgt = 0);
    ~MainWindow();

private:
    void CreateGraph();
    void CreatePlot(QVector<InfoChannel> *arrData);
    Ui::MainWindow *ui;
    QCPDataMap  *mapData;
    QCustomPlot *customPlot;
    QTcpSocket* m_pTcpSocket;
    quint16     m_nNextBlockSize;

private slots:
    void slotReadyRead   (                            );
    void slotError       (QAbstractSocket::SocketError);
    void slotSendToServer(                            );
    void slotConnected   (                            );
};

#endif // MAINWINDOW_H
