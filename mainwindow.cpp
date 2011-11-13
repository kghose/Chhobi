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

    hold_ribbon = new PhotoRibbon();
    ui->QGV_hold->setScene(hold_ribbon);
}

void MainWindow::setup_connections()
{
    QObject::connect(ribbon, SIGNAL(preview_id(unsigned int)),
            this, SLOT(set_preview_photo(unsigned int)));
    QObject::connect(ribbon, SIGNAL(selectionChanged()),
            this, SLOT(ribbon_selection_changed()));
    QObject::connect(hold_ribbon, SIGNAL(preview_id(unsigned int)),
            this, SLOT(set_preview_photo(unsigned int)));

    QObject::connect(ui->captionEdit, SIGNAL(textEdited(QString)),
            this, SLOT(photo_caption_changed()));
    QObject::connect(ui->dateTimeEdit, SIGNAL(editingFinished()),
            this, SLOT(photo_date_changed()));

    QObject::connect(ui->captionEdit, SIGNAL(returnPressed()),
            this, SLOT(save_photo_meta_data()));
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

void MainWindow::set_preview_photo(unsigned int id)
{
    int fno = 5661;
    QString absolute_file_name = QString("/Users/kghose/Source/Sandbox/2011-10-16/DSC_")
            + QString::number(fno+id) + QString(".JPG");
    preview.set_photo(id);
    load_preview_photo(absolute_file_name);

    ui->QL_preview->setPixmap(preview.get_photo().scaled(ui->QL_preview->size(),
                                                          Qt::KeepAspectRatio));
    ui->captionEdit->setText(preview.get_metadata().caption);
    ui->dateTimeEdit->setDateTime(preview.get_metadata().photo_date);

    set_metadata_table(preview.get_metadata());
}

/*void MainWindow::set_metadata_table(PhotoMetaData pmd)
{
    ui->QTW_metadata->setItem(0,0,new QTableWidgetItem("Exposure"));
    ui->QTW_metadata->setItem(0,1,new QTableWidgetItem(pmd.exposure_time.pretty_print() + "s"));
    ui->QTW_metadata->setItem(1,0,new QTableWidgetItem("f-stop"));
    ui->QTW_metadata->setItem(1,1,new QTableWidgetItem(pmd.fnumber.pretty_print()));
    ui->QTW_metadata->setItem(2,0,new QTableWidgetItem("ISO"));
    ui->QTW_metadata->setItem(2,1,new QTableWidgetItem(QString::number(pmd.iso)));
    ui->QTW_metadata->setItem(3,0,new QTableWidgetItem("Focal len"));
    ui->QTW_metadata->setItem(3,1,new QTableWidgetItem(pmd.focal_length.pretty_print() + "mm"));
    ui->QTW_metadata->setItem(4,0,new QTableWidgetItem("Camera"));
    ui->QTW_metadata->setItem(4,1,new QTableWidgetItem(pmd.camera_model));
    ui->QTW_metadata->setItem(5,0,new QTableWidgetItem("Lens"));
    ui->QTW_metadata->setItem(5,1,new QTableWidgetItem(pmd.lens_model));
}*/

void MainWindow::set_metadata_table(PhotoMetaData pmd)
{
    ui->QTW_metadata->setItem(0,0,new QTableWidgetItem(pmd.exposure_time.pretty_print() + "s"));
    ui->QTW_metadata->setItem(1,0,new QTableWidgetItem("f" + pmd.fnumber.pretty_print()));
    ui->QTW_metadata->setItem(2,0,new QTableWidgetItem(QString::number(pmd.iso) + " iso"));
    ui->QTW_metadata->setItem(3,0,new QTableWidgetItem(pmd.focal_length.pretty_print() + "mm"));
    ui->QTW_metadata->setItem(4,0,new QTableWidgetItem(pmd.camera_model));
    ui->QTW_metadata->setItem(5,0,new QTableWidgetItem(pmd.lens_model));
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

void MainWindow::test()
{
    QList<unsigned int> ids;
    for(int n=0; n < 30000; n++)
        ids.append(n+1);
    ribbon->set_ids(ids);
}
