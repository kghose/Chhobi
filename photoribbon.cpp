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
    setRect(0,0,.85*tile_width,.85*tile_width);//Dashes are better usability
    setFlag(QGraphicsItem::ItemIsSelectable);//Vital for rubberband selection
    setAcceptHoverEvents(true);//This is how we preview
}

void RibbonTile::paint(QPainter *painter,
           const QStyleOptionGraphicsItem *option,
           QWidget *widget)
{
    if(isSelected())
        setBrush(Qt::white);
    else if(state==PREVIEW)
        setBrush(Qt::blue);
    else
        setBrush(Qt::gray);
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
    tile_width = 15;
    preview_tile = NULL;
    setBackgroundBrush(Qt::black);
}

//Replace the current list with this one
void PhotoRibbon::set_ids(QList<unsigned int> ids)
{
    clear();//memory leak or not?
    QList<unsigned int>::iterator i;
    int x = 0;
    for (i = ids.begin(); i != ids.end(); ++i) {
        RibbonTile *rt = new RibbonTile(tile_width);
        rt->set_id(*i);
        rt->setPos(x,0);
        addItem((QGraphicsItem *)rt);
        QObject::connect(rt, SIGNAL(preview(RibbonTile *)),
                this, SLOT(set_preview_tile(RibbonTile *)));

        x += tile_width;
    }
    setSceneRect(-10, 0, x+20,20);
}

//Add stuff to the current list (only if they are not already present)
void PhotoRibbon::add_ids(QList<unsigned int> new_ids)
{
    QList<unsigned int> old_ids = get_all_ids();
    QList<unsigned int>::iterator i_old;
    QList<unsigned int>::iterator i_new;

    int x = tile_width * old_ids.count();
    for (i_new = new_ids.begin(); i_new != new_ids.end(); ++i_new) {
        if(!old_ids.contains(*i_new)){
            RibbonTile *rt = new RibbonTile(tile_width);
            rt->set_id(*i_new);
            rt->setPos(x,15);
            addItem((QGraphicsItem *)rt);
            QObject::connect(rt, SIGNAL(preview(RibbonTile *)),
                    this, SLOT(set_preview_tile(RibbonTile *)));
            x += tile_width;
        }
    }
    setSceneRect(-10, 0, x+20,20);
}

void PhotoRibbon::set_preview_tile(RibbonTile *prt)
{
    if(preview_tile != NULL)
        preview_tile->unset_preview();
    preview_tile = prt;
    preview_tile->set_preview();
    emit preview_id(preview_tile->get_id());
}

QList<unsigned int> PhotoRibbon::get_all_ids()
{
    QList<QGraphicsItem *> all_tiles = items();
    QList<QGraphicsItem *>::iterator i;
    QList<unsigned int> all_ids;
    for (i = all_tiles.begin(); i != all_tiles.end(); ++i) {
        all_ids.append(((RibbonTile*) *i)->get_id());
    }
    return all_ids;
}


QList<unsigned int> PhotoRibbon::get_selected_ids()
{
    QList<QGraphicsItem *> selected_tiles = selectedItems();
    QList<QGraphicsItem *>::iterator i;
    QList<unsigned int> selected_ids;
    for (i = selected_tiles.begin(); i != selected_tiles.end(); ++i) {
        selected_ids.append(((RibbonTile*) *i)->get_id());
    }
    return selected_ids;
}
