#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "lighttable.h"
#include "thumbnail.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setup();
    setup_connections();
    test();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setup()
{
    ribbon = new PhotoRibbon();
    ui->QGV_timeline->setScene(ribbon);

    light_table = new LightTable();
    ui->QGV_lighttable->setScene(light_table);
    ui->QGV_lighttable->installEventFilter(this);

    holding_table = new LightTable();
    ui->QGV_holdtable->setScene(holding_table);
    ui->QGV_holdtable->installEventFilter(this);
}

void MainWindow::setup_connections()
{
    //QObject::connect(ui->QPB_edit_ok, SIGNAL(clicked()),
    //        this, SLOT(save_photo_edits()));
}


/*
 * Didn't want to subclass QGraphicsView just to reimplement the resize
 * so we use QTs amazing event filter mechanisms and ask our lighttables to
 * rearrange all the photos accordingly whenever the view gets a resize event
 */
bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Resize) {
        light_table->rearrange();
        holding_table->rearrange();
        qDebug() << "Resize";
    }
    // standard event processing
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::test()
{
    //for(int n=0; n < 10; n++)
    //    light_table->add(new Thumbnail());
    QList<unsigned int> ids;
    for(int n=0; n < 10000; n++)
        ids.append(n+1);
    ribbon->set_ids(ids);
}

void MainWindow::save_photo_edits()
{
    ui->stackedWidget->setCurrentIndex(0);
}
