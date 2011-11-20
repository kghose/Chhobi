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

/*
  PhotoMetaData contains the metadata we load from disk when we pull up the
  photo for display.

  Photo contains the preview photo we are currently displaying

  */
#ifndef PHOTO_H
#define PHOTO_H

#include <QtCore>
#include <QPixmap>

//Needed for the Exiv data
struct Rational
{
    int numerator, denominator;
    QString pretty_print();
};

enum FileType {PHOTO, MOVIE};
struct PhotoMetaData
{
    //Housekeeping
    bool valid;
    QString absolute_file_path, file_name;
    FileType type;

    //Read/write properties
    QString caption;//photo caption 2000 characters max
    QDateTime photo_date;     //Date/Time Original
    int rotation_angle; //In degrees counterclockwise, read off the camera exif data
    QStringList keywords; //keywords associated with the photo


    //Read only properties
    int iso;
    Rational exposure_time, fnumber, focal_length;
    QString camera_model, lens_model;
};

class Photo
{
    unsigned int unique_id;
    QPixmap preview;
    PhotoMetaData meta_data;

public:
    Photo();
    void set_id(unsigned int id) {unique_id=id;}
    void set_meta_data(PhotoMetaData pmd) {meta_data = pmd;}
    void set_pixmap(QPixmap pm) {preview=pm;}

    QPixmap get_photo() {return preview;}  //return the preview pixmap
    PhotoMetaData get_metadata() {return meta_data;}
};

#endif // PHOTO_H
