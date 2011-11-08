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
small tile. Hovering the mouse over the time causes the picture and it's
associated metadata to appear in the preview pane. The caption and keywords can
be edited in the preview pane. Clicking on a tile, or selecting a bunch of tiles
adds them to the holding table. The tiles represent a set of pictures that can
be processed in different ways. The set can be added to a new or existing
collection or the pictures in the set can be copied into a new folder (resized)
if asked for, and the copies can be dropped into a mail program or other
application.


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


Design notes
============

Ribbon design:

The ribbon is a QGraphicsScene that plots a small square for each picture in the
database. The color of the square is taken from the dominant color in the
photo. The squares are clustered into groups by day, month and then year.

Hovering the mouse over a square displays the image "thumbnail" and caption and
various other metadata in the inspection panel. The "selected" image only
changes when we hover into another image.

Each time we hover over an image a show request is sent. The show request is
held until the next show request comes and overwrites it. The actual image is
only fetched when the last fetch is completed. i.e. there is no buffering, so
we don;t have the case where we quickly passed over a bunch of images and now
we have this long wait while each image cycles through in turn in the inspector
panel.

Clicking on the square transfers that image into the holding table. Hovering
over the holding table likewise shows the image in the inspector panel as well.

Clicking on the "action" button on the holding table takes us to the "action"
panel where we can choose to generate resized images, copy/paste them or do
other things with the images.


Code layout
===========
main.cpp -

Just a boilerplate file that instantiates the UI and calls the main loop
mainwindow -

Instantiates the UI (mainwindow.ui) and does general coordinating

lighttable -

Inherits QGraphicsScene. Thumbnails (as QGraphicsItems) can be added or removed.
lighttable takes care of arranging the thumbnails in a grid when thumbnails are
added, when the QGraphicsView is resized or when thumbnails are removed. Takes
care of the usual selection and scrolling etc.

thumbnail -

Inherits QGraphicsItem. Displays the thumbnail and carries the photo metadata
information that can be displayed on hovering and edited on clicking.





Coding notes
============



