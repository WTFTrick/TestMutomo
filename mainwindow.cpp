#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork>
#include <QtGui>

MainWindow::MainWindow() :
    nPort(2323),
    m_nNextBlockSize(0),
    vectorForCheckingDevices(48),
    ChannelsOnBoard(49),
    numberOfBrokenDevice(0),
    LinesCount(49),
    diamCircle(20),
    xPosOfCircle(2300),
    qsettings("settings.conf", QSettings::NativeFormat),
    PressedOnCircle(false),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Mutomo Client");
    setCentralWidget(ui->tabWidget);

    thresholdGraph = ui->customPlot->addGraph();
    threhshold_data = thresholdGraph->data();

    YlowerBound = 0;
    XlowerBound = 0;
    XupperBound = 2400;

    if ( ! QFile::exists("settings.conf") )
    {
        YupperBound = 150;
        strHost = "0.0.0.0";
        value_threshold = 100;
        ui->statusBar->showMessage("Application run. Threshold =" + QString::number(value_threshold));
    }
    else
    {
        get_threshold(qsettings.value("settings/threshold").toInt(), qsettings.value("settings/yUpperBound").toInt());
        connectToHost(qsettings.value("settings/IP").toString());
        ui->statusBar->showMessage("Application run. Threshold value is " + qsettings.value("settings/threshold").toString() + ".");
    }

    CreateThresholdLine();

    graph1 = ui->customPlot->addGraph();
    mapData = graph1->data();

    ui->customPlot->installEventFilter(this);

    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom );
    ui->customPlot->xAxis->setLabel("Номер канала");
    ui->customPlot->yAxis->setLabel("Частота, HZ");
    ui->customPlot->xAxis->setRange(XlowerBound, XupperBound);
    ui->customPlot->yAxis->setRange(YlowerBound, YupperBound);


    graph1->setLineStyle((QCPGraph::LineStyle)(2));

    ui->customPlot->addLayer("BG",ui->customPlot->layer("graph1"), QCustomPlot::limBelow);
    ui->customPlot->setCurrentLayer("BG");
    ui->pb_stopServer->setDisabled(true);

    vw = new viewConstr( this );
    ui->tabWidget->addTab(vw, tr("Configuration"));
    ui->pb_ResetRange->setFocus();

    fVisibleLabels = false;

    // Colour and width of graph
    QPen graphPen;
    graphPen.setColor(QColor(0, 0, 0));
    graphPen.setWidthF(0.4);
    graph1->setPen(graphPen);
    ui->customPlot->replot();

    ip_dialog = new IPDialog( this );
    settings_dialog = new settings( this );

    //MutomoHost = "10.162.1.110"

    CreateConnections();

    bUpdatePlot = true;
    bUpdateViewConstr  = false;

    ClearVectorForCheckingDevices();

    ui->tabWidget->setFocus();


    CreateThresholdDragCircle();

#ifdef ANDROID

    QApplication::setStyle("fusion");

    QFile f(":qstyle/StyleSheet.qss");
    if (!f.exists())
    {
        ui->statusBar->showMessage("Unable to set stylesheet, file not found\n");
    }
    else
    {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }

#else
    //slotMessage("Run on Desktop");
#endif

}

MainWindow::~MainWindow()
{
    //StopServer();

    qsettings.sync();
    delete vw;
    delete settings_dialog;
    delete ip_dialog;
    close();

    delete ui;
}

void MainWindow::CreatePlot(QVector<quint32> *arrData)
{
    ui->pb_startServer->setEnabled(false);
    ui->pb_stopServer->setEnabled(true);
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

void MainWindow::StartServer()
{
    //Команда - начать генерацию данных на сервере и передачу клиенту

    quint32 data = 1;
    TYPE_DATA t_data = DATA_CMD;
    QByteArray arrayStart;
    //arrayStart.setNum(t_data);
    //arrayStart.setNum(data);

    QDataStream out(&arrayStart, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_6);

    out << quint64(0);

    out << t_data;
    out << data;

    out.device()->seek(0);
    quint64 size_pkg = quint64(arrayStart.size() - sizeof(quint64));
    out << size_pkg;

    qDebug() << arrayStart.toUInt();

    //arrayStart += data;
    //qDebug() << "size of QByteArray arrayStart = " << arrayStart.size();
    //qDebug() << "\n";

    if (m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        DataToServer(t_data, arrayStart);

        ui->pb_stopServer->setDisabled(false);
        ui->pb_startServer->setDisabled(true);
        ui->tabWidget->setFocus();
        ui->statusBar->showMessage("Send 'start' command to server");
    }
    else
        ui->statusBar->showMessage("Client unconnected!");
}

void MainWindow::StopServer()
{
    //Команда - остановить генерацию данных на сервере и передачу клиенту

    quint32 data = 0;
    QByteArray arrayStop;
    TYPE_DATA t_data = DATA_CMD;
    arrayStop.setNum(t_data);
    arrayStop.setNum(data);


    if (m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        DataToServer(t_data, arrayStop);
        ui->pb_startServer->setDisabled(false);
        ui->pb_stopServer->setDisabled(true);
        ui->tabWidget->setFocus();
        ui->statusBar->showMessage("Send 'stop' command to server");
    }
    else
        ui->statusBar->showMessage("Client unconnected!");
}

void MainWindow::DataHistRequest()
{
    //Запросить у сервера данные для гистограммы

    TYPE_DATA t_data = DATA_HIST;
    QByteArray null;
    null.setNum(0);
    if (m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
        DataToServer(t_data, null );
    else
        ui->statusBar->showMessage("Client unconnected!");
}

void MainWindow::DataRawRequset()
{
    //Запросить у сервера сырые данные

    TYPE_DATA t_data = DATA_RAW;
    QByteArray null;
    null.setNum(0);
    if (m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
        DataToServer(t_data, null );
    else
        ui->statusBar->showMessage("Client unconnected!");
}

void MainWindow::GetJsonFromViewConstr(QByteArray JsonDoc)
{
    //Передать серверу конфигурацию в виде JSON

    TYPE_DATA t_data = DATA_CONFIG_MUTOMO;

    //qDebug() << JsonDoc;

    if (m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        DataToServer(t_data, JsonDoc);
        ui->statusBar->showMessage("Send configuration to server");
    }
    else
        ui->statusBar->showMessage("Client unconnected!");
}

void MainWindow::DataToServer(TYPE_DATA t_data, QByteArray data)
{
    //  Передача данных серверу
    //  Блок данных |Size|Type|Data|

    //Передавать всё в QByteArray

    QByteArray rawData;
    QDataStream out(&rawData, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_6);

    //  Size
    out << quint64(0);

    //  Type
    //out << t_data;

    //  Data
    out << data;

    out.device()->seek(0);
    quint64 size_pkg = quint64(rawData.size() - sizeof(quint64));
    out << size_pkg;

    quint64 sizeBlock = m_pTcpSocket->write(rawData);

    qDebug() << "Client sent type:" << t_data;
    qDebug() << "Size of t_data:" << sizeof( t_data) << " bytes";
    qDebug() << "Client sent data:" << data;
    qDebug() << "Size of data:" << data.size() << " bytes";
    qDebug() << "Size of package: " << size_pkg;
    qDebug() << "Written to socket" << sizeBlock << "bytes";
}

void MainWindow::DrawPlot()
{
    //qDebug() << "DrawPlot()";

    ui->customPlot->clearItems();

    CreateLines();
    if ( fVisibleLabels )
        CreateLabels();
}

void MainWindow::slotConnected()
{
    qDebug() << "Received the connected() signal";
    qDebug() << "Connection successfull";
    ui->statusBar->showMessage("Connection to server successfull");
}

void MainWindow::slotDisconnected()
{
    ui->statusBar->showMessage("Disconnected");
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
    qsettings.setValue("settings/IP", str);
    //qDebug() << "Mainwindow ip:" << strHost;

    m_pTcpSocket = new QTcpSocket(this);
    m_pTcpSocket->connectToHost(strHost, nPort);
    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pTcpSocket, SIGNAL(disconnected()), SLOT(slotReadyRead()));
}

void MainWindow::mousePress()
{
    ui->customPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
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
    ui->customPlot->yAxis->setRange(0, 100);
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
    } else if (fixedRange.upper > XupperBound)
    {
        fixedRange.upper = XupperBound;
        fixedRange.lower = XupperBound - newRange.size();
        if (fixedRange.lower < XlowerBound || qFuzzyCompare(newRange.size(), XupperBound-XlowerBound))
            fixedRange.lower = XlowerBound;
        ui->customPlot->xAxis->setRange(fixedRange);
    }
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
    double px_size = ui->customPlot->xAxis->coordToPixel(ChannelsOnBoard) - ui->customPlot->xAxis->coordToPixel(0);
    //qDebug() << "px_size =" << px_size;

    if (px_size >= PixelLimit)
        fVisibleLabels = true;
    else
        fVisibleLabels = false;

    ui->customPlot->replot();
}

void MainWindow::CreateLines()
{
    // Add QCPItemLine
    const unsigned char nmBoards = 48; // Number of boards
    const char line_height = 30;
    const char width_line = 3;

    uint nmChannelsMutomo = nmBoards*ChannelsOnBoard;

    for (uint i = 0; i < nmChannelsMutomo; i += ChannelsOnBoard)
    {
        QCPItemLine *tickHLine = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(tickHLine);
        tickHLine->start->setCoords(i, yAxisLowerBound);
        tickHLine->end->setCoords(i, yAxisLowerBound + line_height);
        tickHLine->setPen(QPen(QColor(0, 255, 0), width_line));
    }

    //qDebug() << "y_1 = " << yAxisLowerBound << " y_2 = " << yAxisLowerBound + line_height;
}

void MainWindow::CreateThresholdLine()
{
    for (double i = 0; i < 2510; i += 2500 )
        threhshold_data->operator [](i) = QCPData(i, value_threshold);

    QPen pen;
    pen.setColor(QColor(255,0,0));
    pen.setWidth(2);
    pen.setStyle(Qt::DotLine);
    thresholdGraph->setPen(pen);

    ui->customPlot->replot();
}

void MainWindow::CreateThresholdDragCircle()
{
    thresholdCircle = ui->customPlot->addGraph();
    thresholdCircleData = thresholdCircle->data();

    //for (double i = 0; i < 2; i ++ )
    thresholdCircleData->operator [](0) = QCPData(xPosOfCircle, value_threshold);

    thresholdCircle->setPen(QPen(Qt::red));
    thresholdCircle->setLineStyle(QCPGraph::lsNone);
    QCPScatterStyle ss_disc( QCPScatterStyle::ssCircle, diamCircle);
    thresholdCircle->setScatterStyle(ss_disc);
}

void MainWindow::CreateLabels()
{
    // Add a QCPItemText (number of MT48 on customPlot)
    unsigned char CounterOfBoards = 0; // Counter for boards
    const unsigned char nmBoards = 48; // Number of boards
    const short label_center_x = ChannelsOnBoard / 2;
    const char label_center_y = 7;
    uint nmChannelsMutomo = nmBoards*ChannelsOnBoard;

    for (uint i = 0; i < nmChannelsMutomo; i += ChannelsOnBoard)
    {
        QString NOB = QString("%1").arg(CounterOfBoards);

        if (fVisibleLabels)
        {
            NumberOfBoard = new QCPItemText(ui->customPlot);
            ui->customPlot->addItem(NumberOfBoard);
            NumberOfBoard->position->setCoords(i + label_center_x, yAxisLowerBound + label_center_y);
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

void MainWindow::CreateConnections()
{
    //qDebug() << "CreateConnections";

    // connections between mainWindow and modal dialogs
    connect(ip_dialog, SIGNAL(sendData(QString)), this, SLOT(connectToHost(QString)));
    connect(settings_dialog, SIGNAL(sendThreshold(quint16,quint16)), this, SLOT(get_threshold(quint16,quint16)));

    // QCustomPlot connects
    connect(ui->customPlot, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(MouseRealesed(QMouseEvent*)));
    connect(ui->customPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(MoveThreshold(QMouseEvent*)));
    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
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
    connect(ui->customPlot, SIGNAL(plottableClick(QCPAbstractPlottable*,QMouseEvent*)), this, SLOT(MouseOnDragCircleClicked(QCPAbstractPlottable*, QMouseEvent*)));
    // start/stop server buttons
    connect(ui->pb_startServer, SIGNAL(clicked(bool)), this, SLOT(StartServer()));
    connect(ui->pb_stopServer, SIGNAL(clicked(bool)), this, SLOT(StopServer()));

    // tab changed event
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabSelected()));

    // info from viewConstr class to MainWindow StatusBar
    connect(vw, SIGNAL(messg(QString)), this, SLOT(slotMessage(QString)));

    // replot event
    connect(ui->customPlot, SIGNAL(beforeReplot()), this, SLOT(DrawPlot()));

    connect(vw, SIGNAL(sendJson(QByteArray)), this, SLOT(GetJsonFromViewConstr(QByteArray)));
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
        bUpdateViewConstr = true;
}

void MainWindow::MouseClickOnTextItem(QCPAbstractItem* item, QMouseEvent* event)
{
    if (item)
    {
        if("QCPItemText" == QString(item->metaObject()->className()))
        {
            double x = ui->customPlot->xAxis->pixelToCoord(event->x());
            double y = floor( x/ChannelsOnBoard );
            double left = y*ChannelsOnBoard;
            //qDebug() << y;
            double right = left + ChannelsOnBoard;
            ui->customPlot->xAxis->setRange(left , right);
            ui->customPlot->replot();
        }
    }

}

void MainWindow::MouseOnDragCircleClicked(QCPAbstractPlottable *plot, QMouseEvent *event)
{
    if (plot == thresholdCircle)
    {
        //qDebug() << "Clicked on Threshold Circle";
        //QMouseEvent *_mouseEvent = static_cast<QMouseEvent*>(event);
    }
}

void MainWindow::MousePressed(QMouseEvent *event)
{
    qDebug() << "event->x()" << event->x();
    qDebug() << "event->pos().x()" << event->pos().x();

    double x = ui->customPlot->xAxis->pixelToCoord(event->pos().x());
    double y = ui->customPlot->yAxis->pixelToCoord(event->pos().y());

    quint8 radius  = diamCircle/2;
    quint32 leftBoundX   = thresholdCircleData->first().key - radius;
    quint32 rightBoundX  = thresholdCircleData->first().key + radius;
    quint32 topBoundY    = thresholdCircleData->first().value + radius;
    quint32 bottomBoundY = thresholdCircleData->first().value - radius;
    if ( (x >= leftBoundX)   && (x <= rightBoundX) &&
         (y >= bottomBoundY) && (y <= topBoundY))
    {
        //qDebug() << thresholdCircleData->first().value;
        //qDebug() << thresholdCircleData->first().key;
        qDebug() << "\n====== In circle ======\n";
        ui->statusBar->showMessage("In circle");
        PressedOnCircle = true;
    }
    else
    {
        false;
    }

    qDebug() << "\nPressed on x:" << x;
    qDebug() << "Pressed on y:" << y;
    qDebug() << "leftBoundX:" << leftBoundX;
    qDebug() << "rightBoundX:" << rightBoundX;
    qDebug() << "topBoundY:" << topBoundY;
    qDebug() << "bottomBoundY:" << bottomBoundY;
    qDebug() << "X center of circle: " << thresholdCircleData->first().key;
    qDebug() << "Y center of circle: " << thresholdCircleData->first().value;
}

void MainWindow::MoveThreshold(QMouseEvent *event)
{
    if (PressedOnCircle)
    {
        double y = ui->customPlot->yAxis->pixelToCoord(event->pos().y());
        for (double i = 0; i < 2510; i += 2500 )
            threhshold_data->operator [](i) = QCPData(i, y);

        thresholdCircleData->operator [](0) = QCPData(xPosOfCircle, y);

        ui->customPlot->replot();
        value_threshold = y;
    }
}

void MainWindow::MouseRealesed(QMouseEvent *event)
{
    PressedOnCircle = false;
}

bool MainWindow::eventFilter(QObject *target, QEvent *event)
{
    if(target == thresholdCircle && event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *_mouseEvent = static_cast<QMouseEvent*>(event);



        /*
            Here you have mouseEvent object so you have x,y coordinate of
            MouseEvent.
            Now if you want to convert x,y coordiante of mouse to plot coordinates
            you can easily use QCPAxis::pixelToCoord() method.
        */
    }
    return false;
}

void MainWindow::on_actionSettings_triggered()
{
    // Settings button action

    settings_dialog->setModal(true);
    if (settings_dialog->exec() == QDialog::Accepted)
        qDebug() << "Open modal window thershold settings";
}

void MainWindow::get_threshold(quint16 threshold, quint16 yUpperBound)
{
    value_threshold = threshold;
    YupperBound = yUpperBound;
    qsettings.setValue("settings/threshold", threshold);
    qsettings.setValue("settings/yUpperBound", yUpperBound);
    QString sThr = QString::number(value_threshold);
    ui->statusBar->showMessage("New threshold = " + sThr);
    ui->customPlot->yAxis->setRange( 0, YupperBound );
    CreateThresholdLine();
}

void MainWindow::slotMessage(QString str)
{
    ui->statusBar->showMessage(str);
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

