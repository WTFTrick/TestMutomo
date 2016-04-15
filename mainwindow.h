#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QTcpSocket>
#include <QResizeEvent>
#include <QSettings>
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

    inline void ChannelCheck(quint32 freq, int ind); // выявление плат с каналами, в которых частота регистрации сигналов превышают порог
    void GetJsonFromViewConstr(QByteArray JsonDoc);
    bool* badBoards;
    int numberOfBrokenDevice;

private:
    Ui::MainWindow *ui;

    enum TYPE_DATA{DATA_RAW, DATA_HIST, CFG_MUTOMO, CMD};
    QSettings qsettings;
    int nPort;
    int LinesCount;
    const unsigned char ChannelsOnBoard;
    quint8 value_threshold;
    quint32     m_nNextBlockSize;
    QString strHost;                    // String, that contain an IP-Adress of the server from modal window (Connection dialog)

    double yAxisLowerBound;

    QTcpSocket* m_pTcpSocket;
    QCPItemText * NumberOfBoard;
    QCPDataMap  *mapData;
    QCPGraph    *graph1;
    QCustomPlot *customPlot;
    IPDialog* ip_dialog;                // exemplar of IPDialog class, for creating modal window for Connection.
    settings* settings_dialog;          // exemplar of settings class, for creating modal window for Settings.
    viewConstr* vw;                     // exemplar of viewConstr class, for creating second tab in TabWidget

    bool fVisibleLabels;                // bool variable, if true - draw number of MT48 on customPlot
    bool bUpdatePlot;                   // bool variable, if true - user can see Chart on customPlot
    bool bUpdateViewConstr;             // bool variable, if true - drawes a red rectangles over broken devices in second tab
    QVector<bool> vectorForCheckingDevices;

    void CreateLines();
    void CreateLabels();                 // function, that draw labels (number of MT48) on customPlot widget
    void PaintNumberOfBrokenDevices();  // paint number of broken device on graphic
    void CreatePlot(QVector<quint32> *arrData);
    void CreateConnections();
    void ClearVectorForCheckingDevices();
    void DataToServer(TYPE_DATA t_data, quint32 data);
    void DataHistRequest();
    void DataRawRequset();

private slots:
    void DrawPlot();
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
