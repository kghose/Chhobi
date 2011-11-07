#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "lighttable.h"
#include "photoribbon.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool eventFilter(QObject *, QEvent *);

private:
    Ui::MainWindow *ui;

    PhotoRibbon *ribbon;

    //we place all the current photos/videos on this
    LightTable *light_table;

    //we place the selected photos on this one for later mailing etc.
    LightTable *holding_table;

    void setup();
    void setup_connections();
    void test();

public slots:
    void save_photo_edits();

};

#endif // MAINWINDOW_H
