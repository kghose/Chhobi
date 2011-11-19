Description
===========
Chhobi is a photo /organizer/ program. It does not have facilites to do any
editing of the image data itself (so no red-eye removal, contrast enhancement
etc. etc.)

It simply allows you to change captions and keywords on photos and to organize
them into events (albums) and collections. The captions and keywords are stored
in the EXIV metadata, so they travel with the image file and can be read by
other programs, including some folder managers e.g. Finder on Mac. The
events and collections are stored in a fairly simple sqlite database and so
have the potential to be easily imported/converted to be used by other
programs.

My main aim in designing and writing Chhobi was that the organization should be
kept in a simple easily convertible format so that it can be carried forward
over the years, and not locked up in some complex, properitary format that will
vanish in a decade or so.

A secondary aim was to make it as simple as possible, and do one thing only,
which is to browse and organize large libraries of pictures.

Usage
=====
The main browsing interface is the Ribbon, which represents each picture as a
small tile. The tiles are arranged in order from latest to oldest, from left to
right. Hovering the mouse over the tile causes the picture and it's
associated metadata to appear in the preview pane.

The caption, date and keywords can be edited in the preview pane. Clicking on a
tile, or selecting a bunch of tiles adds them to the holding table. The tiles on
the holding table represent a set of pictures that can be processed in different
ways. The set can be added to a new or existing collection or the pictures in
the set can be copied into a new folder (resized) if asked for, and the copies
can be dropped into a mail program or other external application.


License (GPL)
=============
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

Code layout
===========
main.cpp -

Just a boilerplate file that instantiates the UI and calls the main loop
mainwindow -

Instantiates the UI (mainwindow.ui) and does general coordinating

Coding notes
============
1. Class inheritance:
`class RibbonTile : public QObject, public QGraphicsRectItem` - we have to use the
qualifier `public` before each parent class name, otherwise all members of the
parent class become private (i.e. inaccessible from the inheriting class)

1. To print the lens information using the helper functions from easyaccess.hpp
do:

`lens_model = exifData[Exiv2::lensName(exifData)->key()].print(&exifData).c_str()`

where exifData is an instance of `Exiv2::ExifData`

If print is not passed the whole exifdata, then it just prints the id code,
which is just a number.

1. `QFileDialog::getExistingDirectory` : If we use the native file dialog we
run into strange glitches - should mention this on the Qt mailing list




