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
#include "database.h"
#include "threadeddiskcrawler.h"

const QString db_location = QDir::homePath() + "/.chhobi/chhobi.sqlite3";
const QStringList name_filters = (QStringList() << "*.jpg" << "*.jpeg" << "*.png" << "*.tiff" << "*.avi");//TODO other

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    PhotoRibbon *ribbon, *hold_ribbon;
    Photo preview;
    QDir photos_root;
    Database db;
    ThreadedDiskCrawler disk_crawler;

    QList<QUrl> resized_photos;
    QDir resized_root;

    void setup_ui();
    void setup_connections();
    void setup_database();
    void setup_photo_root();

    void resizeEvent(QResizeEvent * /* event */);
    bool eventFilter(QObject *, QEvent *);
    void closeEvent(QCloseEvent *) {emit stop_crawl();}//This will tell the db to stop if needed

    void set_datetime(PhotoMetaData pmd);
    void set_metadata_table(PhotoMetaData pmd);
    void set_keywords_table(PhotoMetaData pmd);

    QImage fetch_image(QString, QSize, PhotoMetaData &);

public slots:
    void select_photo_root();
    void crawl();
    void crawl_started() {setWindowTitle("Chhobi: Crawling");}
    void now_crawling(const QString &f) {setWindowTitle(f);}
    void crawl_ended() {setWindowTitle("Chhobi*");}

    void load_photo_list();
    void set_preview_photo(PhotoInfo);
    void send_to_holding();
    void show_preview_external();
    void photo_caption_changed();
    void photo_date_changed();
    void photo_keywords_changed(int row, int col);
    void save_photo_meta_data();
    void resize_photos();
    void show_resized_folder();

signals:
    void stop_crawl();
};

#endif // MAINWINDOW_H
