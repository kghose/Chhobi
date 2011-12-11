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
#include "photo.h"

//Just a basic struct to carry some useful photo info
struct PhotoInfo : PhotoMetaData
{
    unsigned int id, tile_color;
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
    void set_pi(PhotoInfo c_pi);
    PhotoInfo get_id() {return pi;}
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget);
    void set_preview();
    void unset_preview();

private:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *);
    QVariant itemChange(GraphicsItemChange, const QVariant &);

signals:
    void preview(RibbonTile *);
};

class PhotoRibbon : public QGraphicsScene
{
    Q_OBJECT

    int tile_size, columns;
    RibbonTile *preview_tile;//This is the tile on preview
    QGraphicsSimpleTextItem *the_date;//The date we show when we scroll
    bool preview_locked;//Mech to prevent preview changeing when we move mouse
    bool holding_table;//True means the contents change (can be deleted, added)

public:
    explicit PhotoRibbon(QObject *parent = 0, bool hd = false);
    void replace_tiles(QList<PhotoInfo>);
    void append_tiles(QList<PhotoInfo>);
    QList<PhotoInfo> get_all_tiles();
    QList<PhotoInfo> get_selected_tiles();

signals:
    void preview_id(PhotoInfo);//Emitted whenever we choose a tile
    void hold();//Emitted whenever we want to hold the pictures

public slots:
    void select_first_tile();
    void set_preview_tile(RibbonTile *);
    void toggle_preview_tile_lock();
    void slider_value_changed(int);//we hook up the slider to these to display
    void slider_released();//(and hide) the date of the photos passing by

private:
    void create_date_item();//We create and add the date thingy on demand
    void add_tiles(QList<PhotoInfo> new_tiles,
                   QList<PhotoInfo> old_tiles = QList<PhotoInfo>()); //convenience function
    void keyPressEvent(QKeyEvent *keyEvent);
    void select_adjacent_tile(bool backward);
    void delete_selected();//Only active for the holding table
    void reorder_tiles();//only needed after deleting
};

#endif // PHOTORIBBON_H
