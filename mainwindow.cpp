#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork>
#include <QtGui>

MainWindow::MainWindow() : m_nNextBlockSize(0), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Mutomo Client");
    setCentralWidget(ui->tabWidget);

    graph1 = ui->customPlot->addGraph();
    mapData = graph1->data();

    ui->customPlot->xAxis->setLabel("Номер канала");
    ui->customPlot->yAxis->setLabel("Частота, HZ");
    ui->customPlot->xAxis->setRange(0, 2500);
    ui->customPlot->yAxis->setRange(0, 100);
    ui->customPlot->graph()->setLineStyle((QCPGraph::LineStyle)(2));
    ui->customPlot->replot();

    ip_dialog = new IPDialog( this );

    connectToHost("10.162.1.110");
    CreateConnections();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::CreatePlot(QVector<InfoChannel> *arrData)
{
    //Receiving data from server and plot it
    for (double i = 0; i < arrData->size(); i++)
    {
        mapData->operator [](i) = QCPData(arrData->at(i).nm_channel, arrData->at(i).freq);
    }
    ui->customPlot->replot();
}

void MainWindow::CreatePlot(QVector<double> *arrData)
{
    for (double i = 0; i < arrData->size(); i++)
    {
        mapData->operator [](i) = QCPData(i, arrData->at(i));
    }
    ui->customPlot->replot();
}

void MainWindow::slotReadyRead()
{
    //Reading data from server
    //qDebug() << "slotReadyRead()";

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

        QVector<InfoChannel> arrData;
        InfoChannel tmpInfoChan;
        arrData.clear();

        double  freq = 0;
        QVector<double> arrFreq;
        for (int i = 0; i < m_nNextBlockSize/4; i++ )
        {
            in >> freq;
            arrFreq.append( freq );
            /*
            in >> tmpInfoChan.nm_channel;
            in >> tmpInfoChan.freq;
            arrData.append( tmpInfoChan);
            */

            if (freq > 0 )
                qDebug() << i << ") freq =" << freq;
        }

        CreatePlot( &arrFreq );
        qDebug() << "Size:" << arrFreq.size();

        m_nNextBlockSize = 0;
    }
}

void MainWindow::slotConnected()
{
    qDebug() << "Received the connected() signal";
    qDebug() << "Connection successfull";
    qDebug() << "______________________";
}

void MainWindow::on_actionConnect_to_triggered()
{
    //Connect button action

    ip_dialog->setModal(true);
    if (ip_dialog->exec() == QDialog::Accepted)
    {
    }
}

void MainWindow::connectToHost(QString str)
{
    //Connecting to server
    strHost = str;
    qDebug() << "mainwindow ip:" << strHost;

    m_pTcpSocket = new QTcpSocket(this);
    m_pTcpSocket->connectToHost(strHost, nPort);
    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    qDebug() << "Ready Read Data";
    qDebug() << "_______________";
}

void MainWindow::CreateConnections()
{
    //qDebug() << "CreateConnections";

    //Connections between mainWindow and modal dialog ('connect to...')

    connect(ip_dialog, SIGNAL(sendData(QString)), this, SLOT(connectToHost(QString)));
}
