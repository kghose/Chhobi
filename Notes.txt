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


