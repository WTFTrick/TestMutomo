#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork>
#include <QtGui>

MainWindow::MainWindow() :
    nPort(2323),
    m_nNextBlockSize(0),
    vectorForCheckingDevices(48),
    numberOfBrokenDevice(0),
    LinesCount(49),
    xPosOfCircle(2227.5),
    diamCircle(20),
    qsettings("settings.conf", QSettings::NativeFormat),
    PressedOnCircle(false),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("MuTomo Клиент");
    setCentralWidget(ui->tabWidget);


    ui->groupBox_scale->setLayout(ui->hl_scale);
    ui->groupBox_data->setLayout(ui->hl_data);
    ui->groupBox_vhps->setLayout(ui->hl_vhps);
    ui->groupBox_voltage->setLayout(ui->hl_voltage);


    settings_dialog = new settings( this );
    settings_dialog->setModal(true);

    ip_dialog = new IPDialog( this );

    YlowerBound = 0;
    XlowerBound = 0;
    XupperBound = 2475;

    if ( !QFile::exists("settings.conf") )
    {
        YupperBound = 150;
        strHost = "0.0.0.0";
        value_threshold = 100;
        ui->statusBar->showMessage("Приложение запущено. Пороговое значение равно " + QString::number(value_threshold) + ".");
        fDetecGridVisible = true;
        fDeviceGridVisible = true;
        settings_dialog->checkbxForDetecGrid->setChecked(true);
        settings_dialog->checkbxForDeviceGrid->setChecked(true);
    }
    else
    {
        value_threshold = qsettings.value("settings/threshold").toInt();
        YupperBound = qsettings.value("settings/yUpperBound").toInt();
        strHost = qsettings.value("settings/IP").toString();
        connectToHost(strHost);
        fDetecGridVisible = qsettings.value("settings/DetecGridCheck").toBool();
        fDeviceGridVisible = qsettings.value("settings/DeviceGridCheck").toBool();
        settings_dialog->checkbxForDetecGrid->setChecked(qsettings.value("settings/CheckBoxForDetecGrid").toBool());
        settings_dialog->checkbxForDeviceGrid->setChecked(qsettings.value("settings/CheckBoxForDeviceGrid").toBool());

        ui->statusBar->showMessage("Приложение запущено. Пороговое значение равно " + QString::number(value_threshold) + ".");
    }

    graph1 = ui->customPlot->addGraph();
    graph1->setLineStyle((QCPGraph::LineStyle)(2));
    mapData = graph1->data();

    ui->customPlot->installEventFilter(this);
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom );
    ui->customPlot->xAxis->setLabel("Номер канала");
    ui->customPlot->yAxis->setLabel("Частота, HZ");
    ui->customPlot->xAxis->setRange(XlowerBound, XupperBound);
    ui->customPlot->yAxis->setRange(YlowerBound, YupperBound);
    ui->customPlot->addLayer("BG",ui->customPlot->layer("bg"), QCustomPlot::limBelow);
    ui->customPlot->setCurrentLayer("BG");


    vw = new viewConstr();
    ui->tabWidget->addTab(vw, tr("Конфигурация"));


    fVisibleLabels = false;

    // Colour and width of graph
    QPen graphPen;
    graphPen.setColor(QColor(0, 0, 0));
    graphPen.setWidthF(0.4);
    graph1->setPen(graphPen);

    //MutomoHost = "10.162.1.110"

    CreateConnections();

#ifdef ANDROID
    //Set Threshold Circle Diametr for Android
    diamCircle = 80;
#else
    //Set Threshold Circle Diametr for Desktop
    diamCircle = 30;
#endif

    bUpdatePlot = true;
    bUpdateViewConstr  = false;
    fVisibleNmChannels = false;

    ui->pb_ResetRange->setFocus();    
    ui->pb_startServer->setEnabled(false);
    ui->pb_stopServer->setEnabled(false);
    ui->pbSetVoltage->setEnabled(false);
    ui->pbStartHVScan->setEnabled(false);
    ui->pbStopHVScan->setEnabled(false);
    ui->pbStopSetVoltage->setEnabled(false);
    ui->spVoltage->setEnabled(false);

    // Threshold line and circle
    // =========================================
    thresholdGraph = ui->customPlot->addGraph();
    threhshold_data = thresholdGraph->data();

    QPen pen1;
    pen1.setColor(QColor(255,0,0));
    pen1.setWidth(2);
    pen1.setStyle(Qt::DotLine);
    thresholdGraph->setPen(pen1);

    thresholdCircle = ui->customPlot->addGraph();
    thresholdCircleData = thresholdCircle->data();


    thresholdCircle->setPen(QPen(Qt::red));
    thresholdCircle->setLineStyle(QCPGraph::lsNone);
    QCPScatterStyle ss_disc( QCPScatterStyle::ssCircle, QPen(Qt::red), QBrush(Qt::white), diamCircle);
    thresholdCircle->setScatterStyle(ss_disc);
    // =========================================

    ui->customPlot->addLayer("belowmain", ui->customPlot->layer("main"), QCustomPlot::limBelow);
    graph1->setLayer("belowmain");



    ui->tabWidget->setFocus();
}

MainWindow::~MainWindow()
{
    qsettings.setValue("settings/threshold", value_threshold);
    qsettings.setValue("settings/yUpperBound", YupperBound);
    qsettings.setValue("settings/IP", strHost);
    qsettings.setValue("settings/DetecGridCheck", fDetecGridVisible);
    qsettings.setValue("settings/DeviceGridCheck", fDeviceGridVisible);
    qsettings.setValue("settings/CheckBoxForDetecGrid", settings_dialog->checkbxForDetecGrid->isChecked());
    qsettings.setValue("settings/CheckBoxForDeviceGrid", settings_dialog->checkbxForDeviceGrid->isChecked());
    qsettings.sync();


    delete settings_dialog;
    delete ip_dialog;
    delete vw;
    delete ui;
}

void MainWindow::CreateConnections()
{
    //qDebug() << "CreateConnections";

    // connections between mainWindow and modal dialogs
    connect(ip_dialog, SIGNAL(sendData(QString)), this, SLOT(connectToHost(QString)));
    //connect(settings_dialog, SIGNAL(sendThreshold(quint16,quint16)), this, SLOT(get_threshold(quint16,quint16)));
    connect(settings_dialog, SIGNAL(checkboxDetec_grid(bool)), this, SLOT(check_DetecGrid(bool)));
    connect(settings_dialog, SIGNAL(checkboxDevice_grid(bool)), this, SLOT(check_DeviceGrid(bool)));

    // QCustomPlot connects
    connect(ui->customPlot, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(MouseRealesed(QMouseEvent*)));
    connect(ui->customPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(MoveThreshold(QMouseEvent*)));
    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(MousePressed(QMouseEvent*)));
    connect(ui->customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(yAxisChanged(QCPRange)));

    // scaled plot
    connect(ui->pb_ZoomIn, SIGNAL(clicked(bool)),               this, SLOT(ScaleChanged()));
    connect(ui->pb_ZoomOut, SIGNAL(clicked(bool)),              this, SLOT(ScaleChanged()));
    connect(ui->pb_ResetRange, SIGNAL(clicked(bool)),           this, SLOT(ScaleChanged()));

    // mouse click on some area of plot
    connect(ui->customPlot, SIGNAL(itemClick(QCPAbstractItem*,QMouseEvent*)), this,SLOT(MouseClickOnTextItem(QCPAbstractItem* , QMouseEvent*)));
    connect(ui->customPlot, SIGNAL(plottableDoubleClick(QCPAbstractPlottable*,QMouseEvent*)), this, SLOT(MouseDoubleClickOnThresholdWidget(QCPAbstractPlottable*,QMouseEvent*)));

    // start/stop server buttons
    connect(ui->pb_startServer, SIGNAL(clicked(bool)), this, SLOT(slStartDAQ()));
    connect(ui->pb_stopServer, SIGNAL(clicked(bool)), this, SLOT(slStopDAQ()));

    // tab changed event
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabSelected()));

    // info from viewConstr class to MainWindow StatusBar
    connect(vw, SIGNAL(messg(QString)), this, SLOT(slotMessage(QString)));

    // replot event
    connect(ui->customPlot, SIGNAL(beforeReplot()), this, SLOT(DrawPlot()));

    connect(vw, SIGNAL(sendJson(QByteArray)), this, SLOT(GetJsonFromViewConstr(QByteArray)));

    // tab hv-scan
    connect(ui->pbSetVoltage,       &QPushButton::clicked, this, &MainWindow::slSetVoltage);
    connect(ui->pbStopSetVoltage,   &QPushButton::clicked, this, &MainWindow::slStopSetVoltage);
    connect(ui->pbStartHVScan,      &QPushButton::clicked, this, &MainWindow::slStartHVScan);
    connect(ui->pbStopHVScan,       &QPushButton::clicked, this, &MainWindow::slStopHVScan);
}

void MainWindow::CreatePlot(QVector<quint32> *arrData)
{
    unsigned int maxY = 0;
    for (double i = 0; i < arrData->size(); i++)
    {
        mapData->operator [](i) = QCPData(i, arrData->at(i));
        if (arrData->at(i) > maxY)
            maxY = arrData->at(i);
    }

    ui->customPlot->replot();
}

void MainWindow::slotReadyRead()
{
    // Reading data from server

    QDataStream in(m_pTcpSocket);
    in.setVersion(QDataStream::Qt_5_4);
    for (;;)
    {
        if (!m_nNextBlockSize)
        {
            if (m_pTcpSocket->bytesAvailable() < sizeof(quint32))
            {
                break;
            }
            in >> m_nNextBlockSize;
        }

        if (m_pTcpSocket->bytesAvailable() < m_nNextBlockSize)
        {
            break;
        }

        ClearVectorForCheckingDevices();

        quint32  freq = 0;
        QVector<quint32> arrFreq;
        for (int i = 0; i < m_nNextBlockSize/4; i++ )
        {
            in >> freq;
            arrFreq.append( freq );

            ChannelCheck(freq, i);
        }

        if ( bUpdatePlot )
            CreatePlot( &arrFreq );

        if ( bUpdateViewConstr )
            vw->BrokenDevice();

        arrFreq.clear();
        m_nNextBlockSize = 0;
    }
}

void MainWindow::slStartDAQ()
{
    QByteArray arrayStart;

    quint32 data = 1;
    TYPE_DATA t_data = CMD;
    QByteArray ba_data;
    ba_data.setNum(data);

    QDataStream out(&arrayStart, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_6);
    out << quint64(0);
    out << t_data;
    out << ba_data;
    out.device()->seek(0);
    quint64 size_pkg = quint64(arrayStart.size() - sizeof(quint64));
    out << size_pkg;


    if (m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        DataToServer(t_data, arrayStart);

        vw->pbSaveConfig->setEnabled(false);
        ui->pb_stopServer->setEnabled(true);
        ui->pb_startServer->setEnabled(false);
        ui->pbSetVoltage->setEnabled(false);
        ui->pbStartHVScan->setEnabled(false);
        ui->pbStopHVScan->setEnabled(false);
        ui->pbStopSetVoltage->setEnabled(false);
        ui->spVoltage->setEnabled(false);

        ui->tabWidget->setFocus();
        ui->statusBar->showMessage("Серверу отправлена команда 'старт'.");
    }
    else
        ui->statusBar->showMessage("Клиент не подключен!");

}

void MainWindow::slStopDAQ()
{
    quint32 data = 0;
    QByteArray arrayStop;
    TYPE_DATA t_data = CMD;

    QByteArray ba_data;
    ba_data.setNum(data);

    QDataStream out(&arrayStop, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_6);

    out << quint64(0);

    out << t_data;
    out << ba_data;

    out.device()->seek(0);
    quint64 size_pkg = quint64(arrayStop.size() - sizeof(quint64));
    out << size_pkg;

    if (m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        DataToServer(t_data, arrayStop);

        vw->pbSaveConfig->setEnabled(true);
        ui->pb_startServer->setEnabled(true);
        ui->pb_stopServer->setEnabled(false);
        ui->pbSetVoltage->setEnabled(true);
        ui->pbStartHVScan->setEnabled(true);
        ui->pbStopHVScan->setEnabled(false);
        ui->pbStopSetVoltage->setEnabled(false);
        ui->spVoltage->setEnabled(true);

        ui->tabWidget->setFocus();
        ui->statusBar->showMessage("Серверу отправлена команда 'стоп'.");
    }
    else
        ui->statusBar->showMessage("Клиент не подключен!");
}

void MainWindow::slDataHistRequest()
{
    // эта функция не будет работать, смотри как реализованы рабочие функции
    TYPE_DATA t_data = DATA_HIST;
    QByteArray null;
    null.setNum(0);
    if (m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
        DataToServer(t_data, null );
    else
        ui->statusBar->showMessage("Клиент не подключен!");
}

void MainWindow::slDataRawRequest()
{
    // эта функция не будет работать, смотри как реализованы рабочие функции
    TYPE_DATA t_data = DATA_RAW;
    QByteArray null;
    null.setNum(0);
    if (m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
        DataToServer(t_data, null );
    else
        ui->statusBar->showMessage("Клиент не подключен!");
}

void MainWindow::slSetVoltage()
{
    QByteArray pkgData;

    int data = ui->spVoltage->value();
    TYPE_DATA t_data = SET_VOLTAGE;

    QByteArray ba_data;
    ba_data.setNum(data);

    QDataStream out(&pkgData, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_6);
    out << quint64(0);
    out << t_data;
    out << ba_data;
    out.device()->seek(0);
    quint64 size_pkg = quint64(pkgData.size() - sizeof(quint64));
    out << size_pkg;

    if (m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        DataToServer(t_data, pkgData);
        ui->statusBar->showMessage("Серверу отправлена команда 'установка напряжения'.");
    }
    else
        ui->statusBar->showMessage("Клиент не подключен!");

    ui->pbStopSetVoltage->setEnabled(true);
    ui->spVoltage->setEnabled(false);
    ui->pbSetVoltage->setEnabled(false);
    ui->pbStartHVScan->setEnabled(false);
    ui->pbStopHVScan->setEnabled(false);
    vw->pbSaveConfig->setEnabled(false);
    ui->pb_stopServer->setEnabled(false);
    ui->pb_startServer->setEnabled(false);

}

void MainWindow::slStopSetVoltage()
{
    QByteArray pkgData;
    quint32 data = 3;
    TYPE_DATA t_data = CMD;

    QByteArray ba_data;
    ba_data.setNum(data);

    QDataStream out(&pkgData, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_6);
    out << quint64(0);
    out << t_data;
    out << ba_data;
    out.device()->seek(0);
    quint64 size_pkg = quint64(pkgData.size() - sizeof(quint64));
    out << size_pkg;

    if (m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        DataToServer(t_data, pkgData);
        ui->statusBar->showMessage("Серверу отправлена команда 'остановить установку напряжения'.");
    }
    else
        ui->statusBar->showMessage("Клиент не подключен!");

    vw->pbSaveConfig->setEnabled(true);
    ui->pbStopSetVoltage->setEnabled(false);
    ui->spVoltage->setEnabled(true);
    ui->pbSetVoltage->setEnabled(true);
    ui->pbStartHVScan->setEnabled(true);
    ui->pbStopHVScan->setEnabled(false);
    ui->pb_stopServer->setEnabled(false);
    ui->pb_startServer->setEnabled(true);

}

void MainWindow::slStartHVScan()
{
    QByteArray pkgData;

    int data = 4;
    TYPE_DATA t_data = CMD;

    QByteArray ba_data;
    ba_data.setNum(data);

    QDataStream out(&pkgData, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_6);
    out << quint64(0);
    out << t_data;
    out << ba_data;
    out.device()->seek(0);
    quint64 size_pkg = quint64(pkgData.size() - sizeof(quint64));
    out << size_pkg;

    if (m_pTcpSocket->state() == QAbstractSocket::ConnectedState){
        DataToServer(t_data, pkgData);
        ui->statusBar->showMessage("Запущен высоковольтный скан.");
    }
    else
        ui->statusBar->showMessage("Клиент не подключен!");

    ui->pbStartHVScan->setEnabled(false);
    ui->spVoltage->setEnabled(false);
    ui->pbStopHVScan->setEnabled(true);
    ui->pbStopSetVoltage->setEnabled(false);
    ui->pbSetVoltage->setEnabled(false);
    vw->pbSaveConfig->setEnabled(false);
    ui->pb_stopServer->setEnabled(false);
    ui->pb_startServer->setEnabled(false);
}

void MainWindow::slStopHVScan()
{
    QByteArray pkgData;

    int data = 5;
    TYPE_DATA t_data = CMD;

    QByteArray ba_data;
    ba_data.setNum(data);

    QDataStream out(&pkgData, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_6);
    out << quint64(0);
    out << t_data;
    out << ba_data;
    out.device()->seek(0);
    quint64 size_pkg = quint64(pkgData.size() - sizeof(quint64));
    out << size_pkg;

    if (m_pTcpSocket->state() == QAbstractSocket::ConnectedState){
        DataToServer(t_data, pkgData);
        ui->statusBar->showMessage("Высоковольтный скан остановлен.");
    }
    else
        ui->statusBar->showMessage("Клиент не подключен!");

    ui->pbSetVoltage->setEnabled(true);
    ui->spVoltage->setEnabled(true);
    ui->pbSetVoltage->setEnabled(true);
    ui->pbStartHVScan->setEnabled(true);
    ui->pbStopHVScan->setEnabled(false);
    vw->pbSaveConfig->setEnabled(true);
    ui->pb_stopServer->setEnabled(false);
    ui->pb_startServer->setEnabled(true);

}

void MainWindow::GetJsonFromViewConstr(QByteArray JsonDoc)
{
    //Передать серверу конфигурацию в виде JSON

    TYPE_DATA t_data = DATA_CONFIG_MUTOMO;

    QByteArray arrayJson;
    QDataStream out(&arrayJson, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_6);

    out << quint64(0);

    out << t_data;
    out << JsonDoc;

    out.device()->seek(0);
    quint64 size_pkg = quint64(arrayJson.size() - sizeof(quint64));
    out << size_pkg;


    if (m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        DataToServer(t_data, arrayJson);
        ui->statusBar->showMessage("Конфигурация отправлена на сервер.");
    }
    else
        ui->statusBar->showMessage("Клиент не подключен!");
}

void MainWindow::DataToServer(TYPE_DATA t_data, QByteArray data)
{
    //  Передача данных серверу
    //  Блок данных |Size|Type|Data|

    quint64 sizeBlock = m_pTcpSocket->write(data);

    /*qDebug() << "Client sent type:" << t_data;
    qDebug() << "Size of t_data:" << sizeof( t_data) << " bytes";
    qDebug() << "Client sent data:" << data;
    qDebug() << "Size of data:" << data.size() << " bytes";
    //qDebug() << "Client sent rawData:" << rawData;
    //qDebug() << "Size of package: " << size_pkg;
    qDebug() << "Written to socket" << sizeBlock << "bytes";*/
}

void MainWindow::DrawPlot()
{
    //qDebug() << "DrawPlot()";

    ui->customPlot->clearItems();

    CreateLines();
    if ( fVisibleLabels )
        CreateLabels();

    DrawThresholdWidget();

    CreateChannelLabels();
}

void MainWindow::slotConnected()
{
    qDebug() << "Received the connected() signal";
    qDebug() << "Connection successfull";
    ui->statusBar->showMessage("Клиент подключился к серверу.");
    ui->pb_startServer->setEnabled(true);
    vw->pbSaveConfig->setEnabled(true);
    ui->pbSetVoltage->setEnabled(true);
    ui->pbStartHVScan->setEnabled(true);
    ui->spVoltage->setEnabled(true);
    ui->pbStopSetVoltage->setEnabled(false);
    ui->actionConnect_to->setEnabled(false);
}

void MainWindow::slotDisconnected()
{
    ui->actionConnect_to->setEnabled(true);
    vw->pbSaveConfig->setEnabled(false);
    ui->pb_startServer->setEnabled(false);
    ui->pb_stopServer->setEnabled(false);
    ui->pbSetVoltage->setEnabled(false);
    ui->pbStartHVScan->setEnabled(false);
    ui->pbStopHVScan->setEnabled(false);
    ui->pbStopSetVoltage->setEnabled(false);
    ui->spVoltage->setEnabled(false);
    ui->statusBar->showMessage("Клиент отключился от сервера.");
}

void MainWindow::on_actionConnect_to_triggered()
{
    // Connect button action

    ip_dialog->setModal(true);
    if (ip_dialog->exec() == QDialog::Accepted)
        qDebug() << "Open modal window";
}

void MainWindow::connectToHost(QString str)
{
    // Connecting to server
    strHost = str;
    //qDebug() << "Mainwindow ip:" << strHost;

    m_pTcpSocket = new QTcpSocket(this);
    m_pTcpSocket->connectToHost(strHost, nPort);
    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pTcpSocket, SIGNAL(disconnected()), SLOT(slotDisconnected()));
}

void MainWindow::mouseWheel()
{
    ui->customPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
    ScaleChanged();
}

void MainWindow::on_pb_ZoomIn_clicked()
{
    ui->customPlot->xAxis->scaleRange(0.85, ui->customPlot->xAxis->range().lower);
    ui->customPlot->yAxis->scaleRange(0.85, ui->customPlot->yAxis->range().lower);
}

void MainWindow::on_pb_ZoomOut_clicked()
{
    ui->customPlot->xAxis->scaleRange(1.17, ui->customPlot->xAxis->range().center());
    ui->customPlot->yAxis->scaleRange(1.17, ui->customPlot->yAxis->range().center());
}

void MainWindow::on_pb_ResetRange_clicked()
{
    ui->customPlot->xAxis->setRange(0, 2500);
    ui->customPlot->yAxis->setRange(0, YupperBound);
}

void MainWindow::xAxisChanged(QCPRange newRange)
{
    QCPRange fixedRange(newRange);

    if (fixedRange.lower < XlowerBound)
    {
        fixedRange.lower = XlowerBound;
        fixedRange.upper = XlowerBound + newRange.size();
        if (fixedRange.upper > XupperBound || qFuzzyCompare(newRange.size(), XupperBound-XlowerBound))
            fixedRange.upper = XupperBound;
        ui->customPlot->xAxis->setRange(fixedRange);
    }
    else
        if (fixedRange.upper > XupperBound)
        {
            fixedRange.upper = XupperBound;
            fixedRange.lower = XupperBound - newRange.size();
            if (fixedRange.lower < XlowerBound || qFuzzyCompare(newRange.size(), XupperBound-XlowerBound))
                fixedRange.lower = XlowerBound;
            ui->customPlot->xAxis->setRange(fixedRange);
        }

    // Settings for Threshold Circle Displacement

    const double displacementCoefficient = 0.8;

    xPosOfCircle = fixedRange.center() +
            ( ( fixedRange.upper - fixedRange.center() ) * displacementCoefficient );

    /*
    qDebug() << "Lower bound:" << fixedRange.lower
             << "; Upper bound" << fixedRange.upper << ";";

    qDebug() << "Different between upper bound and lower bound:"
             << fixedRange.upper - fixedRange.lower << ";";

    qDebug() << "Position of circle:" << xPosOfCircle;
    qDebug() << "----------------------------------------------";
    */

}

void MainWindow::yAxisChanged(QCPRange newRange)
{
    QCPRange fixedRange(newRange);
    if (fixedRange.lower < YlowerBound)
    {
        fixedRange.lower = YlowerBound;
        fixedRange.upper = YlowerBound + newRange.size();
        if (fixedRange.upper > YupperBound || qFuzzyCompare(newRange.size(), YupperBound-YlowerBound))
            fixedRange.upper = YupperBound;
        ui->customPlot->yAxis->setRange(fixedRange);
    }
    else
    {
        if (fixedRange.upper > YupperBound)
        {
            fixedRange.upper = YupperBound;
            fixedRange.lower = YupperBound - newRange.size();
            if (fixedRange.lower < YlowerBound || qFuzzyCompare(newRange.size(), YupperBound-YlowerBound))
                fixedRange.lower = YlowerBound;
            ui->customPlot->yAxis->setRange(fixedRange);
        }
    }

    yAxisLowerBound = fixedRange.lower;

    // debug information
    //qDebug() << "\nlowerBound = " << fixedRange.lower;
}

void MainWindow::ScaleChanged()
{
    const unsigned char PixelLimit = 19;
    const unsigned short PixelLimitForLabels = 975;
    double px_size = ui->customPlot->xAxis->coordToPixel(ChannelsOnBoard) - ui->customPlot->xAxis->coordToPixel(0);

    //qDebug() << "px_size =" << px_size;

    if (px_size >= PixelLimit)
        fVisibleLabels = true;
    else
        fVisibleLabels = false;

    if (px_size >= PixelLimitForLabels)
        fVisibleNmChannels = true;
    else
        fVisibleNmChannels = false;

    ui->customPlot->replot();
}

void MainWindow::CreateLines()
{
    // Add QCPItemLine
    //const char line_height = 30;
    const unsigned char nmBoards = 49; // Number of boards
    const char width_line = 3;
    calculating_height_of_lines
            = ( ui->customPlot->yAxis->range().upper - ui->customPlot->yAxis->range().lower ) / 3;

    nmChannelsMutomo = nmBoards*ChannelsOnBoard;
    vecOfLines.resize(nmChannelsMutomo);

    const uint countOfChannelsInDetector = ChannelsOnBoard * 6;
    double heightOfLinesForDetectors = calculating_height_of_lines + calculating_height_of_lines/3;

    if (fDeviceGridVisible)
    {
        for (uint i = ChannelsOnBoard; i < nmChannelsMutomo; i += ChannelsOnBoard)
        {
            tickHLine = new QCPItemLine(ui->customPlot);
            vecOfLines.insert(i, tickHLine);
            ui->customPlot->addItem(tickHLine);
            tickHLine->start->setCoords(i, yAxisLowerBound);
            tickHLine->end->setCoords(i, yAxisLowerBound + calculating_height_of_lines );
            tickHLine->setPen(QPen(QColor(0, 255, 0), width_line));
            tickHLine->setLayer("belowmain");

        }
    }

    if (fDetecGridVisible)
    {
        for (uint i = countOfChannelsInDetector; i < nmChannelsMutomo; i += countOfChannelsInDetector)
        {
            tickBLine = new QCPItemLine(ui->customPlot);
            ui->customPlot->addItem(tickBLine);
            tickBLine->start->setCoords(i, yAxisLowerBound);
            tickBLine->end->setCoords(i, yAxisLowerBound + heightOfLinesForDetectors);
            tickBLine->setPen(QPen(QColor(0, 0, 255), width_line));
            tickBLine->setLayer("belowmain");
        }
    }

}

void MainWindow::CreateLabels()
{
    // Add a QCPItemText (number of MT48 on customPlot)
    unsigned char CounterOfBoards = 0; // Counter for boards
    const unsigned char nmBoards = 48; // Number of boards
    const short coord_label_center_x = ChannelsOnBoard / 2;
    double coord_label_center_y = calculating_height_of_lines / 4;

    //qDebug() << "label_center_y" << label_center_y;

    uint nmChannelsMutomo = nmBoards*ChannelsOnBoard;

    if (fDeviceGridVisible)
    {
        for (uint i = 0; i < nmChannelsMutomo; i += ChannelsOnBoard)
        {
            QString NOB = QString("%1").arg(CounterOfBoards);

            if (fVisibleLabels)
            {
                NumberOfBoard = new QCPItemText(ui->customPlot);
                ui->customPlot->addItem(NumberOfBoard);
                NumberOfBoard->position->setCoords(i + coord_label_center_x, yAxisLowerBound + coord_label_center_y);
                NumberOfBoard->setText(NOB);
                NumberOfBoard->setFont(QFont(font().family(), 11));

                if ( vectorForCheckingDevices[ CounterOfBoards ] == true )
                    NumberOfBoard->setColor(QColor(255, 0, 0));
                else
                    NumberOfBoard->setColor(QColor(0, 0, 0));

            }

            CounterOfBoards++;
        }
    }

}

void MainWindow::CreateChannelLabels()
{
    if (fVisibleNmChannels)
    {
        const short coord_label_center_y = calculating_height_of_lines / 4;
        int CounterForBoards = 0;

        for (uint i = leftBound; i <= rightBound; i ++)
        {
            QString nmChannel = QString::number(CounterForBoards);

            QCPItemText * nmChannelOnBoard = new QCPItemText(ui->customPlot);
            ui->customPlot->addItem(nmChannelOnBoard);
            nmChannelOnBoard->position->setCoords(i, yAxisLowerBound + coord_label_center_y);
            nmChannelOnBoard->setText(nmChannel);
            nmChannelOnBoard->setFont(QFont(font().family(), 10));
            nmChannelOnBoard->setColor(Qt::black);

            CounterForBoards++;
        }
    }
}

void MainWindow::DrawThresholdWidget()
{
    for (double i = 0; i < 2510; i += 2500 )
        threhshold_data->operator [](i) = QCPData(i, value_threshold);

    thresholdCircleData->operator [](0) = QCPData(xPosOfCircle, value_threshold);
    //qDebug() << "xPosOfCircle = " << xPosOfCircle;

    ui->customPlot->replot();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    ScaleChanged();

}

void MainWindow::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::WindowStateChange)
    {
        if(isMaximized())
        {
            ScaleChanged();
        }
    }
    event->accept();
}

void MainWindow::tabSelected()
{
    bUpdatePlot        = false;
    bUpdateViewConstr  = false;

    if (ui->tabWidget->currentIndex() == 0)
    {
        bUpdatePlot = true;
        ScaleChanged();
    }

    if (ui->tabWidget->currentIndex() == 1)
        ui->statusBar->showMessage("Переход на вкладку высоковольтного скана");

    if (ui->tabWidget->currentIndex() == 2)
        bUpdateViewConstr = true;

    ui->tabWidget->setFocus();
}

void MainWindow::MouseClickOnTextItem(QCPAbstractItem* item, QMouseEvent* event)
{
    if (item)
    {
        if("QCPItemText" == QString(item->metaObject()->className()))
        {
            double x = ui->customPlot->xAxis->pixelToCoord(event->x());
            double y = floor( x/ChannelsOnBoard );
            leftBound = y*ChannelsOnBoard;
            rightBound = leftBound + ChannelsOnBoard;

            ui->customPlot->xAxis->setRange(leftBound , rightBound);

            ui->customPlot->replot();
            ScaleChanged();
            CreateChannelLabels();
        }
    }
}

void MainWindow::MouseDoubleClickOnThresholdWidget(QCPAbstractPlottable *plot, QMouseEvent *event)
{
    if (thresholdCircle)
    {
        on_actionSettings_triggered();
    }
}

void MainWindow::MousePressed(QMouseEvent *event)
{
    double x = ui->customPlot->xAxis->pixelToCoord(event->pos().x());
    double y = ui->customPlot->yAxis->pixelToCoord(event->pos().y());

    double pixelAtY = event->pos().y();

    quint8 radius  = diamCircle/2;
    quint32 leftBoundX   = thresholdCircleData->first().key - radius;
    quint32 rightBoundX  = thresholdCircleData->first().key + radius;
    quint32 topBoundY    = thresholdCircleData->first().value + radius;
    quint32 bottomBoundY = thresholdCircleData->first().value - radius;

    if ( (x >= leftBoundX)   && (x <= rightBoundX) &&
         (y >= bottomBoundY) && (y <= topBoundY))
    {
        PressedOnCircle = true;

        lastPosition = pixelAtY;
        ui->customPlot->setInteractions(0x000);
    }

    /*qDebug() << "\nPressed on x:" << x;
    qDebug() << "Pressed on y:" << y;
    qDebug() << "leftBoundX:" << leftBoundX;
    qDebug() << "rightBoundX:" << rightBoundX;
    qDebug() << "topBoundY:" << topBoundY;
    qDebug() << "bottomBoundY:" << bottomBoundY;
    qDebug() << "X center of circle: " << thresholdCircleData->first().key;
    qDebug() << "Y center of circle: " << thresholdCircleData->first().value;*/
}

void MainWindow::MoveThreshold(QMouseEvent *event)
{
    if (PressedOnCircle)
    {
        const quint8 pixelLimitToMoveCircle = 5;
        const quint8 yBoundForCircle = 20;

        double y = ui->customPlot->yAxis->pixelToCoord(event->pos().y());
        double pixelAtYNew = event->pos().y();

        if ( y <= YupperBound && y >= yBoundForCircle )
        {
            value_threshold = y;

            bool f_replot;
            if (abs(lastPosition - pixelAtYNew) > pixelLimitToMoveCircle)
            {
                f_replot = true;
                lastPosition = pixelAtYNew;
            }
            else
            {
                f_replot = false;
            }

            if (f_replot)
            {
                for (double i = 0; i < 2510; i += 2500 )
                    threhshold_data->operator [](i) = QCPData(i, value_threshold);

                thresholdCircleData->operator [](0) = QCPData(xPosOfCircle, value_threshold);
                ui->customPlot->replot();
                ui->statusBar->showMessage("Пороговое значение равно " + QString::number(value_threshold) + ".");
            }
        }
    }
}

void MainWindow::MouseRealesed(QMouseEvent *event)
{
    if ( PressedOnCircle )
    {
        PressedOnCircle = false;
        ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom );
    }

    //get_threshold(value_threshold,qsettings.value("settings/yUpperBound").toInt());
}

void MainWindow::on_actionSettings_triggered()
{
    // Settings button action

    settings_dialog->sp_thresholdValue->setValue( value_threshold );
    settings_dialog->sp_yUpperBound->setValue( YupperBound );

    if (settings_dialog->exec() == QDialog::Accepted)
    {
        value_threshold = settings_dialog->sp_thresholdValue->value();
        YupperBound = settings_dialog->sp_yUpperBound->value();

        //qDebug() << "New value of value_threshold = " << value_threshold;
        //qDebug() << "Open modal window thershold settings";

        get_threshold();
    }
}

void MainWindow::get_threshold()
{
    ui->statusBar->showMessage("Новое пороговое значение равно " + QString::number(value_threshold));
    ui->customPlot->yAxis->setRange( 0, YupperBound );

    DrawThresholdWidget();
}

void MainWindow::slotMessage(QString str)
{
    ui->statusBar->showMessage(str);
}

void MainWindow::check_DetecGrid(bool checked_state)
{
    fDetecGridVisible = checked_state;
    ui->customPlot->replot();
}

void MainWindow::check_DeviceGrid(bool checked_state)
{
    fDeviceGridVisible = checked_state;
    ui->customPlot->replot();
}

void MainWindow::ChannelCheck(quint32 freq, int ind)
{
    // выявление плат с каналами, в которых частота регистрации сигналов
    // превышает порог

    if (freq > value_threshold)
    {
        numberOfBrokenDevice = ind / ChannelsOnBoard;
        vw->VectorOfbadBoards[numberOfBrokenDevice] = 1;
        vectorForCheckingDevices[numberOfBrokenDevice] = 1;
    }
}

void MainWindow::ClearVectorForCheckingDevices()
{
    //Очистка массива, отвечающего за проверку исправности плат MT48
    for (int i = 0; i < 48; i++)
        vectorForCheckingDevices[i] = 0;
}
