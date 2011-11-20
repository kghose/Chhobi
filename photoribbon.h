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
  This contains definitions for the photo ribbon gadget.

  PhotoInfo is the simplest structure to carry enough information that will
  allow us to load the right photo from disk when a preview is requested.

  RibbonTile is an element of the ribbon. Carries PhotoInfo to emit when we
  hover over it so we can show the right preview.

  PhotoRibbon is the photo ribbon gadget.
*/
#ifndef PHOTORIBBON_H
#define PHOTORIBBON_H

#include <QtGui>

//Just a basic struct to carry some useful photo info
struct PhotoInfo
{
    unsigned int id;
    QString relative_file_path;
    bool operator ==(PhotoInfo o_pi) {return o_pi.id == id ? true : false;}//needed for list 'contains'
};

enum RibbonTileState {NORMAL, PREVIEW};
class RibbonTile : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

    PhotoInfo pi;
    RibbonTileState state;

public:
    explicit RibbonTile(unsigned int tile_width);
    void set_pi(PhotoInfo c_pi) {pi.id=c_pi.id;pi.relative_file_path=c_pi.relative_file_path;}
    PhotoInfo get_id() {return pi;}
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
    void set_ids(QList<PhotoInfo> ids);
    void add_ids(QList<PhotoInfo> ids);
    QList<PhotoInfo> get_all_ids();
    QList<PhotoInfo> get_selected_ids();


signals:
    void preview_id(PhotoInfo);//Emitted whenever we choose a tile

public slots:
    void set_preview_tile(RibbonTile *);
};

#endif // PHOTORIBBON_H
