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
    ~MainWindow();

    QTcpSocket* m_pTcpSocket;           // выявление плат с каналами, в которых частота регистрации сигналов
                                        // превышает порог
    int numberOfBrokenDevice;
    inline void ChannelCheck(quint32 freq, int ind);
    bool* badBoards;

    void GetJsonFromViewConstr(QString JsonDoc);

private:
    Ui::MainWindow *ui;
    QString strHost;                    // String, that contain an IP-Adress of the server from modal window (Connection dialog)
    int nPort;
    QCPDataMap  *mapData;
    QCPGraph    *graph1;
    QCustomPlot *customPlot;
    quint32     m_nNextBlockSize;
    int LinesCount;
    IPDialog* ip_dialog;                // exemplar of IPDialog class, for creating modal window for Connection dialog.
    settings* settings_dialog;
    bool fVisibleLabels;
    bool bUpdatePlot;                   // bool variable, if true - user can see Chart
    bool bUpdateViewConstr;             // bool variable, if true - drawes a red rectangles over broken devices in second tab
    const unsigned char ChannelsOnBoard;
    quint8 value_threshold;
    viewConstr* vw;                     // exemplar of viewConstr class, for creating second tab in TabWidget
    QVector<bool> vectorForCheckingDevices;

    void ClearVectorForCheckingDevices();

    enum TYPE_DATA{DATA_RAW, DATA_HIST, CFG_MUTOMO, CMD};

    QCPItemText * NumberOfBoard;
    void CreateLines();
    void PaintNumberOfBrokenDevices(); //paint number of broken device on graphic
    void CreatePlot(QVector<quint32> *arrData);
    void CreateConnections();

    void DataToServer(TYPE_DATA t_data, quint32 data);
    void DataHistRequest();
    void DataRawRequset();

private slots:
    void StopServer();
    void StartServer();
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
    void slotMessage(QString str);

protected:
    void resizeEvent(QResizeEvent* event);
    void changeEvent(QEvent *event);
    //virtual bool event(QEvent *event);
};

#endif // MAINWINDOW_H
