#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork>
#include <QtGui>

MainWindow::MainWindow() : nPort(2323), m_nNextBlockSize(0),ChannelsOnBoard(49),numberOfBrokenDevice(100), LinesCount(48), ui(new Ui::MainWindow)
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

    connectToHost("0.0.0.0");
    get_threshold(0);
    //MutomoHost = "10.162.1.110"
    CreateConnections();


    bUpdatePlot = true;
}

MainWindow::~MainWindow()
{
    delete vw;

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

    // заполнить контейнер BadBoards нулями

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

        quint32  freq = 0;
        QVector<quint32> arrFreq;
        for (int i = 0; i < m_nNextBlockSize/4; i++ )
        {
            in >> freq;
            arrFreq.append( freq );

            // проверку
            // inline void checkChannel();
            // checkChannel( freq, i );
            ChannelCheck(freq, i);
        }

        if ( bUpdatePlot )
            CreatePlot( &arrFreq );
        else
            qDebug() << "bUpdatePlot = false!";

        arrFreq.clear();
        m_nNextBlockSize = 0;
    }
}

void MainWindow::StartServer()
{
    quint8 status = 1;
    ServerControl(status);
    qDebug() << "Send 'start' command to server";
    ui->pb_stopServer->setDisabled(false);
    ui->pb_startServer->setDisabled(true);
    ui->tabWidget->setFocus();
}

void MainWindow::StopServer()
{
    quint8 status = 0;
    ServerControl(status);
    qDebug() << "Send 'stop' command to server";
    ui->pb_startServer->setDisabled(false);
    ui->pb_stopServer->setDisabled(true);
    ui->tabWidget->setFocus();
}

void MainWindow::ServerControl(quint8 status)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_4);

    out << quint8(0) << status;
    out.device()->seek(0);

    out << quint8(data.size() - sizeof(quint8));
    m_pTcpSocket->write(data);
}

void MainWindow::slotConnected()
{
    qDebug() << "Received the connected() signal";
    qDebug() << "Connection successfull";
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
    //ScaleChanged();
}

void MainWindow::on_pb_ResetRange_clicked()
{
    ui->customPlot->xAxis->setRange(0, 2500);
    ui->customPlot->yAxis->setRange(0, 100);
    //ScaleChanged();
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
    } else if (fixedRange.upper > upperBound)
    {
        fixedRange.upper = upperBound;
        fixedRange.lower = upperBound - newRange.size();
        if (fixedRange.lower < lowerBound || qFuzzyCompare(newRange.size(), upperBound-lowerBound))
            fixedRange.lower = lowerBound;
        ui->customPlot->yAxis->setRange(fixedRange);
    }
}

void MainWindow::ScaleChanged()
{
    const unsigned char PixelLimit = 19;
    double px_size = ui->customPlot->xAxis->coordToPixel(ChannelsOnBoard) - ui->customPlot->xAxis->coordToPixel(0);
    //qDebug() << "px_size =" << px_size;

    if (px_size > PixelLimit)
        fVisibleLabels = true;
    else
        if (px_size < PixelLimit)
            fVisibleLabels = false;

    CreateLines();
}

void MainWindow::CreateLines()
{
    // delete items
    ui->customPlot->clearItems();

    // Add QCPItemLine and QCPItemText
    unsigned char CounterOfBoards = 0;           // Counter for boards
    const unsigned char nmBoards = 48;          //  Number of boards
    const char line_height = 30;
    const char width_line = 3;
    const short label_center_x = ChannelsOnBoard / 2;
    const char label_center_y = 7;

    uint nmChannelsMutomo = nmBoards*ChannelsOnBoard;
    for (uint i = 0; i < nmChannelsMutomo; i += ChannelsOnBoard)
    {
        QCPItemLine *tickHLine = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(tickHLine);
        tickHLine->start->setCoords(i, 0);
        tickHLine->end->setCoords(i, line_height);
        tickHLine->setPen(QPen(QColor(0, 255, 0), width_line));

        CounterOfBoards++;
        QString NOB = QString("%1").arg(CounterOfBoards);
        if (fVisibleLabels)
        {
            QCPItemText *NumberOfBoard = new QCPItemText(ui->customPlot);
            ui->customPlot->addItem(NumberOfBoard);
            NumberOfBoard->position->setCoords(i + label_center_x, label_center_y);
            NumberOfBoard->setText(NOB);
            NumberOfBoard->setFont(QFont(font().family(), 11));
            //NumberOfBoard->setPadding(QMargins(10, 0, 0, 0));
        }
    }
    //qDebug() << "fVisibleLabels=" << fVisibleLabels;

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

    // mouse click on some area of plot
    connect(ui->customPlot, SIGNAL(itemClick(QCPAbstractItem*,QMouseEvent*)), this,SLOT(MousePress(QCPAbstractItem* , QMouseEvent*)));

    // start/stop server buttons
    connect(ui->pb_startServer, SIGNAL(clicked(bool)), this, SLOT(StartServer()));
    connect(ui->pb_stopServer, SIGNAL(clicked(bool)), this, SLOT(StopServer()));


    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabSelected()));
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
    if(ui->tabWidget->currentIndex()!=0)
    {
        // не обновлять график
        bUpdatePlot = false;
        //qDebug() << "bUpdatePlot =" << bUpdatePlot;

        //vw->BrokenDevice();
    }
    else
    {
        bUpdatePlot = true;
        //qDebug() << "bUpdatePlot =" << bUpdatePlot;
    }
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
        qDebug() << "Open modal window";
}

void MainWindow::get_threshold(int threshold)
{
    value_threshold = threshold;
    qDebug() << "Threshold" << value_threshold;
}

void MainWindow::ChannelCheck(quint32 freq, int ind)
{
    // выявление плат с каналами, в которых частота регистрации сигналов
    // превышает порог

    if (freq > value_threshold)
    {
        numberOfBrokenDevice = ind / ChannelsOnBoard;
        vw->VectorOfbadBoards[ind] = 1;
    }

    //delete vw;
}
