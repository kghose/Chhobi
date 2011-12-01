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

Some design choices
===================
Most design choices have been made that make the user experience less "slick"
but make the code more straightforward and less complicated and thus less
prone to bugs

Changes
-------
When we change photos (change the caption/keywords/date) they are not immediately
propagated (mirrored) to the database. Instead they are retrieved on the next
load (or can be manually forced). This means that changes are visible immediately
(since the metadata is loaded directly from the photo) but any searches/ordering
will reflect the old data until a refresh or restart is done.

This simplifies the code, speeds up the operations and is of minimal annoyance
in day to day use.

Keywords
--------
* No longer stored in an association table, kept in a simple text field
* When keywords are deleted or an image is purged the keyword list is not updated.
This leads to "empty" keywords i.e. keywords which will not fetch any results
Again, a force "refresh" will cause this to be updated.


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
Some observations:
1. Changing a file causes the modified date on the parent directory to change, but
not in the grandparents etc.
1. Adding a folder changes the last modified time on the parent folder
1. The child folder's modified time remains unchanged (e.g. if it was copied over).

This last point poses a particular problem. One elegant tracking algorithm could
have been to descend down each directory recursively only looking at directories
whose last changed time is less than

So, if we copy over a folder of photos from 2005 (the present being 2011) into
our current photo root the photo root's last modified time changes, but the last
modified time of the 2005 folder is still 2005 (or whatever). So any algorithm
that uses a global "last tracked" time will fail, since this last tracked time
will be greater than 2005.

Requirements:
1. Importing is a lengthy process so the import should be in a separate thread
1. The import should detect new and changed files/folders, skipping unchanged ones

Algorithm:
1. Start at the photo root and recurse into each child directory
1. If the directory's last modified time is after the last recurse time OR the
directory has not been imported before go through each file in the directory
1. For each file if the last modified time is after the last recurse time OR the
file has not been imported before


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

For QGraphicsItem `setPen` and `setBrush` trigger a `paint` event
-----------------------------------------------------------------
This was fun to figure out. I used to check for the state of the RibbonTile in
the `paint` event and use `setPen` to set the outline based on if the image was
under preview, or selected. Then I noticed that when I just had Chhobi open it
would be consuming 70% CPU just stitting there. I narrowed it down to a visible
QGraphicsView. Then, by using print statements I found that the `paint` event
was being called at regular intervals. I let that percolate in my mind for a bit
and then it came to me: I commented out all the `setPen` lines in `paint` for
RibbonTile and the problem went away.

### Solution
1. For the set/unset preview I just update the pen using the slots I made
1. For the selected/unselected I discovered `QGraphicsItem::itemChange` which
allows me to set the pen there.

`++i` OR `i++`?
---------------
From discussions on stack overflow [1][1] we have:
1. `++i` does not expect to return a value while `i++` might need to return a
value
2. A modern compiler, for simple data types (i.e. integer) will treat the two
the same if the context is right e.g. in a `for` loop
3. For an object (e.g. an interator), however, `++i` will be faster than `i++`
because a compiler cannot optimize away the creation of a temporary object for
`i++`

[1]:http://stackoverflow.com/questions/24886/is-there-a-performance-difference-between-i-and-i-in-c


BUGS TO FIX
===========
1. Photo import (not detecting new folders)
1. Directory filters to prevent loading of .meta files
1. [DONE] Preview image portrait orientation proper size. -> Turned out to be
interesting because the photo could need to be rotated AND the preview frame
could be portrait or landscape sized.


TODO
====
1. Improve directory crawling
1. Put directory crawling in separate thread so that Chhobi if functional during crawl
1. [DONE] Put absolute filename in status bar
1. [DONE] Last modified for directories working correctly i.e. are changed files retrawled?
1. [DONE] SQL queries bind values
1. [DONE] Holding table mailing + copy to separate directory + resize as needed
1. [DONE] Step through pictures
1. Phonon to view videos
1. [DONE] Keyword adn caption storing in database
1. Implement searching by caption
1. Implement sub-selection by keywords
1. [DONE] Change to a dense panel ("lighttable") like arrangement? Need to switch to
click to select and double click to hold model
1. [DONE] Refactor code to merge PhotoInfo and PhotoMetaData
1. [DONE] Add separators every x photos with some kind of date identification to help
us locate things faster.
1. Put a nice border round the preview (need to subclass something)
1. [DONE] Remove images from holding table
