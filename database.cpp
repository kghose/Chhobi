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
    QSqlQuery query;
    query.prepare("SELECT id,filepath,tile_color,type,datetaken FROM photos ORDER BY datetaken DESC");
    return get_photos_by_query(query);
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

QList<PhotoInfo> Database::get_photos_by_query(QSqlQuery query)
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
        db.transaction();//TODO check for errors
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
    if(isroot) {
        db.commit();//TODO error checking
        if(keep_running) {//If we've got this far, we were allowed to finish our crawl
            QSettings settings;
            settings.setValue("last descent", QDateTime::currentDateTime());
        } else
            qDebug() << "Terminated";
    }
}

/*
int compute_tile_color(QFileInfo qfi)
{
    QImageReader qir(qfi.absoluteFilePath());
    qir.setScaledSize(QSize(100,100));
    qir.setQuality(0);
    QImage pmI = qir.read();
    int this_rgb, mean_r = 0, mean_g = 0, mean_b = 0;
    for(int x=0; x<pmI.width(); x++)
        for(int y=0; y<pmI.height(); y++){
        this_rgb = pmI.pixel(x,y) & 0xffffff;
        mean_r += (this_rgb >> 16) & 0xff;
        mean_g += (this_rgb >> 8) & 0xff;
        mean_b += (this_rgb) & 0xff;
    }
    int N = 10000;
    return (((mean_r/N) & 0xff) << 16)  + (((mean_g/N) & 0xff) << 8) + ((mean_b/N) & 0xff);
}
*/

//This is a new photo to be inserted into the database
void Database::import_photo(QFileInfo qfi)
{
    PhotoMetaData pmd = load_metadata(qfi.absoluteFilePath());
    int cnt = pmd.keywords.count();
    QString kwds;
    for(int i=0; i<cnt; i++)
        kwds += "<" + pmd.keywords[i] + ">";

    QString relative_file_path = photos_root.relativeFilePath(qfi.absoluteFilePath());
    //QString query_str = "SELECT id FROM photos WHERE filepath LIKE '" + relative_file_path + "'";
    QSqlQuery query;
    query.prepare("SELECT id FROM photos WHERE filepath LIKE :rfp");
    query.bindValue(":rfp", relative_file_path);
    query.exec();
    if(query.next()) {
        int id = query.value(0).toInt();
        query.prepare("UPDATE photos SET filepath=:filepath, caption=:caption, "
                      "keywords=:keywords, datetaken=:datetaken, tile_color=:tile_color, "
                      "type=:type WHERE id=:id");
        query.bindValue(":id", id);
    } else {
        query.prepare("INSERT INTO photos (id, filepath, caption, keywords, datetaken, tile_color, type, rowtype) "
                      "VALUES(NULL, :filepath, :caption, :keywords, :datetaken, :tile_color, :type)");
    }
    query.bindValue(":filepath", relative_file_path);
    query.bindValue(":caption", pmd.caption);
    query.bindValue(":keywords", kwds);
    query.bindValue(":datetaken", pmd.photo_date);
    query.bindValue(":type", pmd.type);
    //if(pmd.type==PHOTO)
        //query.bindValue(":tile_color", compute_tile_color(qfi));
    //else
    //    query.bindValue(":tile_color", 0x0000ff);
    query.exec();
    //int id = query.lastInsertId().toInt(&ok);
    insert_keywords(pmd.keywords);
}

//Move into crawler
void Database::purge_photo(PhotoInfo pi)
{
    QSqlQuery query;
    query.prepare("DELETE FROM photos WHERE id=:id");
    query.bindValue(":id", pi.id);
    query.exec();
    //gotta get rid of keyword assocs?
}

void Database::insert_keywords(QStringList kwl)
{
    int cnt = kwl.count();
    QSqlQuery query;
    for(int i=0; i<cnt; i++) {
        query.prepare("SELECT id FROM keywords WHERE keyword LIKE :keyword");
        query.bindValue(":keyword", kwl[i]);
        query.exec();
        if(!query.next()) {
            query.prepare("INSERT INTO keywords (id, keyword) VALUES(NULL,:keyword)");
            query.bindValue(":keyword", kwl[i]);
            query.exec();
        }
    }
}
