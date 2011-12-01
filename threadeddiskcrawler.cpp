#include "threadeddiskcrawler.h"
#include "exivmanager.h"

void ThreadedDiskCrawler::run()
{
    db = QSqlDatabase::database(conn_name);//conn_name is in database.h
    if(!db.open()) {
        qDebug() << db.lastError();
        return;
    }
    keep_running = true;
    QSettings settings;
    last_descent = settings.value("last descent").toDateTime();
    settings.setValue("last descent", QDateTime::currentDateTime());
    //Last descent time should be at start of descent to be conservative
    load_directories_in_database();
    db.transaction();//TODO check for errors
    descend(photos_root);
    purge_zombie_directories();
    db.commit();//TODO error checking
}

//need to call stop slot before this
//Don't forget to put apropriate filters on dir (e.g. .jpeg .png etc)
void ThreadedDiskCrawler::restart(QDir dir)
{
    while(isRunning())
        ;
    photos_root = dir;
    start();
}

//Load list of directories in the database
void ThreadedDiskCrawler::load_directories_in_database()
{
    directories.clear();
    QSqlQuery query(db);
    query.prepare("SELECT id, relpath FROM directories");
    query.exec();
    directories.reserve(query.size()+100);
    while(query.next())
        directories[query.value(1).toString()] = query.value(0).toInt();
}

QHash<QString,int> ThreadedDiskCrawler::all_files_under_dir(int dir_id)
{
    QSqlQuery query(db);
    query.prepare("SELECT id, filepath FROM photos WHERE parentdir_id=:parentdir_id");
    query.bindValue(":parentdir_id", dir_id);
    query.exec();
    QHash<QString,int> files;
    files.reserve(query.size()+20);
    while(query.next())
        files[query.value(1).toString()] = query.value(0).toInt();
    return files;
}

/*Importing function
The most common reason for changes in the pictures are that we edit the metadata
and save the changes to pictures anywhere in the directory tree. These changes
need to be reimported into the database and making these changes causes the
parent directory (and only the parent directory) last modified time to be changed.

The next common reason for changes is that new pictures have been downloaded from
the camera and have been placed in a new folder at some distance under the root.
This causes the modified time of the folder/pictures and parent folder to
change.

The least common reason for changes is that an old folder (or picture) is copied
over into the directory tree. This causes the parent folder last modified time
to change but the modified time of the folder and its files can be old (as old
as original creation date)

Our job is to go through the directory tree and
1. Import new photos
2. Reimport changed ones
3. Remove photos from the database that no longer exist on disk
4. Do it as quickly as possible

We do this as follows:

The importing function is a recursive function that starts at the photo_root
and works its way down the tree.

At each directory it pulls up the current directory entry from the database. If
the directory is not in the database then all child files are imported.

If the directory has been imported, if the last modified date is later than the
last descent date the list of files under this directory are pulled up from the
database. Each file on disk is checked against the database contents. If the
file exists and its last modified date is after the last descent date it is
reimported. If the file does not exist, it is imported. If there is a file in
the database and not on disk, it is purged.

If a directory is unmodified none of the enclosed files are read.

Finally, all the child directories are listed and sequentially recursed into.

After we are done, we find out the zombie directories (present in db, absent on
disk) and remove them as well as their enclosed files.

The last descent time is set as the start of the descent. This is because the
descent takes time and by the time it has finished we might have made changes to
some of the photos.

As we descend into the file system we ocassionaly send out signals informing
the main program of what we are up to so it knows we haven't gone narcoleptic or
some such.


Database features just to improve importing performance:
1. Directories are stored in a separate table
2. Photo entries in the photos table have a dir_id field which indicates the
id of the enclosing directory. This allows us to pull up all the photos under a
given directory.
3. We load all the directories into a variable at the start (into a QSet) alongwith
their ids so we don't need to repeatedly query the db for each directory - and
we will recurse every directory
4. We query the db to find what files are under current dir. We won't have to do
this very often - only when our directory is modified.


dir is the absolute path, but everything stored in the db is relative to the
photo_root so we need to make use of relativeFilePath
*/
void ThreadedDiskCrawler::descend(QDir &dir)
{
    if(!keep_running) return;//best to quit out without changing anything

    QString relpath = photos_root.relativeFilePath(dir.path());
    emit now_searching(relpath);

    bool newdir = true,  //is this new
         modified = true;//has this been modified since the last descent
    int dir_id;
    if(directories.contains(relpath)) {
        newdir = false;
        dir_id = directories[relpath];
        if(QFileInfo(dir,".").lastModified() < last_descent) //changed directory?
            modified = false;
        directories.remove(relpath);//in the end after completing the descent, we will end up with zombie directories if any
    } else {
        dir_id = import_directory(relpath);
    }

    //Always recurse child directories
    QFileInfoList children = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
    QFileInfoList::iterator qfi;
    for(qfi = children.begin(); qfi != children.end(); ++qfi) {
        QDir d = dir;
        d.setPath((*qfi).filePath());
        descend(d);
    }

    int counter = 0;
    if(newdir || modified) {//Have to process all enclosed files
        QHash<QString,int> files_in_db = all_files_under_dir(dir_id);
        QFileInfoList children = dir.entryInfoList(QDir::Files);
        QFileInfoList::iterator qfi;
        for(qfi = children.begin(); qfi != children.end(); ++qfi) {
            if(!keep_running) return;
            if((counter % 50)==49) //For subdir with lots files, be courteous
                emit now_searching((*qfi).baseName());
            if(newdir || ((*qfi).lastModified() >= last_descent))
                import_photo(*qfi, dir_id);
            files_in_db.remove(photos_root.relativeFilePath((*qfi).absoluteFilePath()));
            //in the end we will be left with zombie files
            ++counter;
        }

        QHash<QString,int>::iterator zf;
        for(zf=files_in_db.begin(); zf!= files_in_db.end(); ++zf)
            purge_photo(*zf);
    }
}

//use the relative path
int ThreadedDiskCrawler::import_directory(QString relpath)
{
    QSqlQuery query(db);
    //We know this does not exist.
    query.prepare("INSERT INTO directories (id, relpath) VALUES (NULL, :relpath)");
    query.bindValue(":relpath", relpath);
    query.exec();
    return query.lastInsertId().toInt();
}

int compute_tile_color(QFileInfo qfi)
{
    QImageReader qir(qfi.absoluteFilePath());
    qir.setScaledSize(QSize(1,1));
    qir.setQuality(0);
    QImage pmI = qir.read();
    return pmI.pixel(0,0) & 0xffffff;
}

//This is a new photo to be inserted into the database
void ThreadedDiskCrawler::import_photo(QFileInfo qfi, int parentdir_id)
{
    PhotoMetaData pmd = load_metadata(qfi.absoluteFilePath());
    int cnt = pmd.keywords.count();
    QString kwds;
    for(int i=0; i<cnt; i++)
        kwds += "<" + pmd.keywords[i] + ">";

    QString relative_file_path = photos_root.relativeFilePath(qfi.absoluteFilePath());
    QSqlQuery query(db);
    query.prepare("SELECT id FROM photos WHERE filepath LIKE :rfp");
    query.bindValue(":rfp", relative_file_path);
    query.exec();
    if(query.next()) {
        int id = query.value(0).toInt();
        query.prepare("UPDATE photos SET filepath=:filepath, caption=:caption, "
                      "keywords=:keywords, datetaken=:datetaken, tile_color=:tile_color, "
                      "type=:type, parentdir_id=:parentdir_id WHERE id=:id");
        query.bindValue(":id", id);
    } else {
        query.prepare("INSERT INTO photos (id, filepath, caption, keywords, datetaken, tile_color, type, parentdir_id) "
                      "VALUES(NULL, :filepath, :caption, :keywords, :datetaken, :tile_color, :type, :parentdir_id)");
    }
    query.bindValue(":filepath", relative_file_path);
    query.bindValue(":caption", pmd.caption);
    query.bindValue(":keywords", kwds);
    query.bindValue(":datetaken", pmd.photo_date);
    query.bindValue(":type", pmd.type);
    query.bindValue(":parentdir_id", parentdir_id);
    if(pmd.type==PHOTO)
        query.bindValue(":tile_color", compute_tile_color(qfi));
    else //movie
        query.bindValue(":tile_color", 0x0000ff);
    query.exec();
    insert_keywords(pmd.keywords);
}

void ThreadedDiskCrawler::insert_keywords(QStringList kwl)
{
    int cnt = kwl.count();
    QSqlQuery query(db);
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

//If a file is in the DB but not on disk, it has been deleted
//We can detect this when we go through a directory changed after the last
//crawl and we find the file missing from disk.
void ThreadedDiskCrawler::purge_photo(int id)
{
    QSqlQuery query(db);
    query.prepare("DELETE FROM photos WHERE id=:id");
    query.bindValue(":id", id);
    query.exec();
    //gotta get rid of keyword assocs?
}

//don't forget to remove all the relevant file entries too!
void ThreadedDiskCrawler::purge_zombie_directories()
{
    QSqlQuery query(db);
    QHash<QString,int>::iterator zd;
    for(zd=directories.begin(); zd!= directories.end(); ++zd) {
        //don't forget to remove all the relevant file entries too!
        query.prepare("DELETE FROM photos WHERE parentdir_id=:id");
        query.bindValue(":id", *zd);
        query.exec();
        //Now get rid of subdirs
        query.prepare("DELETE FROM directories WHERE id=:id");
        query.exec();
    }
}
