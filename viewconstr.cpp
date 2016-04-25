#include "viewconstr.h"

bool f_orient = false;

viewConstr::viewConstr(QWidget *parent) : count(720), step(80), countOfBoards(48),
    VectorOfRectanglesOverComboBoxes(48), VectorOfbadBoards(48), nmOfBoardsOnDetector(6),  QWidget(parent)
{
    ClearVectorOfBrokenDevices();
    CreateView();
    CreateConnections();
    ClearJSONFile();
}

viewConstr::~viewConstr()
{
    screen->deleteLater();
    gv->deleteLater();
    ClearVectorOfBrokenDevices();
    close();
}

void viewConstr::CreateView()
{
    //Create graphics view,scene, button and layout

    screen = QGuiApplication::primaryScreen();
    screen->setOrientationUpdateMask(Qt::PortraitOrientation| Qt::LandscapeOrientation| Qt::InvertedPortraitOrientation| Qt::InvertedLandscapeOrientation);

    pb_toJson = new QPushButton("To JSON");
    pb_toJson->setSizePolicy(QSizePolicy::Policy::Fixed,QSizePolicy::Policy::Fixed);

    scene = new QGraphicsScene(this);
    gv = new QGraphicsView();
    gv->setScene(scene);
    gv->setSceneRect(QRectF(125, 200 , 300, 300));
    gv->centerOn( 0 , 0 );
    gv->setAlignment(Qt::AlignCenter);
    gv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gv->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mainLayout = new QVBoxLayout;
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
    int empty_area = count / 2.25;  // empty area between blocks of detectors, where can be radiograph object
    int xcb = width + 15;           // x coordinate for combobox, which containt x and y values
    QSize ComboBoxFixedSize(60, 50);
    int counterForComboBox = 0;
    int nullX = 1;

    // Drawing a rectangles
    for (int i = 0; i < count; i = i + step)
    {
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

                RectanglesForComboBoxes = new QGraphicsRectItem();
                RectanglesForComboBoxes = scene->addRect(j+8, i+5, 63, 53,  transparentPen, TBrush);
                VectorOfRectanglesOverComboBoxes[counterForComboBox] = RectanglesForComboBoxes;

                QComboBox* cmb = new QComboBox();
                listComboBox << cmb;
                cmb->setFixedSize( ComboBoxFixedSize );
                cmb->addItem(  s_numberOfBoardMT );
                gpw = scene->addWidget( cmb );
                cmb->move(j+10,i+6);
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
    connect(pb_toJson, SIGNAL(clicked(bool)), this, SLOT(ToJson()));
    connect(screen, SIGNAL(orientationChanged(Qt::ScreenOrientation)), this, SLOT(onRotate(Qt::ScreenOrientation)));
}

void viewConstr::ClearVectorOfBrokenDevices()
{
    for (int i = 0; i < countOfBoards; i++)
        VectorOfbadBoards[i] = 0;
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

void viewConstr::ToJson()
{
    //Добавить кнопку Get Config
    ClearJSONFile();
    QJsonObject jsonDevice;
    QJsonObject jsonDetector;
    QJsonObject jsonTempInDetector;
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

        QString nameField("Detector ");
        nameField += QString::number(indDetector);

        jsonDetector[nameField] = jsonTempInDetector;
    }
    docJSON.setObject(jsonDetector);

    //QString JsonDoc = docJSON.toJson();
    QByteArray json = docJSON.toJson();

    emit sendJson(json);

    // Writing data in file blocks.json

    /*QFile jsnFile("blocks.json");
    jsnFile.open(QFile::Append);
    QTextStream outJson(&jsnFile);
    outJson << docJSON.toJson();
    jsnFile.close();*/

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

    emit messg( "Change orientation"  );
}




