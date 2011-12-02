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
#ifndef THREADEDDISKCRAWLER_H
#define THREADEDDISKCRAWLER_H

#include <QThread>
#include <QSet>
#include "database.h"

class ThreadedDiskCrawler : public QThread
{
    Q_OBJECT

    volatile bool keep_running; //The slot stop() sets this to false allowing us to abort the import function
    bool any_new_photos;
    QDir photos_root; //The root from which we start looking for photos
    QDateTime last_descent; //The last time we checked for photos
    QSqlDatabase db; //Handle to our sqlite database
    QHash<QString,int> directories;

    void load_directories_in_database();//Load everything from the database
    void descend(QDir &);
    int import_directory(QString);
    void import_photo(QFileInfo, int);
    void insert_keywords(QStringList);
    QHash<QString,int> all_files_under_dir(int dir_id);

    //Misc housekeeping
    void clean_up_keywords_in_database();
    void purge_photo(int);
    void purge_zombie_directories();

public:
    void run();
    bool found_new_photos() {return any_new_photos;}

signals:
    void now_searching(const QString &);//keep informed of current search

public slots:
    void stop() {keep_running = false;}
    void restart(QDir);
};

#endif // THREADEDDISKCRAWLER_H
