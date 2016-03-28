#include "viewconstr.h"

viewConstr::viewConstr(QWidget *parent) : count(720), step(80), countOfBoards(48),
    VectorOfRectanglesOverComboBoxes(48), VectorOfbadBoards(48), nmOfBoardsOnDetector(6),  QWidget(parent)
{
    ClearJSONFile();
    CreateView();
    CreateConnections();
}

void viewConstr::CreateView()
{
    // Add label, button, scene and layout
    QLabel *fileNameLabel = new QLabel(tr("Configuration:"));
    pb_toJson = new QPushButton("To JSON");
    fileNameLabel->setAlignment(Qt::AlignCenter); //Set label at center

    for (int i = 0; i < countOfBoards; i++)
        VectorOfbadBoards[i] = 0;

    scene = new QGraphicsScene(this);
    gv = new QGraphicsView();
    gv->setScene(scene);
    gv->setAlignment(Qt::AlignCenter);
    gv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gv->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(fileNameLabel);
    mainLayout->addWidget(gv);
    mainLayout->addWidget(pb_toJson);
    setLayout(mainLayout);

    QBrush TBrush(Qt::transparent);
    QPen outlinePen(Qt::black);
    QPen transparentPen(Qt::transparent);
    QPen redPen(Qt::red);
    outlinePen.setWidth(0.5);

    // Add variables for drawing comboboxes, spinboxes and rectangles

    int width = step * nmOfBoardsOnDetector;
    quint8 height = 60;
    int empty_area = count / 2.25; // empty area between blocks of detectors, where can be radiograph object
    int xcb = width + 15; // x coordinate for combobox, which containt x and y values
    int xsb = width + 100; // x coordinate for spinbox, which contain order nubmer
    int NumberInSpinBox = 0; // vaule, which will be in spinBox
    QSize ComboBoxFixedSize(60, 50);
    QSize SpinBoxFixedSize(85, 50);
    int counterForComboBox = 0;

    // Drawing a transparent rectagles, for better viewing
    rectangle = scene->addRect(0, -20, width, height / 6, transparentPen, TBrush);
    rectangle = scene->addRect(0, 720, width, height / 6, transparentPen, TBrush);

    // Drawing a rectangles
    for (quint16 i = 0; i < count; i = i + step)
    {
        if (i != empty_area)
        {
            rectangle = scene->addRect(0, i, width, height, outlinePen,TBrush);
        }
    }

    // Drawing a comboboxes, which contain number of MT48

    for (quint16 i = 0; i < count; i = i + step)
    {

        for (quint16 j = 0; j < count; j = j + step)
        {

            if ((j < width) && (i != empty_area))
            {
                QString s_numberOfBoardMT = QString::number( counterForComboBox );

                QComboBox* cmb = new QComboBox();
                listComboBox << cmb;
                cmb->setFixedSize( ComboBoxFixedSize );
                cmb->addItem(  s_numberOfBoardMT );

                gpw = scene->addWidget( cmb );
                cmb->move(j+10,i+6);

                RectanglesForComboBoxes = new QGraphicsRectItem();
                RectanglesForComboBoxes = scene->addRect(j+10, i+6, 59, 49,  transparentPen, TBrush);

                VectorOfRectanglesOverComboBoxes[counterForComboBox] = RectanglesForComboBoxes;
                scene->update();

                counterForComboBox++;
            }
        }
    }

    //Drawing a comboboxes (which contain x and y) and spinboxes

    for (int k = 0; k < count; k += step)
    {
        if (k != empty_area)
        {
            QComboBox* cmb_coord = new QComboBox();
            ListCoordComboBox << cmb_coord;
            cmb_coord->setFixedSize( ComboBoxFixedSize );
            cmb_coord->addItem( "X" );
            cmb_coord->addItem( "Y" );
            gpw = scene->addWidget( cmb_coord );
            cmb_coord->move(xcb,k+6);

            QSpinBox* spinbx_numb = new QSpinBox();
            ListSpinBox << spinbx_numb;
            spinbx_numb->setRange(0,50);
            spinbx_numb->setFixedSize(SpinBoxFixedSize);
            gpw = scene->addWidget(spinbx_numb);
            spinbx_numb->move(xsb,k+6);
            spinbx_numb->setValue(NumberInSpinBox);
            NumberInSpinBox++;
        }
    }
}

void viewConstr::resizeEvent(QResizeEvent *event)
{
    gv->fitInView(-100, -100, scene->width()+100, scene->height()+100, Qt::KeepAspectRatio);

    QWidget::resizeEvent(event);
}

bool viewConstr::event(QEvent *event)
{
    if( event->type() == QEvent::Show)
    {
        gv->fitInView(-100, -100, scene->width()+100, scene->height()+100, Qt::KeepAspectRatio);
    }
    return QWidget::event(event);
}

void viewConstr::CreateConnections()
{
    connect(pb_toJson, SIGNAL(clicked(bool)), this, SLOT(ToJson()));
}

void viewConstr::ClearJSONFile()
{
    QFile file("blocks.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        file.write("");
    }
}

void viewConstr::BrokenDevice()
{
    // Cycle for broken boards

    QPen redPen(Qt::red);
    QPen transparentPen(Qt::transparent);

    for ( int i = 0; i < countOfBoards; i++ )
        if ( VectorOfbadBoards[i] == 1)
        {
            qDebug() << "Device with number" << i << "broken:" << VectorOfbadBoards[i];
            VectorOfRectanglesOverComboBoxes[i]->setPen(redPen);
        }
        else
        {
            qDebug() << "Device with number" << i << "broken:" << VectorOfbadBoards[i];
            VectorOfRectanglesOverComboBoxes[i]->setPen(transparentPen);
        }
}

void viewConstr::ToJson()
{
    ClearJSONFile();

    BrokenDevice();

    QJsonDocument docJSON;
    QJsonObject jsonDevice;
    QJsonObject jsonDetector;
    QJsonObject jsonTempInDetector;
    const short nmDetectors = 8;
    //const short nmDevicesOnDetec = 6;

    // Creating a structure of JSON file

    for (int indDetector = 0; indDetector < nmDetectors; ++indDetector)
    {
        for(int indDevice = 0; indDevice < nmOfBoardsOnDetector; ++indDevice )
        {
            QString nameField("Number of MT48: ");
            nameField += QString::number(indDevice);
            jsonDevice[nameField] = listComboBox[(indDetector*nmOfBoardsOnDetector) + indDevice]->currentText();
        }
        jsonTempInDetector["Device "] = jsonDevice;

        jsonTempInDetector["Coordinate"] = ListCoordComboBox[indDetector]->currentText();

        jsonTempInDetector["Number "] = ListSpinBox[indDetector]->value();

        QString nameField("Detector ");
        nameField += QString::number(indDetector);

        jsonDetector[nameField] = jsonTempInDetector;
    }
    docJSON.setObject(jsonDetector);

    // Writing data in file blocks.json

    /*QFile jsnFile("blocks.json");
    jsnFile.open(QFile::Append);
    QTextStream outJson(&jsnFile);
    outJson << docJSON.toJson();
    jsnFile.close();*/

    // Send JSON to server

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_4);

    out << (quint16)0;
    QString output = docJSON.toJson();
    out << output;

    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    //mw->m_pTcpSocket->write(block);



    /*QPen redPen(Qt::red);
    foreach( QGraphicsRectItem *item, VectorOfRectanglesOverComboBoxes )
    VectorOfRectanglesOverComboBoxes.takeAt( VectorOfRectanglesOverComboBoxes.indexOf( item ) )->setPen(redPen);
    scene->update();*/
}

bool* viewConstr::getBadBoards()
{
    return VectorOfbadBoards.data();
}


