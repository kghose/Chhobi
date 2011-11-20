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

//these limits are from
//http://www.controlledvocabulary.com/imagedatabases/iptc_naa.html#IPTCchart
const int max_caption_characters = 2000;
const int max_keyword_characters = 64;

Database::Database() {
    ;
}

Database::~Database() {
    db.close();
}

//db needs to be opened before using this
bool Database::create_db() {
    qDebug() << "Creating brand new database " << db.databaseName();
    QSqlQuery query;
    query.exec("create table photos("
        "id INTEGER PRIMARY KEY, filepath text, caption text, datetaken integer);");
    query.exec("create table keywords("
        "id INTEGER PRIMARY KEY,"
        "keyword text);");
    query.exec("create table photos_keywords("
        "photo_id integer,"
        "keyword_id integer);");
    //TODO: error checking
    return true;
}

//Initialization functions
bool Database::open(QFileInfo dbpath)//Open or create a database in dbdir
{
    bool create = false;
    db = QSqlDatabase::addDatabase("QSQLITE");
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

//Retrieval functions
QList<PhotoInfo> Database::get_all_photos()
{
    return get_photos_by_sql("SELECT id,filepath FROM photos ORDER BY datetaken DESC");
}

QList<PhotoInfo> Database::get_photos_with_caption(QString)
{
    ;//return get_photos_by_sql("SELECT * FROM photos WHERE caption LIKE '' ORDER BY datetaken DESC");
}

QList<PhotoInfo> Database::get_photos_with_keyword(QString)
{
    ;
}

QList<PhotoInfo> Database::get_photos_with_no_keyword()
{
    ;
}

QList<PhotoInfo> Database::get_photos_by_sql(QString query_str)
{
    QSqlQuery query;
    QList<PhotoInfo> pl;
    query.exec(query_str);
    while(query.next()) {
        PhotoInfo this_pi;
        this_pi.id = query.value(0).toInt();
        this_pi.relative_file_path = query.value(1).toString();
        pl.append(this_pi);
    }
    qDebug() << pl.count();
    return pl;
}


/*Importing function
This is a recursive function. It first lists all subdirectories and photo files
in `dir`. It imports the metadata from each file and then descends down each
subdirectory to repeat this action.

Normally this function would lockup the application, so we use
QApplication::processEvents().

The first time we call it, from the photo root we set isroot to be true. This
tells the function to set the instance variable keep_running to be true. keep_running
can be set to be false using the slot stop(). When keep_running switches to false
the descend function returns, all the way back to the top of the stack, finally
exiting.

As the function loops through it signals each directory it is descending into and
also files, every 100 files. So we can hook up the signal to a slot to tell our
parent we haven't gone narcoleptic or soem such.
*/
void Database::descend(QDir &dir, bool isroot)
{
    if(isroot) {
        keep_running = true;
        photos_root = dir;
        QSettings settings;
        last_descent = settings.value("last descent").toDateTime();
    }
    if(!keep_running) return;//best to quit out without changing anything
    emit now_searching(dir.path());
    QFileInfoList children = dir.entryInfoList(); //children has subdirectories and photos
    for(int n = 0; n < children.size(); n++) {
        if(!keep_running) return;
        if(children[n].lastModified() >= last_descent) {//this is either a subdirectory or photo that has been changed
            if(children[n].isDir()) {//directory, recurse into
                QDir d = dir;
                d.setPath(children[n].filePath());
                descend(d);
            } else {//image file, import and synch
                import_photo(children[n]);
            }
        }
    }
    if(isroot) {//If we've got this far, we were allowed to finish our crawl
        QSettings settings;
        settings.setValue("last descent", QDateTime::currentDateTime());
    }
}

//This is a new photo to be inserted into the database
void Database::import_photo(QFileInfo qfi)
{
    bool ok = false;
    QString relative_file_path = photos_root.relativeFilePath(qfi.absoluteFilePath());
    QString query_str = "SELECT id FROM photos WHERE filepath LIKE '" + relative_file_path + "'";
    QSqlQuery query;
    query.exec(query_str);
    if(query.next())
        query_str = "UPDATE photos SET filepath='" + relative_file_path +
                "', datetaken=" + QString::number(QDateTime::currentDateTime().toTime_t()) +
                " WHERE id=" + query.value(0).toString();
    else
        query_str = "INSERT INTO photos (id, filepath) values(NULL, '" + relative_file_path +"');";
    query.exec(query_str);
    int id = query.lastInsertId().toInt(&ok);
    //save_or_create_keywords(photo);
}

void Database::purge_photo(PhotoInfo pi)
{
    QString query_str = "DELETE FROM photos WHERE id=" + QString::number(pi.id);
    QSqlQuery query;
    query.exec(query_str);
    //gotta get rid of keyword assocs
}


/*
 * sqlite escapes single quotes with another single quote
 * http://www.mail-archive.com/sqlite-users@sqlite.org/msg16753.html
 */
QString sqlite_escape(QString strin) {
    return strin.replace("'","''");
}
