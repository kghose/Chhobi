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

    hold_ribbon = new PhotoRibbon();
    ui->QGV_hold->setScene(hold_ribbon);
}

void MainWindow::setup_connections()
{
    QObject::connect(ribbon, SIGNAL(preview_id(unsigned int)),
            this, SLOT(set_preview_photo(unsigned int)));
}

void MainWindow::set_preview_photo(unsigned int id)
{
    preview.set_photo(id);
    ui->QL_preview->setPixmap(preview.get_photo().scaled(
       ui->QL_preview->width(),
       ui->QL_preview->height(),
       Qt::KeepAspectRatio));
}

void MainWindow::test()
{
    QList<unsigned int> ids;
    for(int n=0; n < 10000; n++)
        ids.append(n+1);
    ribbon->set_ids(ids);
}
