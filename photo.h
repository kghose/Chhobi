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

#ifndef PHOTO_H
#define PHOTO_H

#include <QString>
#include <QPixmap>
#include <QUrl>
class Photo
{
    unsigned int unique_id;
    QString absolute_file_path;
    QPixmap preview;

public:
    Photo();
    void set_photo(unsigned int id); //fetch the photo and metadata
    QPixmap get_photo();  //return the oreview pixmap
    QUrl get_photo_url(); //return absolute file path
    void save_metadata(); //save any changes to the metadata
};

#endif // PHOTO_H
