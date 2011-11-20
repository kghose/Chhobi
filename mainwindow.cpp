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
    ui->setupUi(this);
    settings_parameters();
    setup();
    setup_connections();
    load_photos();
}

MainWindow::~MainWindow()
{
    delete ui;

    //Remove in production version
    //QSettings settings;
    //settings.clear();
}

void MainWindow::settings_parameters()
{
    QCoreApplication::setOrganizationName("BengalBionics");
    QCoreApplication::setOrganizationDomain("bengalbionics.com");
    QCoreApplication::setApplicationName("Chhobi");
}

void MainWindow::setup()
{
    ribbon = new PhotoRibbon();
    ui->QGV_timeline->setScene(ribbon);

    hold_ribbon = new PhotoRibbon();
    ui->QGV_hold->setScene(hold_ribbon);
}

void MainWindow::setup_connections()
{
    //Menu
    QObject::connect(ui->actionSet_Root, SIGNAL(triggered()),
            this, SLOT(set_photo_root()));

    //Ribbon
    QObject::connect(ribbon, SIGNAL(preview_id(PhotoInfo)),
            this, SLOT(set_preview_photo(PhotoInfo)));
    QObject::connect(ribbon, SIGNAL(selectionChanged()),
            this, SLOT(ribbon_selection_changed()));
    QObject::connect(hold_ribbon, SIGNAL(preview_id(PhotoInfo)),
            this, SLOT(set_preview_photo(PhotoInfo)));

    //Editing controls
    QObject::connect(ui->captionEdit, SIGNAL(textEdited(QString)),
            this, SLOT(photo_caption_changed()));
    QObject::connect(ui->captionEdit, SIGNAL(returnPressed()),
            this, SLOT(save_photo_meta_data()));
    QObject::connect(ui->dateTimeEdit, SIGNAL(editingFinished()),
            this, SLOT(save_photo_meta_data()));

    //Database crawler
    QObject::connect(&db, SIGNAL(now_searching(QString)),
            ui->QL_preview, SLOT(setText(QString)));
}

void MainWindow::load_preview_photo(QString absolute_file_name)
{
    preview.set_meta_data(load_metadata(absolute_file_name));
    QImage pmI(absolute_file_name);
    if(pmI.isNull())
        pmI.load(":/Images/Icons/chhobi-icon.png");

    preview.set_pixmap(QPixmap::fromImage(
          rotate(preview.get_metadata().rotation_angle, pmI.scaled(ui->QL_preview->size(),Qt::KeepAspectRatio))));

}

void MainWindow::set_preview_photo(PhotoInfo pi)
{
    QSettings settings;
    QString photo_root = settings.value("photo root").toString();//Shouldn't need a default
    QString absolute_file_name = photo_root + "/" + pi.relative_file_path;
    load_preview_photo(absolute_file_name);

    PhotoMetaData pmd = preview.get_metadata();

    ui->QL_preview->setPixmap(preview.get_photo().scaled(ui->QL_preview->size(),
                                                          Qt::KeepAspectRatio));
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
        metadata_string = "";
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
void MainWindow::ribbon_selection_changed()
{
    hold_ribbon->add_ids(ribbon->get_selected_ids());
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
    pmd.caption = ui->captionEdit->text();
    pmd.photo_date = ui->dateTimeEdit->dateTime();
    preview.set_meta_data(pmd);
    save_metadata(preview.get_absolute_file_path(), preview.get_metadata());
    ui->captionEdit->setStyleSheet("");
    ui->dateTimeEdit->setStyleSheet("");
}

//Open the preview photo in a proper external viewer
void MainWindow::show_preview_external()
{
    QDesktopServices::openUrl(preview.get_photo_url());
}

//Funny thing - if we try to use native dialog, it screws up
void MainWindow::set_photo_root()
{
    QSettings settings;
    QString photo_root = settings.value("photo root", "/").toString();
    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly | QFileDialog::DontUseNativeDialog;
    QString directory = QFileDialog::getExistingDirectory(this,
                                tr("Choose photo root"),
                                photo_root,
                                options);
    if (!directory.isEmpty()) {
        settings.setValue("photo root", directory);
        load_photos();
    }
}

//Disable the mainwindow, open the database, import/refresh new photos, insert
//them into the ribbon and then return control to the main window
void MainWindow::load_photos()
{
    this->setEnabled(false);
    QSettings settings;
    if(!settings.contains("photo root")) {
        set_photo_root();
    } else {
        QString photo_root = settings.value("photo root").toString();//Shouldn't need a default
        if(!settings.contains("database file name"))
            settings.setValue("database file name", QDir::home().absolutePath() + "/Source/Sandbox/Haba/Baba/chhobi.sqlite3");
        QFileInfo dbpath(settings.value("database file name").toString());
        db.open(dbpath);
        QStringList name_filters; name_filters << "*.jpg" << "*.jpeg" << "*.png" << "*.tiff" << "*.avi";//TODO other
        QDir dir(photo_root);
        dir.setNameFilters(name_filters);
        dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);
        dir.setSorting(QDir::Time | QDir::DirsFirst);
        db.descend(dir, true);//true -> this is a root
        ribbon->set_ids(db.get_all_photos());
        ui->QL_preview->setText("Photos imported");
        this->setEnabled(true);
    }
}

void MainWindow::test()
{
    /*
    QList<unsigned int> ids;
    for(int n=0; n < 30000; n++)
        ids.append(n+1);
    ribbon->set_ids(ids);*/
}
