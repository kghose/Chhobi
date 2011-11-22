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
        "id INTEGER PRIMARY KEY, filepath text, caption text, keywords text, datetaken integer, tile_color integer);");
    query.exec("create table keywords("
        "id INTEGER PRIMARY KEY,"
        "keyword text);");
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
    query.setForwardOnly(true);//should save memory
    QList<PhotoInfo> pl;
    query.exec(query_str);
    while(query.next()) {
        PhotoInfo this_pi;
        this_pi.id = query.value(0).toInt();
        this_pi.relative_file_path = query.value(1).toString();
        pl.append(this_pi);
    }
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
    bool modified;
    if(isroot) {
        keep_running = true;
        photos_root = dir;
        QSettings settings;
        last_descent = settings.value("last descent").toDateTime();
    }
    if(!keep_running) return;//best to quit out without changing anything
    emit now_searching(photos_root.relativeFilePath(dir.path()));
    QApplication::processEvents();
    if(QFileInfo(dir,".").lastModified() >= last_descent)
        modified = true;
    else
        modified = false;

    QFileInfoList children = dir.entryInfoList(); //children has subdirectories and photos
    for(int n = 0; n < children.size(); n++) {
        if(!keep_running) return;
        if((n % 50)==49) {//For subdir with lots files, be courteous
            emit now_searching(children[n].baseName());
            QApplication::processEvents();
        }
        if(children[n].isDir()) {//directory, must recurse into
            QDir d = dir;
            d.setPath(children[n].filePath());
            descend(d);
        } else if(modified) {//image file in a directory that has been modifed, import and synch?
            if(children[n].lastModified() >= last_descent) {//yup, changed
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
    PhotoMetaData pmd = load_metadata(qfi.absoluteFilePath());
    int cnt = pmd.keywords.count();
    QString kwds;
    for(int i=0; i<cnt; i++)
        kwds += "<" + pmd.keywords[i] + ">";

    QString relative_file_path = photos_root.relativeFilePath(qfi.absoluteFilePath());
    QString query_str = "SELECT id FROM photos WHERE filepath LIKE '" + relative_file_path + "'";
    QSqlQuery query;
    query.exec(query_str);
    if(query.next())
        query_str = "UPDATE photos SET "
                "filepath='" + relative_file_path + "',"
                "caption='" + pmd.caption + "',"
                "keywords='" + kwds + "',"
                "datetaken=" + QString::number(pmd.photo_date.toTime_t()) +
                " WHERE id=" + query.value(0).toString();
    else
        query_str = "INSERT INTO photos (id, filepath, caption, keywords, datetaken) VALUES("
                "NULL,"
                "'" + relative_file_path +"',"
                "'" + pmd.caption +"',"
                "'" + kwds +"'," +
                QString::number(pmd.photo_date.toTime_t()) +");";
    query.exec(query_str);
    //int id = query.lastInsertId().toInt(&ok);
    insert_keywords(pmd.keywords);
}

void Database::purge_photo(PhotoInfo pi)
{
    QString query_str = "DELETE FROM photos WHERE id=" + QString::number(pi.id);
    QSqlQuery query;
    query.exec(query_str);
    //gotta get rid of keyword assocs?
}

void Database::insert_keywords(QStringList kwl)
{
    int cnt = kwl.count();
    QSqlQuery query;
    QString query_str;
    for(int i=0; i<cnt; i++) {
        query_str = "SELECT id FROM keywords WHERE keyword LIKE '" + kwl[i] + "'";
        query.exec(query_str);
        if(!query.next()) {
            query_str = "INSERT INTO keywords (id, keyword) VALUES(NULL, '" + kwl[i] + "')";
            query.exec(query_str);
        }
    }
}

/*
 * sqlite escapes single quotes with another single quote
 * http://www.mail-archive.com/sqlite-users@sqlite.org/msg16753.html
 */
QString sqlite_escape(QString strin) {
    return strin.replace("'","''");
}
