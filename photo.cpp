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

#include "photo.h"

QString Rational::pretty_print()
{
    QString the_string;
    if(numerator > denominator)
        the_string = QString::number(float(numerator)/float(denominator));
    else
        the_string = "1/" + QString::number(float(denominator)/float(numerator));
    return the_string;
}

Photo::Photo()
{
}

//This should fetch us the preview and the metadata
void Photo::set_photo(unsigned int id)
{
    int fno = 5661;
    absolute_file_path = QString("/Users/kghose/Source/Sandbox/2011-10-16/DSC_")
            + QString::number(fno+id) + QString(".JPG");
    preview.load(absolute_file_path);
}

QUrl Photo::get_photo_url()
{
    return QUrl("file://" + absolute_file_path);
}
