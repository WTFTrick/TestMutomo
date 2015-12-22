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
    ui->customPlot->xAxis->setRange(0, 1000);
    ui->customPlot->yAxis->setRange(0, 100);
    ui->customPlot->graph()->setLineStyle((QCPGraph::LineStyle)(2));
    ui->customPlot->replot();

    ip_dialog = new IPDialog();
    CreateConnections();

    QString str = "192.168.10.1";
    connectToHost(str);
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
            //qDebug() << "freq" << arrData.at(i).freq;
        }

        CreatePlot(&arrData);
        //qDebug() << "Size:" << arrData.size();
        m_nNextBlockSize = 0;
    }
}

void MainWindow::slotConnected()
{
    qDebug() << "Received the connected() signal";
}

void MainWindow::on_actionConnect_to_triggered()
{
    //ip_dialog = new IPDialog(this);
    ip_dialog->setModal(true);
    if (ip_dialog->exec() == QDialog::Accepted)
    {
    }
}

void MainWindow::connectToHost(QString str)
{
    strHost = str;
    qDebug() << "Main dialog ip:" << strHost;
    m_pTcpSocket = new QTcpSocket(this);
    m_pTcpSocket->connectToHost(strHost, nPort);
    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
}

void MainWindow::CreateConnections()
{
    qDebug() << "CC";

    connect(ip_dialog, SIGNAL(sendData(QString)), this, SLOT(connectToHost(QString)));
}
