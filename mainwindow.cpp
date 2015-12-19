#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork>
#include <QtGui>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

MainWindow::MainWindow(const QString& strHost,int nPort, QWidget* pwgt /*=0*/) : m_nNextBlockSize(0), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCentralWidget(ui->tabWidget);

    m_pTcpSocket = new QTcpSocket(this);

    m_pTcpSocket->connectToHost(strHost, nPort);
    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this,         SLOT(slotError(QAbstractSocket::SocketError))
           );

    ui->customPlot->addGraph();

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
    QVector<double> arrX(101);
    QVector<double> arrY(101);

    for (int i = 0; i < arrData->size(); i++)
    {
        arrX[i] = arrData->at(i).nm_channel;
        arrY[i] = arrData->at(i).freq;
    }

     ui->customPlot->graph(0)->setData(arrX, arrY);

    /*
    for (int i = 0; i < arrData->size(); i++)
    {
        mapData->operator [](i) = QCPData(arrData->at(i).nm_channel, arrData->at(i).freq);
        qDebug() << "CreatePlot freq:" << arrData->at(i).freq;
    }
    mapData = ui->customPlot->graph(0)->data();
    */

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
            qDebug() << tmpInfoChan.freq;
            arrData.append( tmpInfoChan);
            CreatePlot(&arrData);
        }
        arrData.clear();
        m_nNextBlockSize = 0;
    }
}

// ----------------------------------------------------------------------
void MainWindow::slotError(QAbstractSocket::SocketError err)
{
    QString strError =
        "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                     "The host was not found." :
                     err == QAbstractSocket::RemoteHostClosedError ?
                     "The remote host is closed." :
                     err == QAbstractSocket::ConnectionRefusedError ?
                     "The connection was refused." :
                     QString(m_pTcpSocket->errorString())
                    );
}

// ----------------------------------------------------------------------
void MainWindow::slotSendToServer()
{
}

// ------------------------------------------------------------------
void MainWindow::slotConnected()
{
    qDebug() << "Received the connected() signal";
    slotReadyRead();
}
