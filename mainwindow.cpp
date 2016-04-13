#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork>
#include <QtGui>

MainWindow::MainWindow() : nPort(2323), m_nNextBlockSize(0),vectorForCheckingDevices(49), ChannelsOnBoard(49),numberOfBrokenDevice(0),
    LinesCount(49), qsettings("settings.conf" ,QSettings::NativeFormat), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Mutomo Client");
    setCentralWidget(ui->tabWidget);

    graph1 = ui->customPlot->addGraph();
    mapData = graph1->data();
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom );
    ui->customPlot->xAxis->setLabel("Номер канала");
    ui->customPlot->yAxis->setLabel("Частота, HZ");
    ui->customPlot->xAxis->setRange(0, 2500);
    ui->customPlot->yAxis->setRange(0, 100);

    ui->customPlot->graph()->setLineStyle((QCPGraph::LineStyle)(2));

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
    ui->customPlot->graph()->setPen(graphPen);
    ui->customPlot->replot();

    ip_dialog = new IPDialog( this );
    settings_dialog = new settings( this );

    //MutomoHost = "10.162.1.110"
    connectToHost(qsettings.value("section/IP").toString());
    get_threshold(qsettings.value("section/threshold").toInt());

    CreateConnections();

    bUpdatePlot = true;
    bUpdateViewConstr  = false;

    ClearVectorForCheckingDevices();

    ui->tabWidget->setFocus();
    ui->statusBar->showMessage("Application run. Threshold =" + qsettings.value("section/threshold").toString());
}

MainWindow::~MainWindow()
{
    delete vw;
    qsettings.sync();
    StopServer();
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

        if ( fVisibleLabels )
            CreateLabels();

        arrFreq.clear();
        m_nNextBlockSize = 0;
    }
}

void MainWindow::StartServer()
{
    //Команда - начать генерацию данных на сервере и передачу клиенту

    quint8 data = 1;
    TYPE_DATA t_data = CMD;
    DataToServer(t_data, data);

    ui->pb_stopServer->setDisabled(false);
    ui->pb_startServer->setDisabled(true);
    ui->tabWidget->setFocus();
    ui->statusBar->showMessage("Send 'start' command to server");
}

void MainWindow::StopServer()
{
    //Команда - остановить генерацию данных на сервере и передачу клиенту

    quint8 data = 0;
    TYPE_DATA t_data = CMD;
    DataToServer(t_data, data);

    ui->pb_startServer->setDisabled(false);
    ui->pb_stopServer->setDisabled(true);
    ui->tabWidget->setFocus();
    ui->statusBar->showMessage("Send 'stop' command to server");
}

void MainWindow::DataHistRequest()
{
    //Запросить у сервера данные для гистограммы

    TYPE_DATA t_data = DATA_HIST;
    DataToServer(t_data, 0 );
}

void MainWindow::DataRawRequset()
{
    //Запросить у сервера сырые данные

    TYPE_DATA t_data = DATA_RAW;
    DataToServer(t_data, 0 );
}

void MainWindow::GetJsonFromViewConstr(QString JsonDoc)
{
    //Передать серверу конфигурацию в виде JSON

    TYPE_DATA t_data = CFG_MUTOMO;
    //DataToServer(t_data, JsonDoc);

    qDebug() << "MainWindow Get JsonDoc";
}

void MainWindow::DataToServer(TYPE_DATA t_data, quint32 data)
{
    //  Передача данных серверу
    //  Блок данных |Size|Type|Data|

    QByteArray rawData;
    QDataStream out(&rawData, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_6);

    //  Size
    out << quint64(0);

    //  Type
    quint8 t = t_data;
    //out << t_data;
    out << t;

    //  Data
    out << data;


    out.device()->seek(0);
    quint64 size_pkg = quint64(rawData.size() - sizeof(quint64));
    out << size_pkg;

    quint64 sizeBlock = m_pTcpSocket->write(rawData);


    /*qDebug() << "Client received type:" << t;
    qDebug() << "Client received data:" << data;
    qDebug() << "Size of package: " << size_pkg;
    qDebug() << "Written to socket" << sizeBlock << "bytes.";*/
}

void MainWindow::slotConnected()
{
    qDebug() << "Received the connected() signal";
    qDebug() << "Connection successfull";
    ui->statusBar->showMessage("Connection to server successfull");
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
    qsettings.setValue("section/IP", str);
    qDebug() << "Mainwindow ip:" << strHost;


    m_pTcpSocket = new QTcpSocket(this);
    m_pTcpSocket->connectToHost(strHost, nPort);
    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
}

void MainWindow::mousePress()
{
    ui->customPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::mouseWheel()
{
    ui->customPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
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
    double lowerBound = 0;
    double upperBound = 2531;
    QCPRange fixedRange(newRange);
    if (fixedRange.lower < lowerBound)
    {
        fixedRange.lower = lowerBound;
        fixedRange.upper = lowerBound + newRange.size();
        if (fixedRange.upper > upperBound || qFuzzyCompare(newRange.size(), upperBound-lowerBound))
            fixedRange.upper = upperBound;
        ui->customPlot->xAxis->setRange(fixedRange);
    } else if (fixedRange.upper > upperBound)
    {
        fixedRange.upper = upperBound;
        fixedRange.lower = upperBound - newRange.size();
        if (fixedRange.lower < lowerBound || qFuzzyCompare(newRange.size(), upperBound-lowerBound))
            fixedRange.lower = lowerBound;
        ui->customPlot->xAxis->setRange(fixedRange);
    }
}

void MainWindow::yAxisChanged(QCPRange newRange)
{
    double lowerBound = 0;
    double upperBound = 150;
    QCPRange fixedRange(newRange);
    if (fixedRange.lower < lowerBound)
    {
        fixedRange.lower = lowerBound;
        fixedRange.upper = lowerBound + newRange.size();
        if (fixedRange.upper > upperBound || qFuzzyCompare(newRange.size(), upperBound-lowerBound))
            fixedRange.upper = upperBound;
        ui->customPlot->yAxis->setRange(fixedRange);
    }
    else
        if (fixedRange.upper > upperBound)
        {
            fixedRange.upper = upperBound;
            fixedRange.lower = upperBound - newRange.size();
            if (fixedRange.lower < lowerBound || qFuzzyCompare(newRange.size(), upperBound-lowerBound))
                fixedRange.lower = lowerBound;
            ui->customPlot->yAxis->setRange(fixedRange);
        }

    // newRange.lower //вызов
    CreateLines(newRange.lower);
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

    CreateLabels();
}

void MainWindow::CreateLines(double lowerBound)
{
    // Add QCPItemLine
    const unsigned char nmBoards = 48; // Number of boards
    const char line_height = 30;
    const char width_line = 3;

    uint nmChannelsMutomo = nmBoards*ChannelsOnBoard;

    //qDebug() << "LowerBound:" << lowerBound;
    if (lowerBound >= 30)
    {
        for (uint i = 0; i < nmChannelsMutomo; i += ChannelsOnBoard)
        {
            QCPItemLine *tickHLine = new QCPItemLine(ui->customPlot);
            ui->customPlot->addItem(tickHLine);
            tickHLine->start->setCoords(i, lowerBound);
            tickHLine->end->setCoords(i, line_height + lowerBound);
            tickHLine->setPen(QPen(QColor(0, 255, 0), width_line));
        }
    }

    if (lowerBound <= 30)
    {
        for (uint i = 0; i < nmChannelsMutomo; i += ChannelsOnBoard)
        {
            QCPItemLine *tickHLine = new QCPItemLine(ui->customPlot);
            ui->customPlot->addItem(tickHLine);
            tickHLine->start->setCoords(i, 0);
            tickHLine->end->setCoords(i, line_height);
            tickHLine->setPen(QPen(QColor(0, 255, 0), width_line));
        }
    }

    ui->customPlot->replot();
}

void MainWindow::CreateLabels()
{
    // Add a QCPItemText (number of MT48 on customPlot)

    ui->customPlot->clearItems();
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
            NumberOfBoard->position->setCoords(i + label_center_x, label_center_y);
            NumberOfBoard->setText(NOB);
            NumberOfBoard->setFont(QFont(font().family(), 11));

            if ( vectorForCheckingDevices[ CounterOfBoards ] == true )
                NumberOfBoard->setColor(QColor(255, 0, 0));
            else
                NumberOfBoard->setColor(QColor(0, 0, 0));
        }

        CounterOfBoards++;
    }

    ui->customPlot->replot();
}

void MainWindow::CreateConnections()
{
    //qDebug() << "CreateConnections";

    // Connections between mainWindow and modal dialog ('connect to...')
    connect(ip_dialog, SIGNAL(sendData(QString)), this, SLOT(connectToHost(QString)));
    connect(settings_dialog, SIGNAL(sendThreshold(int)), this, SLOT(get_threshold(int)));
    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(ui->customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(yAxisChanged(QCPRange)));

    // scaled plot
    connect(ui->pb_ZoomIn, SIGNAL(clicked(bool)), this, SLOT(ScaleChanged()));
    connect(ui->pb_ZoomOut, SIGNAL(clicked(bool)), this, SLOT(ScaleChanged()));
    connect(ui->pb_ResetRange, SIGNAL(clicked(bool)), this, SLOT(ScaleChanged()));
    connect(ui->customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(ScaleChanged()));

    // mouse click on some area of plot
    connect(ui->customPlot, SIGNAL(itemClick(QCPAbstractItem*,QMouseEvent*)), this,SLOT(MousePress(QCPAbstractItem* , QMouseEvent*)));

    // start/stop server buttons
    connect(ui->pb_startServer, SIGNAL(clicked(bool)), this, SLOT(StartServer()));
    connect(ui->pb_stopServer, SIGNAL(clicked(bool)), this, SLOT(StopServer()));


    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabSelected()));

    connect(vw, SIGNAL(messg(QString)), this, SLOT(slotMessage(QString)));
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    ScaleChanged();
    CreateLines(0);
}

void MainWindow::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::WindowStateChange)
    {
        if(isMaximized())
        {
            ScaleChanged();
            CreateLines(0);
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

void MainWindow::MousePress(QCPAbstractItem* item, QMouseEvent* event)
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

void MainWindow::on_actionSettings_triggered()
{
    // Settings button action

    settings_dialog->setModal(true);
    if (settings_dialog->exec() == QDialog::Accepted)
        qDebug() << "Open modal window thershold settings";
}

void MainWindow::get_threshold(int threshold)
{
    value_threshold = threshold;
    qsettings.setValue("section/threshold", threshold);
    QString tresh_val = QString::number(value_threshold);
    ui->statusBar->showMessage("Threshold = " + tresh_val);
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

/*bool MainWindow::event(QEvent *event)
{
    //qDebug() << "MainWindow::event()";

    if ( event->type () == QEvent::OrientationChange)
        //ui->statusBar->showMessage("Orientation changed");

    return QMainWindow::event(event);
}
*/
