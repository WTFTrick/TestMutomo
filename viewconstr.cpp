#include "viewconstr.h"

viewConstr::viewConstr(QWidget *parent) : count(720), step(80), VectorOfbadBoards(49), nmOfBoardsOnDetector(6),  QWidget(parent)
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

    scene = new QGraphicsScene(this);
    gv = new QGraphicsView();
    gv->setScene(scene);
    gv->setAlignment(Qt::AlignCenter);
    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(fileNameLabel);
    mainLayout->addWidget(gv);
    mainLayout->addWidget(pb_toJson);
    setLayout(mainLayout);

    QBrush TBrush(Qt::transparent);
    QPen outlinePen(Qt::black);
    QPen transparentPen(Qt::transparent);
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

    //rOverComboBoxes.resize(49);
    //rOverComboBoxes[0] = new QGraphicsRectItem(rectForComboBox);

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
                // Random numbers for comboboxes
                QString s_numberOfBoardMT = QString::number( counterForComboBox );

                QComboBox* cmb = new QComboBox();
                listComboBox << cmb;
                cmb->setFixedSize( ComboBoxFixedSize );
                cmb->addItem(  s_numberOfBoardMT );
                counterForComboBox++;

                gpw = scene->addWidget( cmb );
                cmb->move(j+10,i+6);

                rectForComboBox = scene->addRect(j+10, i+6, 59, 49,  outlinePen, TBrush);
                scene->update();
                //rOverComboBoxes.at(counterForComboBox) = rectForComboBox;
                rOverComboBoxes << new QGraphicsRectItem(rectForComboBox);

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

void viewConstr::BrokenDevice(int index)
{
    QBrush TBrush(Qt::transparent);
    QPen outlinePen(Qt::red);
    outlinePen.setWidth(1);

    QPoint positionOfComboBox;
    positionOfComboBox = listComboBox[index]->pos();
    scene->addRect(positionOfComboBox.rx(), positionOfComboBox.ry(), 59, 49,  outlinePen, TBrush);
}

void viewConstr::ToJson()
{
    // Clearing JSON file before write data in it

    //BrokenDevice(ind);

    ClearJSONFile();

    qDebug() << "ToJson buttonn clicked! Data was converted to JSON!";

    QJsonDocument docJSON;
    QJsonObject jsonDevice;
    QJsonObject jsonDetector;
    QJsonObject jsonTempInDetector;

    //const short nmDevicesOnDetec = 6;
    const short nmDetectors = 8;

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
    mw = new MainWindow();

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_4);

    out << (quint16)0;
    QString output = docJSON.toJson();
    out << output;
    //qDebug() << docJSON.toJson();

    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    mw->m_pTcpSocket->write(block);
}

bool* viewConstr::getBadBoards()
{
    //return &VectorOfbadBoards;
}


