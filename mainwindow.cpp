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
    setup_connections();

    //setup database
    QSettings settings;
    if(!settings.contains("database file name"))
        settings.setValue("database file name", db_location);
    QFileInfo dbpath(settings.value("database file name").toString());
    db.open(dbpath);
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

    hold_ribbon = new PhotoRibbon();
    ui->QGV_hold->setScene(hold_ribbon);
}

void MainWindow::setup_connections()
{
    //Event filter for drag on mailing label
    ui->mailLabel->installEventFilter(this);

    //Menu
    QObject::connect(ui->actionSet_Root, SIGNAL(triggered()),
            this, SLOT(set_photo_root()));

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
    QObject::connect(&db, SIGNAL(now_searching(QString)),
            ui->QL_preview, SLOT(setText(QString)));
    QObject::connect(this, SIGNAL(db_stop()),
            &db, SLOT(stop()));

}

void MainWindow::set_preview_photo(PhotoInfo pi)
{
    PhotoMetaData pmd;
    QString absolute_file_name = photos_root.absoluteFilePath(pi.relative_file_path);
    this->statusBar()->showMessage(absolute_file_name);
    QImage pmI;
    if(photos_root.exists(pi.relative_file_path)) {
        pmd = load_metadata(absolute_file_name);
        QImageReader qir(absolute_file_name);
        QSize orig = qir.size();
        float ratio = 0;
        if(orig.width() > orig.height())
            ratio = (float)ui->QL_preview->size().width()/(float)orig.width();
        else
            ratio = (float)ui->QL_preview->size().height()/(float)orig.height();
        qir.setScaledSize(ratio*orig);
        pmI = qir.read();
        //pmI.load(absolute_file_name);
    } else {//Photo reference in db but not on disk
        pmd.valid = false;
        pmd.rotation_angle = 0;
        pmI.load(":/Images/Icons/chhobi-icon.png");
        db.purge_photo(pi);
    }

    preview.set_meta_data(pmd);
    if(pmd.type == PHOTO)
        preview.set_pixmap(QPixmap::fromImage(
          rotate(preview.get_metadata().rotation_angle, pmI.scaled(ui->QL_preview->size(),Qt::KeepAspectRatio))));
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
void MainWindow::set_photo_root()
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
        load_photo_list();
    }
}

//Disable the mainwindow, open the database, import/refresh new photos, insert
//them into the ribbon and then return control to the main window
void MainWindow::load_photo_list()
{
    this->setEnabled(false);
    QSettings settings;
    if(!settings.contains("photo root")) {
        set_photo_root();
    } else {
        photos_root = QDir(settings.value("photo root").toString());//Shouldn't need a default
        int datetime_row_interval = settings.value("date marker row interval", 100).toInt();//
        ribbon->set_dateprint_row_interval(datetime_row_interval);
        QDir dir(photos_root);
        dir.setNameFilters(name_filters);
        dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);
        dir.setSorting(QDir::Time | QDir::DirsFirst);
        db.descend(dir, true);//true -> this is a root
        ribbon->replace_tiles(db.get_all_photos());
        ui->QL_preview->setText("Photos imported");
        this->setEnabled(true);
    }
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
                QImageReader qir(orig_file.absoluteFilePath());
                QSize orig = qir.size();
                float ratio = 0, new_size = ui->spinBox->value();
                if(orig.width() > orig.height())
                    ratio = (float)new_size/(float)orig.width();
                else
                    ratio = (float)new_size/(float)orig.height();
                qir.setScaledSize(ratio*orig);
                resized_root.mkpath(resized_file.absoluteDir().path());
                qir.read().save(resized_file.absoluteFilePath());
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
