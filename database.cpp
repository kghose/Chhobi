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
#include "database.h"
#include "exivmanager.h"

//Initialization functions
bool open_database(QFileInfo dbpath)//Open or create a database in dbdir
{
    bool create = false;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", conn_name);
    if(!dbpath.exists()) {
        QDir("/").mkpath(dbpath.absolutePath());
        create = true;
    }
    db.setDatabaseName(dbpath.absoluteFilePath());
    db.open(); //TODO: proper error checking?
    if(create) create_db();//assume we need to create db if we can't open it
    //TODO: Further error checking
    return true;
}

//db needs to be opened before using this
bool create_db() {
    QSqlDatabase db = QSqlDatabase::database(conn_name);
    if(!db.open()) {
        qDebug() << db.lastError();
        return false;
    }
    qDebug() << "Creating brand new database " << db.databaseName();
    QSqlQuery query(db);
    query.exec("CREATE TABLE photos("
        "id INTEGER PRIMARY KEY, filepath text, caption text, keywords text, "
        "datetaken integer, tile_color integer, type integer, parentdir_id integer);");
    query.exec("CREATE TABLE keywords("
        "id INTEGER PRIMARY KEY,"
        "keyword text);");
    query.exec("CREATE TABLE directories("
        "id INTEGER PRIMARY KEY,"
        "relpath text);");

    //TODO: error checking
    return true;
}

QStringList get_keywords_in_db()
{
    QSqlDatabase db = QSqlDatabase::database(conn_name);
    if(!db.open())
        qDebug() << db.lastError();
    QSqlQuery query(db);
    query.prepare("SELECT keyword FROM keywords ORDER BY keyword ASC");
    query.setForwardOnly(true);//should save memory
    QStringList kwds;
    query.exec();
    while(query.next()) kwds.append(query.value(0).toString());
    return kwds;
}

QList<PhotoInfo> get_all_photos()
{
    QSqlDatabase db = QSqlDatabase::database(conn_name);
    if(!db.open())
        qDebug() << db.lastError();
    QSqlQuery query(db);
    query.prepare("SELECT id,filepath,tile_color,type,datetaken FROM photos ORDER BY datetaken DESC");
    return get_photos_by_query(query);
}

QList<PhotoInfo> get_photos_with_caption(QString caption)
{
    QSqlDatabase db = QSqlDatabase::database(conn_name);
    if(!db.open())
        qDebug() << db.lastError();
    QSqlQuery query(db);
    query.prepare("SELECT id,filepath,tile_color,type,datetaken FROM photos WHERE caption LIKE :capt ORDER BY datetaken DESC");
    query.bindValue(":capt", "%" + caption + "%");
    return get_photos_by_query(query);
}

QList<PhotoInfo> get_photos_with_keyword(QString kwd)
{
    QSqlDatabase db = QSqlDatabase::database(conn_name);
    if(!db.open())
        qDebug() << db.lastError();
    QSqlQuery query(db);
    query.prepare("SELECT id,filepath,tile_color,type,datetaken FROM photos WHERE keywords LIKE :kwd ORDER BY datetaken DESC");
    query.bindValue(":kwd", "%<" + kwd + ">%");
    return get_photos_by_query(query);
}

QList<PhotoInfo> get_photos_by_query(QSqlQuery query)
{
    query.setForwardOnly(true);//should save memory
    QList<PhotoInfo> pl;
    query.exec();
    while(query.next()) {
        PhotoInfo this_pi;
        this_pi.id = query.value(0).toInt();
        this_pi.relative_file_path = query.value(1).toString();
        this_pi.tile_color = query.value(2).toInt();
        this_pi.type = (FileType) query.value(3).toInt();
        this_pi.photo_date = query.value(4).toDateTime();
        pl.append(this_pi);
    }
    return pl;
}
