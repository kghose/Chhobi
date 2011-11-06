#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "lighttable.h"

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

    //we place all the current photos/videos on this
    LightTable *light_table;

    //we place the selected photos on this one for later mailing etc.
    LightTable *holding_table;

    void setup();
    void resize(int w, int h);
    void test();
};

#endif // MAINWINDOW_H
