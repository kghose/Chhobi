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
        setPen(QPen(Qt::blue));
    else
        setPen(QPen(Qt::gray));
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
void PhotoRibbon::set_ids(QList<PhotoInfo> ids)
{
    clear();//memory leak or not?
    QList<PhotoInfo>::iterator i;
    int x = 0, y = 0, col = 0;
    for (i = ids.begin(); i != ids.end(); ++i) {
        RibbonTile *rt = new RibbonTile(tile_size);
        rt->set_pi(*i);
        rt->setPos(x,y);
        rt->setBrush(QBrush(QColor(qrand()%256,qrand()%256,qrand()%256)));
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

//Add stuff to the current list (only if they are not already present)
void PhotoRibbon::add_ids(QList<PhotoInfo> new_ids)
{
    QList<PhotoInfo> old_ids = get_all_ids();
    QList<PhotoInfo>::iterator i_old;
    QList<PhotoInfo>::iterator i_new;


    int col = old_ids.count()%columns,
        x = col*tile_size,
        y = (old_ids.count()/columns)*tile_size;

    for (i_new = new_ids.begin(); i_new != new_ids.end(); ++i_new) {
        if(!old_ids.contains(*i_new)){
            RibbonTile *rt = new RibbonTile(tile_size);
            rt->set_pi(*i_new);
            rt->setPos(x,y);
            addItem((QGraphicsItem *)rt);
            QObject::connect(rt, SIGNAL(preview(RibbonTile *)),
                    this, SLOT(set_preview_tile(RibbonTile *)));
            x += tile_size;
            col += 1;
            if(col == columns) {
                x = 0;
                y += tile_size;
                col=0;
            }
        }
    }
    setSceneRect(-10, 0, columns*tile_size+20,y+20);
}


void PhotoRibbon::set_preview_tile(RibbonTile *prt)
{
    if(preview_tile != NULL)
        preview_tile->unset_preview();
    preview_tile = prt;
    preview_tile->set_preview();
    emit preview_id(preview_tile->get_id());
}

QList<PhotoInfo> PhotoRibbon::get_all_ids()
{
    QList<QGraphicsItem *> all_tiles = items();
    QList<QGraphicsItem *>::iterator i;
    QList<PhotoInfo> all_ids;
    for (i = all_tiles.begin(); i != all_tiles.end(); ++i) {
        all_ids.append(((RibbonTile*) *i)->get_id());
    }
    return all_ids;
}


QList<PhotoInfo> PhotoRibbon::get_selected_ids()
{
    QList<QGraphicsItem *> selected_tiles = selectedItems();
    QList<QGraphicsItem *>::iterator i;
    QList<PhotoInfo> selected_ids;
    for (i = selected_tiles.begin(); i != selected_tiles.end(); ++i) {
        selected_ids.append(((RibbonTile*) *i)->get_id());
    }
    return selected_ids;
}
