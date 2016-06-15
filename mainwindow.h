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
class settings;

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


    QTcpSocket* m_pTcpSocket;

    inline void ChannelCheck(quint32 freq, int ind); // выявление плат с каналами, в которых частота регистрации сигналов превышают порог

    bool* badBoards;
    int numberOfBrokenDevice;

private:
    Ui::MainWindow *ui;

    QSettings qsettings;

    enum TYPE_DATA {DATA_RAW, DATA_HIST, DATA_CONFIG_MUTOMO, CMD, SET_VOLTAGE};
    int nPort;
    int LinesCount;
    const unsigned char ChannelsOnBoard;
    quint32 value_threshold;
    quint32 m_nNextBlockSize;
    double xPosOfCircle;               // X coordinate of threshold circle;
    QString strHost;                    // String, that contain an IP-Adress of the server from modal window (Connection dialog)

    double yAxisLowerBound;
    double  calculating_height_of_lines;
    double YlowerBound;
    double YupperBound;
    double XlowerBound;
    double XupperBound;
    double lastPosition;
    double leftBound;
    double rightBound;

    QVector<QCPItemLine*> vecOfLines;

    uint nmChannelsMutomo;
    QCPItemLine *tickHLine;
    QCPItemLine *tickBLine;
    QCPItemText * NumberOfBoard;
    QCPItemText * nmChannelOnBoard;
    QCPDataMap  *mapData;
    QCPGraph    *graph1;
    QCPDataMap* threhshold_data;
    QCPDataMap* thresholdCircleData;
    QCPGraph    *thresholdGraph;
    QCPGraph    *thresholdCircle;
    QCustomPlot *customPlot;
    IPDialog* ip_dialog;                // exemplar of IPDialog class, for creating modal window for Connection.
    settings* settings_dialog;          // exemplar of settings class, for creating modal window for Settings.
    viewConstr* vw;                     // exemplar of viewConstr class, for creating second tab in TabWidget
    int diamCircle;                     // variable, that means diametr of threshold circle

    bool fVisibleNmChannels;
    bool fDeviceGridVisible;
    bool fDetecGridVisible;
    bool PressedOnCircle;               // bool variable, if true - work MouseMove event and threshold line+circles move on Y axis
    bool fVisibleLabels;                // bool variable, if true - draw number of MT48 on customPlot
    bool bUpdatePlot;                   // bool variable, if true - user can see Chart on customPlot
    bool bUpdateViewConstr;             // bool variable, if true - drawes a red rectangles over broken devices in second tab
    QVector<bool> vectorForCheckingDevices;

    void CreateLines();
    void DrawThresholdWidget();         // Threshold line + circle
    void CreateLabels();                // function, that draw labels (number of MT48) on customPlot widget
    void PaintNumberOfBrokenDevices();  // paint number of broken device on graphic
    void CreatePlot(QVector<quint32> *arrData);
    void CreateConnections();
    void ClearVectorForCheckingDevices();
    void DataToServer(TYPE_DATA t_data, QByteArray data);
    void CreateChannelLabels();

private slots:
    void GetJsonFromViewConstr(QByteArray JsonDoc);
    void DrawPlot();
    void ScaleChanged();
    void mouseWheel();
    void slotReadyRead();
    void slotConnected();
    void slotDisconnected();
    void on_actionConnect_to_triggered();
    void connectToHost(QString str);
    void xAxisChanged(QCPRange newRange);
    void yAxisChanged(QCPRange newRange);
    void on_pb_ZoomIn_clicked();
    void on_pb_ZoomOut_clicked();
    void on_pb_ResetRange_clicked();
    void MouseClickOnTextItem(QCPAbstractItem* item, QMouseEvent* event);
    void MouseDoubleClickOnThresholdWidget(QCPAbstractPlottable* plot, QMouseEvent* event);
    void MousePressed(QMouseEvent* event);
    void MoveThreshold(QMouseEvent* event);
    void MouseRealesed(QMouseEvent* event);
    void tabSelected();
    void on_actionSettings_triggered();
    void get_threshold();
    void slotMessage(QString str);
    void check_DetecGrid(bool checked_state);
    void check_DeviceGrid(bool checked_state);

    void slStartDAQ();                          // запустить систему сбора данных на сервере
    void slStopDAQ();                           // остановить систему сбора данных на сервере
    void slDataHistRequest();                   // запросить данные гистограммы скорости счета
    void slDataRawRequest();                    // запросить "сырые" данные
    void slSetVoltage();                        // запустить установку необходимого напряжения
    void slStopSetVoltage();                    // остановить установку заданного напряжения
    void slStartHVScan();                       // запустить высоковольтный скан
    void slStopHVScan();                        // остановить высоковольтный скан

protected:
    void resizeEvent(QResizeEvent* event);
    void changeEvent(QEvent *event);
};

#endif // MAINWINDOW_H
