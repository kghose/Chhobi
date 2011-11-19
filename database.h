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

/*
Handles all access to the sqlite database. It uses the PhotoMetaData structure
to pass data back and forth. All the information stored on the database is
merely a copy of the photo metadata to speed up searches based on date, caption
or keyword. The keywords are called "collections".

This class contains functions to search through folders containing pictures,
find the ones missing from the database, or changed since the last search, and
import the metadata for those photos into the database.

Other functions allow us to search the database for photos meeting certain
criteria.
 */

class Database : public QObject
{
    Q_OBJECT

    bool keep_running; //The slot stop() sets this to false allowing us to abort the import function
    QDir photos_root; //The root from which we start looking for photos
    QDateTime last_descent; //The last time we checked for photos
    QSqlDatabase db; //Handle to our sqlite database

public:
    Database();
    ~Database();

    //Initialization functions
    bool open(QFileInfo dbpath); //Open or create a database in dbdir
    void set_photo_root(QDir root) {photos_root = root;}
    void set_last_descent(QDateTime qdt) {last_descent = qdt;}

    //Retrieval functions
    QList<unsigned int> get_all_photos();
    QList<unsigned int> get_photos_with_caption(QString);
    QList<unsigned int> get_photos_with_keyword(QString);
    QList<unsigned int> get_photos_with_no_keyword();
    QList<unsigned int> get_photos_by_sql(QString);

    //Misc housekeeping
    void clean_up_keywords_in_database();

    //Importing functions
    void descend(QDir &, bool isroot = false);
    void import_photo(QFileInfo );

signals:
    void now_searching(const QString &);//keep informed of current search

public slots:
    void stop() {keep_running = false;}

private:
    bool create_db(); //create a new empty database
};

QString sqlite_escape(QString strin);

#endif /*DATABASE_H_*/
