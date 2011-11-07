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

#ifndef PHOTORIBBON_H
#define PHOTORIBBON_H

#include <QtGui>

enum RibbonTileState {NORMAL, PREVIEW};
class RibbonTile : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

    unsigned int id;
    RibbonTileState state;

public:
    explicit RibbonTile(unsigned int tile_width);
    void set_id(unsigned int c_id) {id=c_id;}
    unsigned int get_id() {return id;}
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget);
    void set_preview() {state=PREVIEW;}
    void unset_preview() {state=NORMAL;}

private:
    void hoverEnterEvent(QGraphicsSceneHoverEvent * event);

signals:
    void preview(RibbonTile *);
};

class PhotoRibbon : public QGraphicsScene
{
    Q_OBJECT

    unsigned int item_count, tile_width;
    RibbonTile *preview_tile;

public:
    explicit PhotoRibbon(QObject *parent = 0);
    void set_ids(QList<unsigned int> ids);

signals:
    void preview_id(unsigned int);//Emitted whenever we choose a tile

public slots:
    void set_preview_tile(RibbonTile *);
};

#endif // PHOTORIBBON_H
