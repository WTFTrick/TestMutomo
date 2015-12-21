#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork>
#include <QtGui>

MainWindow::MainWindow(const QString& strHost,int nPort) : m_nNextBlockSize(0), arraySize(1000), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Mutomo Client");
    setCentralWidget(ui->tabWidget);

    m_pTcpSocket = new QTcpSocket(this);
    m_pTcpSocket->connectToHost(strHost, nPort);
    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));

    QVector<double> arrX(arraySize);
    QVector<double> arrY(arraySize);

    graph1 = ui->customPlot->addGraph();
    graph1->setData(arrX, arrY);

    mapData = graph1->data();
    ui->customPlot->xAxis->setLabel("Номер канала");
    ui->customPlot->yAxis->setLabel("Частота, HZ");
    ui->customPlot->xAxis->setRange(0, 1000);
    ui->customPlot->yAxis->setRange(0, 100);
    ui->customPlot->graph()->setLineStyle((QCPGraph::LineStyle)(2));
    ui->customPlot->replot();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::CreatePlot(QVector<InfoChannel> *arrData)
{
    for (double i = 0; i < arrData->size(); i++)
    {
        mapData->operator [](i) = QCPData(arrData->at(i).nm_channel, arrData->at(i).freq);
    }
    ui->customPlot->replot();
}

void MainWindow::slotReadyRead()
{
    QDataStream in(m_pTcpSocket);
    in.setVersion(QDataStream::Qt_4_5);
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
        for (int i = 0; i < m_nNextBlockSize/4; i++ )
        {
            in >> tmpInfoChan.nm_channel;
            in >> tmpInfoChan.freq;
            arrData.append( tmpInfoChan);
        }
        CreatePlot(&arrData);
        m_nNextBlockSize = 0;
    }
}

void MainWindow::slotConnected()
{
    //qDebug() << "Received the connected() signal";
}
