#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork>
#include <QtGui>

MainWindow::MainWindow() : m_nNextBlockSize(0),LinesCount(48), ui(new Ui::MainWindow)
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

    /* Ticks and Grid features
    ui->customPlot->xAxis->setAutoTickStep(false);
    ui->customPlot->xAxis->setTickStep(200);
    ui->customPlot->yAxis->setAutoTickStep(false);
    ui->customPlot->yAxis->setTickStep(10);
    ui->customPlot->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1.25, Qt::DotLine));
    ui->customPlot->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1.25, Qt::DotLine));
    */

    // Add QCPItemLine and QCPItemText
    int NumberOfBoardi = 0;
    for (int i = 1; i < LinesCount*LinesCount ; i = i + LinesCount)
    {
    QCPItemLine *tickHLine=new QCPItemLine(ui->customPlot);
    ui->customPlot->addItem(tickHLine);
    tickHLine->start->setCoords(i,0);
    tickHLine->end->setCoords(i,30);
    tickHLine->setPen(QPen(QColor(0, 255, 0), 3));
    }

    NumberOfBoard = new QCPItemText(ui->customPlot);

    // Colour and width of graph
    QPen graphPen;
    graphPen.setColor(QColor(0, 0, 0));
    graphPen.setWidthF(0.5);
    ui->customPlot->graph()->setPen(graphPen);
    ui->customPlot->replot();

    ip_dialog = new IPDialog( this );

    connectToHost("0.0.0.0");
    //Mutomo host = "10.162.1.110"
    CreateConnections();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::CreatePlot(QVector<InfoChannel> *arrData)
{
    // Receiving data from server and plot it
    for (double i = 0; i < arrData->size(); i++)
    {
        mapData->operator [](i) = QCPData(arrData->at(i).nm_channel, arrData->at(i).freq);
    }
    ui->customPlot->replot();
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

    // Dynamic range of x and y axis;

    ui->customPlot->xAxis->setRange(0, arrData->size() + 100);
    qDebug() << "arrData->size():" << arrData->size();
    ui->customPlot->yAxis->setRange(0, maxY + 20);

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

        //------------------------------
        QVector<InfoChannel> arrData;
        InfoChannel tmpInfoChan;
        arrData.clear();
        //------------------------------

        quint32  freq = 0;
        QVector<quint32> arrFreq;
        for (int i = 0; i < m_nNextBlockSize/4; i++ )
        {
            in >> freq;
            arrFreq.append( freq );

            //qDebug() << "arrFreq.size() =" << arrFreq.size();

            /*
            in >> tmpInfoChan.nm_channel;
            in >> tmpInfoChan.freq;
            arrData.append( tmpInfoChan);
            */

            //if (freq > 0 )
                //qDebug() << "i=" << i << ", freq =" << freq;

        }
        CreatePlot( &arrFreq );
        //qDebug() << "size:" << arrFreq.size();

        arrFreq.clear();

        m_nNextBlockSize = 0;
    }
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
}

void MainWindow::on_pb_ZoomIn_clicked()
{
    ui->customPlot->xAxis->scaleRange(0.85, ui->customPlot->xAxis->range().center());
    ui->customPlot->yAxis->scaleRange(0.85, ui->customPlot->yAxis->range().center());
    ui->customPlot->replot();
}

void MainWindow::on_pb_ZoomOut_clicked()
{
    ui->customPlot->xAxis->scaleRange(1.17, ui->customPlot->xAxis->range().center());
    ui->customPlot->yAxis->scaleRange(1.17, ui->customPlot->yAxis->range().center());
    ui->customPlot->replot();
}

void MainWindow::on_pb_ResetRange_clicked()
{
    ui->customPlot->xAxis->setRange(0, 2500);
    ui->customPlot->yAxis->setRange(0, 100);
    ui->customPlot->replot();
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
    double upperBound = 150; // note: code assumes lowerBound < upperBound
    QCPRange fixedRange(newRange);
    if (fixedRange.lower < lowerBound)
    {
      fixedRange.lower = lowerBound;
      fixedRange.upper = lowerBound + newRange.size();
      if (fixedRange.upper > upperBound || qFuzzyCompare(newRange.size(), upperBound-lowerBound))
        fixedRange.upper = upperBound;
      ui->customPlot->yAxis->setRange(fixedRange); // adapt this line to use your plot/axis
    } else if (fixedRange.upper > upperBound)
    {
      fixedRange.upper = upperBound;
      fixedRange.lower = upperBound - newRange.size();
      if (fixedRange.lower < lowerBound || qFuzzyCompare(newRange.size(), upperBound-lowerBound))
        fixedRange.lower = lowerBound;
      ui->customPlot->yAxis->setRange(fixedRange); // adapt this line to use your plot/axis
    }
}

void MainWindow::AxisChanged(QCPRange newRange)
{
    double upperBound = 1200; // note: code assumes lowerBound < upperBound
    QCPRange fixedRange(newRange);
    //qDebug() << fixedRange.size();
    if (fixedRange.size() > upperBound)
    {
      for (int i = 1; i < LinesCount*LinesCount ; i = i + LinesCount)
      {
      NumberOfBoard->position->setCoords(i + 24, 7);
      QString null = "    ";
      NumberOfBoard->setText(null);
      NumberOfBoard->setFont(QFont(font().family(), 0));
      ui->customPlot->replot();
      }
    }
    else
    {
        int NumberOfBoardi = 0;
        for (int i = 1; i < LinesCount*LinesCount ; i = i + LinesCount)
        {
        NumberOfBoardi++;
        QString NOB = QString("%1").arg(NumberOfBoardi);
        //qDebug() << NumberOfBoardi;
        NumberOfBoard->position->setCoords(i + 24, 7);
        NumberOfBoard->setText(NOB);
        NumberOfBoard->setFont(QFont(font().family(), 10));
        NumberOfBoard->setPadding(QMargins(8, 0, 0, 0));
        ui->customPlot->replot();
        }
    }
}

void MainWindow::CreateConnections()
{
    //qDebug() << "CreateConnections";

    // Connections between mainWindow and modal dialog ('connect to...')
    connect(ip_dialog, SIGNAL(sendData(QString)), this, SLOT(connectToHost(QString)));
    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(ui->customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(AxisChanged(QCPRange)));
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(yAxisChanged(QCPRange)));
}
