/*
 * This file is part of Chhobi - a photo organizer program.
 * Copyright (c) 2008 Kaushik Ghose kaushik.ghose@gmail.com
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

#ifndef DATABASE_H_
#define DATABASE_H_

#include <QtCore>
#include <QtSql>//from http://doc.trolltech.com/4.3/qtsql.html#details
#include "photoribbon.h"//needed for PhotoInfo

const QString conn_name("chhobi database");

//Initialization functions
bool open_database(QFileInfo dbpath); //Open or create a database in dbdir
bool create_db(); //create a new empty database

//Misc functions
QStringList get_keywords_in_db();

//Retrieval functions
QList<PhotoInfo> get_all_photos();
QList<PhotoInfo> get_photos_with_caption(QString);
QList<PhotoInfo> get_photos_with_keyword(QString);
//QList<PhotoInfo> get_photos_with_no_keyword();
QList<PhotoInfo> get_photos_by_query(QSqlQuery);

#endif /*DATABASE_H_*/
