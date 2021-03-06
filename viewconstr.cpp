#include "viewconstr.h"

//bool f_orient = false;

viewConstr::viewConstr(QWidget *parent) :
    count(720),
    step(80),
    countOfBoards(48),
    VectorOfRectanglesOverComboBoxes(48),
    VectorOfbadBoards(48),
    nmOfBoardsOnDetector(6),
    QWidget(parent)
{
    ClearVectorOfBrokenDevices();
    CreateView();
    CreateConnections();
    ClearJSONFile();
}

viewConstr::~viewConstr()
{
    for (int i = 0; i < ListCoordComboBox.size(); ++i){
        delete ListCoordComboBox.at(i);
    }
    for (int i = 0; i < listComboBox.size(); ++i)
        delete listComboBox.at(i);

    delete scene;
    delete mainLayout;
}

void viewConstr::CreateView()
{
    //Create graphics view,scene, button and layout
    screen = QGuiApplication::primaryScreen();
    screen->setOrientationUpdateMask(Qt::PortraitOrientation| Qt::LandscapeOrientation| Qt::InvertedPortraitOrientation| Qt::InvertedLandscapeOrientation);


    scene = new QGraphicsScene(  );
    gv = new QGraphicsView( this );
    gv->setScene(scene);
    gv->setSceneRect(QRectF(125, 200 , 300, 300));
    gv->centerOn( 0 , 0 );
    gv->setAlignment(Qt::AlignCenter);
    gv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gv->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    pbSaveConfig = new QPushButton("Сохранить конфигурацию");
    pbSaveConfig->setMinimumHeight( 27 );
    pbSaveConfig->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);

    mainLayout = new QVBoxLayout( );
    mainLayout->addWidget(gv);
    mainLayout->addWidget( pbSaveConfig );
    mainLayout->setAlignment(pbSaveConfig, Qt::AlignHCenter);
    setLayout(mainLayout);

    QBrush TBrush(Qt::transparent);
    QPen outlinePen(Qt::black);
    QPen transparentPen(Qt::transparent);
    outlinePen.setWidth(0.5);
    // Add variables for drawing comboboxes, spinboxes and rectangles

    const quint8 height = 60;
    int width = step * nmOfBoardsOnDetector;
    int empty_area = count / 2.25;  // empty area between blocks of detectors, where can be radiograph object
    int xcb = width + 15;           // x coordinate for combobox, which containt x and y values
    QSize ComboBoxFixedSize(60, 50);
    int counterForComboBox = 0;
    int nullX = 1;

    // Drawing a rectangles
    for (int i = 0; i < count; i = i + step){
        if (i != empty_area)
            scene->addRect(nullX, i, width, height, outlinePen,TBrush);
    }

    // Drawing a comboboxes, which contain number of MT48
    for (quint16 i = 0; i < count; i = i + step)
    {
        for (quint16 j = 0; j < count; j = j + step)
        {
            if ((j < width) && (i != empty_area))
            {
                QString s_numberOfBoardMT = QString::number( counterForComboBox );
                VectorOfRectanglesOverComboBoxes[counterForComboBox] = scene->addRect(j + 8, i + 5, 63, 53,  transparentPen, TBrush);

                QComboBox* cmb = new QComboBox( );
                cmb->setFixedSize( ComboBoxFixedSize );
                cmb->addItem(  s_numberOfBoardMT );
                cmb->move(j+10,i+6);

                listComboBox << cmb;
                gpw = scene->addWidget( cmb );
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
            QComboBox* cmb_coord = new QComboBox( );
            ListCoordComboBox << cmb_coord;
            cmb_coord->setFixedSize( ComboBoxFixedSize );
            cmb_coord->addItem( "X" );
            cmb_coord->addItem( "Y" );
            gpw = scene->addWidget( cmb_coord );
            cmb_coord->move(xcb,k+6);
        }
    }

}

void viewConstr::resizeEvent(QResizeEvent *event)
{
    /*
    if ( f_orient ){
        scene->addRect(0, 0, 50, 50,  QPen(Qt::transparent), QBrush(Qt::red));
        f_orient = false;
    }
    else{
        scene->addRect(0, 0, 50, 50,  QPen(Qt::transparent), QBrush(Qt::blue));
        f_orient = true;
    }

    gv->repaint();
    */

    gv->fitInView(-40, -40, scene->width() + 40, scene->height() + 40, Qt::KeepAspectRatio);

    QWidget::resizeEvent(event);
}

bool viewConstr::event(QEvent *event)
{
    if( event->type() == QEvent::Show)
        gv->fitInView(-40, -40,  scene->width() + 40, scene->height() + 40,  Qt::KeepAspectRatio);

    return QWidget::event(event);
}

void viewConstr::CreateConnections()
{
    connect(pbSaveConfig, SIGNAL(clicked(bool)), this, SLOT(createJSONConfiguration()));
    connect(screen, SIGNAL(orientationChanged(Qt::ScreenOrientation)), this, SLOT(onRotate(Qt::ScreenOrientation)));
}

void viewConstr::ClearVectorOfBrokenDevices()
{
    for (int i = 0; i < countOfBoards; i++)
        VectorOfbadBoards[i] = 0;
}

void viewConstr::ClearJSONFile()
{
    QFile file("config.json");
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
    redPen.setWidth(2);

    for ( int i = 0; i < countOfBoards; i++ )
        if ( VectorOfbadBoards[i] == 1)
        {
            VectorOfRectanglesOverComboBoxes[i]->setPen(redPen);
            VectorOfRectanglesOverComboBoxes[i]->setBrush(QBrush(Qt::red));
        }
        else
        {
            VectorOfRectanglesOverComboBoxes[i]->setPen(transparentPen);
            VectorOfRectanglesOverComboBoxes[i]->setBrush(QBrush(Qt::transparent));
        }

    ClearVectorOfBrokenDevices();

}

void viewConstr::createJSONConfiguration()
{
    //==================================================================

    //Old version

    ClearJSONFile();
    const short nmDetectors = 8;
    /*QJsonObject jsonDevice;
    QJsonObject jsonDetector;
    QJsonObject jsonTempInDetector;

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

        QString nameField("Detector ");
        nameField += QString::number(indDetector);

        jsonDetector[nameField] = jsonTempInDetector;
    }

    docJSON.setObject(jsonDetector);

    //QString JsonDoc = docJSON.toJson();
    //QByteArray json = docJSON.toJson();

    //emit sendJson(json);*/



    //====================================================================

    //New version

    QJsonObject jsonMainObj;
    QJsonArray  jsonTypeConfig;

    for (quint8 indChamber = 0; indChamber < nmDetectors; indChamber++ )
    {
        QJsonObject jsonChamberObj;
        jsonChamberObj["chamber"] = indChamber;
        jsonChamberObj["coordinate"] = ListCoordComboBox[indChamber]->currentText();

        //jsonChamberObj["num_layers"]   = ui->sbNumberLayers->value();
        //jsonChamberObj["num_tubes"]    = ui->sbNumberTubesInLayer->value();
        //jsonChamberObj["type_device"]  = ui->cbTypeDevice->currentText();

        // Record device numbers
        QJsonArray jsonDevices;

        for (quint8 indDevice = 0; indDevice < nmOfBoardsOnDetector; ++indDevice){
            jsonDevices.push_back( listComboBox[(indChamber*nmOfBoardsOnDetector) + indDevice]->currentText() );
        }
        jsonChamberObj["devices"] = jsonDevices;
        jsonTypeConfig.append( jsonChamberObj );
    }
    jsonMainObj["MuTomo"] = jsonTypeConfig;

    // Record main json-object to file
    QJsonDocument json_cfg( jsonMainObj );

    // Writing data in file config.json
    QFile jsnFile("config.json");
    jsnFile.open(QFile::Append);
    QTextStream outJson(&jsnFile);
    outJson << json_cfg.toJson();
    jsnFile.close();

    // Send QByteArray to server
    QByteArray json_config = json_cfg.toJson();
    emit sendJson(json_config);
}

void viewConstr::onRotate(Qt::ScreenOrientation)
{
    scene->addRect(0, 0, 1, 1,  QPen(Qt::transparent), QBrush(Qt::transparent));
    gv->setSceneRect(QRectF(125, 200 , 300, 300));

    // don't work without changes in scene
    // ------------------

    /*
    if ( f_orient ){
        scene->addRect(0, 0, 50, 50,  QPen(Qt::transparent), QBrush(Qt::red));
        f_orient = false;
    }
    else{
        scene->addRect(0, 0, 50, 50,  QPen(Qt::transparent), QBrush(Qt::blue));
        f_orient = true;
    }
    */

    // ------------------

    //emit messg( "Change orientation"  );
}




