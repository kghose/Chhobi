Description
===========
Chhobi is a photo *organizer* program. It does not have facilites to do any
editing of the image data itself (so no red-eye removal, contrast enhancement
etc. etc.). It simply allows you to change captions and keywords on photos.
The captions and keywords are stored in the EXIV metadata, so they are in a
standard format, stored in the image file so they travel with the image
(when you copy/move/backup them) and can be read by other programs, including
some folder managers (e.g. Finder on Mac) and not locked up in some complex,
properitary format that will vanish in a decade or so.

My main aim in designing and writing Chhobi was to keep the interface (and code)
as simple and uncluttered as possible, and do one thing only, which is to browse
and organize large libraries of pictures.

Usage
=====
The main browsing interface is the Ribbon, which represents each picture as a
small tile. The tiles are arranged in order from latest to oldest. Hovering the
mouse over the tile causes a preview and the photo metadata to appear.

Clicking on the tile will "lock" the preview and allow you to move your mouse
without changing the preview. The caption, date and keywords can be edited. You
can use the left and right mouse buttons to step through the pictures. Pressing
`h` will cause the photo to be added to the holding table. You can click and
drag the mouse to select multiple photos.

Click on the tabs to switch between the `show` and `hold` tables. Clicking
`resize` will cause the held photos to be resized and copied into a temporary
folder. When the progress bar completes and the `mail` icon becomes enabled
you can drag and drop the `mail` icon:

1. Into an email program
1. Into a `browse for file` field of a webpage
1. Into Picasa's add photos field

to add the resized photos.You can also open the folder
enclosing the resized photos by clicking on the folder button. Note that the
original folder structure of the images is maintained, so all the photos are
not copied into a single folder.

License (GPL)
=============
> Chhobi is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
>
> Chhobi is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
>
> You should have received a copy of the GNU General Public License along with
Chhobi; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA`

Some design choices
===================
Most design choices have been made that make the user experience less "slick"
but make the code more straightforward and less complicated and thus less
prone to bugs (hopefully).

Changes
-------
When we change photos (change the caption/keywords/date) they are not immediately
propagated (mirrored) to the database. Instead, on the next crawl they are
detected as changed photos and reimported into the database.
**This means that changes are visible immediately (since the metadata is
loaded directly from the photo) but any searches/ordering will reflect the old
data until a "crawl" is completed.**

This simplifies the code, speeds up the operations and is of minimal annoyance
in day to day use.

Keywords
--------
* No longer stored in an association table, kept in a simple text field
* When keywords are deleted or an image is purged the keyword list is not updated.
This leads to "empty" keywords i.e. keywords which will not fetch any results.

Deployment
==========
For mac:
macdeployqt-4.6 Chhobi.app  -dmg


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

* The importing function is a recursive function that starts at the photo_root
and works its way down the tree.

* At each directory it pulls up the current directory entry from the database. If
the directory is not in the database then all child files are imported.

* If the directory has been imported, if the last modified date is later than the
last descent date the list of files under this directory are pulled up from the
database. Each file on disk is checked against the database contents. If the
file exists and its last modified date is after the last descent date it is
reimported. If the file does not exist, it is imported. If there is a file in
the database and not on disk, it is purged.

* If a directory is unmodified none of the enclosed files are read.

* Finally, all the child directories are listed and sequentially recursed into.

* After we are done, we find out the zombie directories (present in db, absent on
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
From discussions on [stack overflow][1] we have:

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
1. [DONE] Photo import (not detecting new folders)
1. [DONE] Directory filters to prevent loading of .meta files
1. [DONE] Preview image portrait orientation proper size. -> Turned out to be
interesting because the photo could need to be rotated AND the preview frame
could be portrait or landscape sized.


TODO
====
1. Change preview from QLabel to scrollview so we can zoom the image, have
   multiple images and have a border round the image
1. Selecting multiple photos should display the photos arranged in a grid on the
   preview pane, and we should be able to modify captions and keywords for the
   collection together (give them common keywords - like old Chhobi).
1. Hint for keywords
1. Phonon to view videos
1. Clicking on preview should open it in external application
1. [NOT DOING] Put a nice border round the preview (need to subclass something)
1. [DONE] - now wrap, just stop When we step to the end or begining of the list we should wrap around
1. [DONE] When we delete an item we should rearrange holding table items.
1. [DONE] Implement searching by caption
1. [DONE] Implement sub-selection by keywords
1. [DONE] Temporary directory should be unique to current run of resizing
1. [DONE] Refactor database.h/.cpp to not use classes, but just functions
1. [DONE] Improve directory crawling
1. [DONE] Put directory crawling in separate thread so that Chhobi is functional during crawl
1. [DONE] Put absolute filename in status bar
1. [DONE] Last modified for directories working correctly i.e. are changed files retrawled?
1. [DONE] SQL queries bind values
1. [DONE] Holding table mailing + copy to separate directory + resize as needed
1. [DONE] Step through pictures
1. [DONE] Keyword adn caption storing in database
1. [DONE] Change to a dense panel ("lighttable") like arrangement? Need to switch to
          click to select and double click to hold model
1. [DONE] Refactor code to merge PhotoInfo and PhotoMetaData
1. [DONE] Add separators every x photos with some kind of date identification to help
          us locate things faster.
1. [DONE] Remove images from holding table
