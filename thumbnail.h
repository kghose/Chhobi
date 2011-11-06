/*
 * This file is part of Chhobi - a photo organizer program.
 * Copyright (c) 2008 Kaushik Ghose kaushik.ghose@gmail.com
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

#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <QtGui>

enum ThumbnailCaptionState {HIDDEN, SHOWN, HIDING};
struct ThumbnailCaption
{
    QString caption;
    ThumbnailCaptionState state;
    int x_offset, y_offset, width, height;
    void paint(QPainter *painter);
    QRectF boundingRect() const;
};

class Thumbnail : public QGraphicsItem
{
    unsigned int tile_size, margin, thumbnail_size;
    QPixmap pm;
    ThumbnailCaption tc;

public:
    explicit Thumbnail();
    QRectF boundingRect() const;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget);

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);


signals:

public slots:
};

#endif // THUMBNAIL_H
