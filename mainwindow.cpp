#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork>
#include <QtGui>

MainWindow::MainWindow(const QString& strHost,int nPort) : m_nNextBlockSize(0), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCentralWidget(ui->tabWidget);

    m_pTcpSocket = new QTcpSocket(this);
    m_pTcpSocket->connectToHost(strHost, nPort);
    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));

    graph1 = ui->customPlot->addGraph();
    ui->customPlot->xAxis->setLabel("x");
    ui->customPlot->yAxis->setLabel("y");
    ui->customPlot->xAxis->setRange(0, 100);
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
    QVector<double> arrX(101), arrY(101);

    for (double i = 0; i < arrData->size(); i++)
    {
        arrX[i] = arrData->at(i).nm_channel;
        arrY[i] = arrData->at(i).freq;
    }

    graph1->setData(arrX, arrY);

    /*for (double i = 0; i < arrData->size(); i++)
    {
        qDebug() << "CreatePlot nm_channel:" << arrData->at(i).nm_channel;
        qDebug() << "CreatePlot freq:" << arrData->at(i).freq;
        mapData->operator [](i) = QCPData(arrData->at(i).nm_channel, arrData->at(i).freq);
    }
    mapData = ui->customPlot->graph(0)->data();*/

    ui->customPlot->replot();

    arrX.clear();
    arrY.clear();
}

void MainWindow::slotReadyRead()
{
    qDebug() << "Slot reading ...";
    QDataStream in(m_pTcpSocket);
    in.setVersion(QDataStream::Qt_4_5);
    for (;;) {

        if (!m_nNextBlockSize)
        {
            if (m_pTcpSocket->bytesAvailable() < sizeof(quint32)) {
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
        for (int i = 0; i < m_nNextBlockSize/4; i++ )
        {
            in >> tmpInfoChan.nm_channel;
            in >> tmpInfoChan.freq;
            //qDebug() << tmpInfoChan.freq;
            arrData.append( tmpInfoChan);
            CreatePlot(&arrData);
        }
        arrData.clear();
        m_nNextBlockSize = 0;
    }
}

void MainWindow::slotConnected()
{
    qDebug() << "Received the connected() signal";
}
