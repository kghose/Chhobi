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

#include <QDesktopServices>
#include <QUrl>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "exivmanager.h"
#include "imagerotate.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //for our settings
    QCoreApplication::setOrganizationName("BengalBionics");
    QCoreApplication::setOrganizationDomain("bengalbionics.com");
    QCoreApplication::setApplicationName("Chhobi");

    setup_ui();
    restore_appearance();
    setup_connections();
    setup_database();
    setup_photo_root();
    load_photo_list();
    crawl();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setup_ui()
{
    ui->setupUi(this);

    ribbon = new PhotoRibbon();
    ui->QGV_timeline->setScene(ribbon);

    hold_ribbon = new PhotoRibbon(0, true);//true for holding table
    ui->QGV_hold->setScene(hold_ribbon);
}

void MainWindow::restore_appearance()
{
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    emit stop_crawl();//This will tell the db to stop if needed
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());

    while(disk_crawler.isRunning()) {;}//wait until crawler is done
    QMainWindow::closeEvent(event);
}

void MainWindow::setup_connections()
{
    //Event filter for drag on mailing label
    ui->mailLabel->installEventFilter(this);

    //Menu
    QObject::connect(ui->actionSet_Root, SIGNAL(triggered()),
            this, SLOT(select_photo_root()));
    QObject::connect(ui->actionCrawl, SIGNAL(triggered()),
                     this, SLOT(crawl()));
    QObject::connect(ui->actionReload, SIGNAL(triggered()),
                     this, SLOT(load_photo_list()));

    //Ribbon
    QObject::connect(ribbon, SIGNAL(preview_id(PhotoInfo)),
            this, SLOT(set_preview_photo(PhotoInfo)));
    QObject::connect(ribbon, SIGNAL(hold()),
            this, SLOT(send_to_holding()));
    QObject::connect(hold_ribbon, SIGNAL(preview_id(PhotoInfo)),
            this, SLOT(set_preview_photo(PhotoInfo)));

    //Editing controls
    QObject::connect(ui->captionEdit, SIGNAL(textEdited(QString)),
            this, SLOT(photo_caption_changed()));
    QObject::connect(ui->captionEdit, SIGNAL(returnPressed()),
            this, SLOT(save_photo_meta_data()));
    QObject::connect(ui->dateTimeEdit, SIGNAL(editingFinished()),
            this, SLOT(save_photo_meta_data()));

    //Holding table connections
    QObject::connect(ui->resizeButton, SIGNAL(clicked()),
            this, SLOT(resize_photos()));
    QObject::connect(ui->folderButton, SIGNAL(clicked()),
            this, SLOT(show_resized_folder()));


    //Database crawler
    QObject::connect(&disk_crawler, SIGNAL(started()),
                     this, SLOT(crawl_started()));
    QObject::connect(&disk_crawler, SIGNAL(finished()),
                     this, SLOT(crawl_ended()));
    QObject::connect(&disk_crawler, SIGNAL(now_searching(const QString &)),
                     this, SLOT(now_crawling(const QString &)));
    QObject::connect(this, SIGNAL(stop_crawl()),
                     &disk_crawler, SLOT(stop()));//needed for window close

}

void MainWindow::setup_database()
{
    //setup database
    QSettings settings;
    if(!settings.contains("database file name"))
        settings.setValue("database file name", db_location);
    QFileInfo dbpath(settings.value("database file name").toString());
    open_database(dbpath);
}

void MainWindow::setup_photo_root()
{
    QSettings settings;
    if(!settings.contains("photo root"))
        select_photo_root();//if successful this call setup_photo_root again
    else {
        photos_root = QDir(settings.value("photo root").toString());
        photos_root.setNameFilters(name_filters);
        photos_root.setSorting(QDir::Time | QDir::DirsFirst);
    }
}

QImage MainWindow::fetch_image(QString absolute_file_name, QSize max_size, PhotoMetaData &pmd)
{
    pmd = load_metadata(absolute_file_name);//The preview image needs this returned
    QImageReader qir(absolute_file_name);
    QSize orig = qir.size();
    float ratio, rw, rh , width, height;
    if(pmd.rotation_angle==0 || pmd.rotation_angle==180) {
        width = orig.width();
        height = orig.height();
    } else {
        height = orig.width();
        width = orig.height();
    }
    rw = (float)max_size.width()/width;
    rh = (float)max_size.height()/height;
    if( rw > rh)
        ratio = rh;
    else
        ratio = rw;
    qir.setScaledSize(ratio*orig);
    return rotate(pmd.rotation_angle, qir.read());
}

void MainWindow::set_preview_photo(PhotoInfo pi)
{
    PhotoMetaData pmd;
    QString absolute_file_name = photos_root.absoluteFilePath(pi.relative_file_path);
    this->statusBar()->showMessage(absolute_file_name);
    QImage pmI;
    if(photos_root.exists(pi.relative_file_path)) {
        pmI = fetch_image(absolute_file_name, ui->QL_preview->size(), pmd);
    } else {//Photo reference in db but not on disk
        pmd.valid = false;
        pmI.load(":/Images/Icons/chhobi-icon.png");
        qDebug() << pi.relative_file_path << ": If this zombie is not purged next load, we have a coding error";
    }

    preview.set_meta_data(pmd);
    if(pmd.type == PHOTO)
        preview.set_pixmap(QPixmap::fromImage(pmI));
    else
        preview.set_pixmap(QPixmap(":/Images/Icons/cholochitro.png"));

    ui->QL_preview->setPixmap(preview.get_photo());
    ui->captionEdit->setText(pmd.caption);
    set_datetime(pmd);
    set_metadata_table(pmd);
    set_keywords_table(pmd);
}

//really just needed this, since the date changed signal goes out even when we
//first load it
void MainWindow::set_datetime(PhotoMetaData pmd)
{
    QObject::disconnect(ui->dateTimeEdit, SIGNAL(dateTimeChanged (const QDateTime &)),
            this, SLOT(photo_date_changed()));
    if(pmd.photo_date.isValid())
        ui->dateTimeEdit->setDateTime(pmd.photo_date);
    else
        ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
    QObject::connect(ui->dateTimeEdit, SIGNAL(dateTimeChanged (const QDateTime &)),
            this, SLOT(photo_date_changed()));
}

void MainWindow::set_metadata_table(PhotoMetaData pmd)
{
    QString metadata_string;
    if(!pmd.valid)
        metadata_string = "\n\nFILE NOT FOUND";
    else if (pmd.type==MOVIE)
        metadata_string = "\nMovie file";
    else
        metadata_string =
           pmd.exposure_time.pretty_print() + "s\n" +
                "f" + pmd.fnumber.pretty_print() + "\n" +
                QString::number(pmd.iso) + " iso\n" +
                pmd.focal_length.pretty_print() + "mm\n" +
                pmd.camera_model + "\n" +
                pmd.lens_model;
    ui->QPTE_metadata->setPlainText(metadata_string);
}

void MainWindow::set_keywords_table(PhotoMetaData pmd)
{
    QObject::disconnect(ui->QTW_keywords, SIGNAL(cellChanged (int, int)),
            this, SLOT(photo_keywords_changed(int, int)));
    ui->QTW_keywords->clear();
    int cnt = pmd.keywords.count();
    ui->QTW_keywords->setRowCount(cnt+1);
    for(int i=0;i<cnt;i++)
        ui->QTW_keywords->setItem(i,0,new QTableWidgetItem(pmd.keywords[i]));
    QObject::connect(ui->QTW_keywords, SIGNAL(cellChanged (int, int)),
            this, SLOT(photo_keywords_changed(int, int)));
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QSize scaledSize = preview.get_photo().size();
    scaledSize.scale(ui->QL_preview->size(), Qt::KeepAspectRatio);
    if (ui->QL_preview->pixmap() && scaledSize != ui->QL_preview->pixmap()->size())
        ui->QL_preview->setPixmap(preview.get_photo().scaled(ui->QL_preview->size(),
                                                              Qt::KeepAspectRatio));
    QMainWindow::resizeEvent(event);
}

//Call this when we select items in the ribbon, which means we want to put them
//in the holding ribbon
void MainWindow::send_to_holding()
{
    hold_ribbon->append_tiles(ribbon->get_selected_tiles());
}

void MainWindow::photo_caption_changed()
{
    ui->captionEdit->setStyleSheet("background: lightgreen");
}

void MainWindow::photo_date_changed()
{
    ui->dateTimeEdit->setStyleSheet("background: lightgreen");
}

void MainWindow::photo_keywords_changed(int row, int col)
{
    bool empty_row = false, last_row = false;
    if(ui->QTW_keywords->item(row,col)->text()=="") empty_row = true;
    if(row==preview.get_metadata().keywords.count()) last_row = true;
    if(last_row) {
        if(!empty_row) {
            ui->QTW_keywords->setRowCount(row+2);
            ui->QTW_keywords->setCurrentCell(row+1,0);
        }
    } else if(empty_row) {
        ui->QTW_keywords->removeRow(row);
    }
    PhotoMetaData pmd = preview.get_metadata();
    pmd.keywords.clear();
    for(int i=0; i < ui->QTW_keywords->rowCount()-1; i++)
        pmd.keywords << ui->QTW_keywords->item(i,0)->text();
    preview.set_meta_data(pmd);
    save_photo_meta_data();
}


//For now, save the metadata right away
void MainWindow::save_photo_meta_data()
{
    PhotoMetaData pmd = preview.get_metadata();
    if(!pmd.valid) return; //in valid metadata, don't bother saving
    pmd.caption = ui->captionEdit->text();
    pmd.photo_date = ui->dateTimeEdit->dateTime();
    preview.set_meta_data(pmd);
    save_metadata(preview.get_metadata());
    ui->captionEdit->setStyleSheet("");
    ui->dateTimeEdit->setStyleSheet("");
}

//Open the preview photo in a proper external viewer
void MainWindow::show_preview_external()
{
    //QDesktopServices::openUrl(preview.get_photo_url());
}

//Funny thing - if we try to use native dialog, it screws up
void MainWindow::select_photo_root()
{
    QSettings settings;
    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly | QFileDialog::DontUseNativeDialog;
    QString directory = QFileDialog::getExistingDirectory(this,
                                tr("Choose photo root"),
                                settings.value("photo root", "/").toString(),
                                options);
    if (!directory.isEmpty()) {
        settings.setValue("photo root", directory);
        settings.remove("last descent");//need to force a re trawling
        setup_photo_root();
        crawl();
    }
}

void MainWindow::crawl()
{
    QSettings settings;
    if(settings.contains("photo root")) {
        emit stop_crawl();
        disk_crawler.restart(photos_root);
    } else
        qDebug() << "Photo root not set";
}

//Disable the mainwindow, fetch photo list from the database, insert
//them into the ribbon and then return control to the main window
void MainWindow::load_photo_list()
{
    QSettings settings;
    this->setEnabled(false);
    int datetime_row_interval = settings.value("date marker row interval", 100).toInt();//
    ribbon->set_dateprint_row_interval(datetime_row_interval);
    ribbon->replace_tiles(get_all_photos());
    ui->QL_preview->setText("Photos imported");
    setWindowTitle("Chhobi");
    this->setEnabled(true);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
   if(obj == (QObject*)ui->mailLabel)
       if(event->type() == QEvent::MouseButtonPress) {
           QMouseEvent* this_evt = (QMouseEvent *)event;
           if (this_evt->button() == Qt::LeftButton) {
               QDrag *drag = new QDrag(this);
               drag->setPixmap(QPixmap(":/Images/Icons/chiti.png"));
               drag->setHotSpot(QPoint(drag->pixmap().width()/2,
                                       drag->pixmap().height()/2));
               QMimeData *mimeData = new QMimeData;
               mimeData->setUrls(resized_photos);
               drag->setMimeData(mimeData);
               drag->exec();
           }
       }
   // standard event processing
   return QObject::eventFilter(obj, event);
}

void MainWindow::resize_photos()
{
    ui->mailLabel->setEnabled(false);

    PhotoMetaData dummy;
    QSize max_size(ui->spinBox->value(), ui->spinBox->value());
    resized_root = QDir::temp();
    QList<PhotoInfo> tiles = hold_ribbon->get_all_tiles();
    QList<PhotoInfo>::iterator i;
    resized_photos.clear();
    ui->progressBar->setMaximum(tiles.count());
    int photo_count = 0;
    for (i = tiles.begin(); i != tiles.end(); ++i) {
        if(photos_root.exists(i->relative_file_path)) {
            QFileInfo orig_file(photos_root.absoluteFilePath(i->relative_file_path)),
                    resized_file(resized_root.absoluteFilePath(i->relative_file_path));
            if(i->type == PHOTO) {
                resized_root.mkpath(resized_file.absolutePath());
                fetch_image(orig_file.absoluteFilePath(), max_size, dummy).save(resized_file.absoluteFilePath());
            } else {//Probably a movie, copy over?
                qDebug() << i->relative_file_path;
            }
            resized_photos << QUrl(resized_file.absoluteFilePath());
        }
        photo_count++;
        ui->progressBar->setValue(photo_count);
        QApplication::processEvents();
    }
    ui->mailLabel->setEnabled(true);
}

//Open the folder where we've kept the resized photos
void MainWindow::show_resized_folder()
{
    if(resized_photos.count() > 0)
        QDesktopServices::openUrl(QUrl("file:///"
            + resized_root.path()));
}
