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

#include "photoribbon.h"

RibbonTile::RibbonTile(unsigned int tile_width) : QGraphicsRectItem()
{
    state = NORMAL;
    setRect(0,0,.95*tile_width,.95*tile_width);//Dashes are better usability
    setFlag(QGraphicsItem::ItemIsSelectable);//Vital for rubberband selection
    setAcceptHoverEvents(true);//This is how we preview
}

void RibbonTile::paint(QPainter *painter,
           const QStyleOptionGraphicsItem *option,
           QWidget *widget)
{
    if(isSelected())
        setPen(QPen(Qt::white));
    else if(state==PREVIEW)
        setPen(QPen(Qt::yellow));
    else
        setPen(QPen(Qt::black));
    //we'll do our own painting of the selection indicator thank you.
    QStyleOptionGraphicsItem myoption = (*option);
    myoption.state &= !QStyle::State_Selected;
    QGraphicsRectItem::paint(painter, &myoption, widget);
}

void RibbonTile::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
    emit preview(this);
}

PhotoRibbon::PhotoRibbon(QObject *parent) :
    QGraphicsScene(parent)
{
    tile_size = 10;
    columns = 16; //do this from measurement of view port? or set this?
    preview_tile = NULL;
    setBackgroundBrush(Qt::black);
}

//Replace the current list with this one
void PhotoRibbon::replace_tiles(QList<PhotoInfo> new_tiles)
{
    clear();//memory leak or not?
    add_tiles(new_tiles);
}

//Add stuff to the current list (only if they are not already present)
void PhotoRibbon::append_tiles(QList<PhotoInfo> new_tiles)
{
    QList<PhotoInfo> old_tiles = get_all_tiles();
    add_tiles(new_tiles, old_tiles);
}

void PhotoRibbon::add_tiles(QList<PhotoInfo> new_tiles,
                            QList<PhotoInfo> old_tiles)
{
    QList<PhotoInfo>::iterator i;
    int x = 0, y = 0, col = 0;
    if(!old_tiles.isEmpty()) {
        col = old_tiles.count()%columns;
        x = col*tile_size;
        y = (old_tiles.count()/columns)*tile_size;
    }

    for (i = new_tiles.begin(); i != new_tiles.end(); ++i) {
        if(old_tiles.contains(*i))
            continue;
        RibbonTile *rt = new RibbonTile(tile_size);
        rt->set_pi(*i);
        rt->setPos(x,y);
        int r = ((*i).tile_color >> 16) & 0xff,
            g = ((*i).tile_color >> 8) & 0xff,
            b = ((*i).tile_color) & 0xff;
        rt->setBrush(QBrush(QColor(r,g,b)));
        addItem((QGraphicsItem *)rt);
        QObject::connect(rt, SIGNAL(preview(RibbonTile *)),
                this, SLOT(set_preview_tile(RibbonTile *)));

        x += tile_size;
        col += 1;
        if(col == columns) {
            x = 0;
            y += tile_size;
            col = 0;
        }
    }
    setSceneRect(-5, -5, columns*tile_size+20, y+20);
}

void PhotoRibbon::set_preview_tile(RibbonTile *prt)
{
    if(preview_tile != NULL)
        preview_tile->unset_preview();
    preview_tile = prt;
    preview_tile->set_preview();
    emit preview_id(preview_tile->get_id());
}

QList<PhotoInfo> PhotoRibbon::get_all_tiles()
{
    QList<QGraphicsItem *> all_tiles = items();
    QList<QGraphicsItem *>::iterator i;
    QList<PhotoInfo> all_ids;
    for (i = all_tiles.begin(); i != all_tiles.end(); ++i) {
        all_ids.append(((RibbonTile*) *i)->get_id());
    }
    return all_ids;
}


QList<PhotoInfo> PhotoRibbon::get_selected_tiles()
{
    QList<QGraphicsItem *> selected_tiles = selectedItems();
    QList<QGraphicsItem *>::iterator i;
    QList<PhotoInfo> selected_ids;
    for (i = selected_tiles.begin(); i != selected_tiles.end(); ++i) {
        selected_ids.append(((RibbonTile*) *i)->get_id());
    }
    return selected_ids;
}
