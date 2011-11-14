#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/*
 * This file is part of Chhobi - a photo organizer program.
 * Copyright (c) 2012 Kaushik Ghose kaushik.ghose@gmail.com
 *
 * Chhobi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Chhobi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Chhobi; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <QMainWindow>
#include "photoribbon.h"
#include "photo.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void resizeEvent(QResizeEvent * /* event */);

private:
    Ui::MainWindow *ui;

    PhotoRibbon *ribbon, *hold_ribbon;
    Photo preview;

    void setup();
    void setup_connections();

    void load_preview_photo(QString absolute_file_name);
    void set_metadata_table(PhotoMetaData pmd);
    void set_keywords_table(PhotoMetaData pmd);
    void test();

public slots:
    void set_preview_photo(unsigned int id);
    void ribbon_selection_changed();
    void show_preview_external();
    void photo_caption_changed();
    void photo_date_changed();
    void photo_keywords_changed(int row, int col);
    void save_photo_meta_data();
};

#endif // MAINWINDOW_H
