Description
===========
Chhobi is a photo /organizer/ program. It does not have facilites to do any
editing of the image data itself (so no red-eye removal, contrast enhancement
etc. etc.)

It simply allows you to change captions and keywords on photos. The captions and
keywords are stored in the EXIV metadata, so they travel with the image file and
can be read by other programs, including some folder managers e.g. Finder on Mac.

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
ways. The pictures in the set can be copied into a new folder (resized) if
asked for, and the copies can be dropped into a mail program or other external
application.


License (GPL)
=============
Chhobi is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Chhobi is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
Chhobi; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

Code layout
===========
main.cpp -

Just a boilerplate file that instantiates the UI and calls the main loop

mainwindow.cpp -

Instantiates the UI (mainwindow.ui)

Coding notes
============
Tracking file system changes
--------------------------
Changing a file causes the modified date on the parent directory to change, but
not in the grandparents etc. For this reason we have to recurse down every
directory, but we save time by only going through files in a directory which
has changed after our latest trawl.


Class inheritance
-----------------
`class RibbonTile : public QObject, public QGraphicsRectItem` - we have to use the
qualifier `public` before each parent class name, otherwise all members of the
parent class become private (i.e. inaccessible from the inheriting class)

Lens information Exiv2
----------------------
To print the lens information using the helper functions from easyaccess.hpp
do:
`if(Exiv2::lensName(exifData) != exifData.end())
    pmd.lens_model = Exiv2::lensName(exifData)->print(&exifData).c_str();`
If print is not passed the whole exifdata, then it just prints the id code,
which is just a number. We need the `exifData.end()` test because this function
can return us a not found which is the same as end

QFileDialog glitches
--------------------
`QFileDialog::getExistingDirectory` : If we use the native file dialog we
run into strange glitches - should mention this on the Qt mailing list

QSettings
---------
QSettings when loading a QDateTime entry that does not exists returns an
invalid QDateTime, as expected

QDateTime comparisons
---------------------
A valid QDateTime >= an invalid QDateTime

How to get file creation time on Mac OS X (64 bit)
--------------------------------------------------
* QFileInfo.created() on POSIX systems (like Mac OS X) returns the last modified time [1].
* This is annoying because on Mac OS X you know the system (e.g. finder) has access to the actual creation time.
* Turns out that BSD systems (like Mac OS X) have an extension to sys/stat.h that contains the file creation time [2].

[1]: http://doc.qt.nokia.com/latest/qfileinfo.html#created
[2]: http://developer.apple.com/library/IOS/#documentation/System/Conceptual/ManPages_iPhoneOS/man2/stat.2.html

The recipe, then is (shown using the QT framework)

    #include <QtCore>
    #include <sys/stat.h>

    int main(int argc, char *argv[])
    {
        QString fname("/Users/kghose/Sandbox/ChhobiTest/2005/2005_07_10/MVI_0693.AVI");
        QDateTime thedatetime;
        struct stat64 the_time;
        stat64(fname.toStdString().c_str(), &the_time);
        thedatetime.setTime_t(the_time.st_birthtimespec.tv_sec);
        qDebug() << thedatetime;
    }

TODO
====
1. Put absolute filename in status bar
1. Last modified for directories working correctly i.e. are changed files retrawled?
1. Implement searching by caption
1. Implement sub-selection by keywords
1. Change to a dense panel ("lighttable") like arrangement? Need to switch to
click to select and double click to hold model
