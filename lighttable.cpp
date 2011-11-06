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
 This class inherits QGraphicsScene and is responsible for taking thumbnails and
 arranging them nicely on the scene. Thumbnails can be added or deleted
 Lighttable will rearrange them as needed. Thumbnails can be selected/unselected
 and Lighttable will keep track of that for mass actions.
 */

#include "lighttable.h"

LightTable::LightTable(QObject *parent) : QGraphicsScene(parent)
{
    //Some defaults to tide us by
    mode = GRID;
    table_x_margin = 10;
    table_y_margin = 10;
    thumbnail_size = 100;
    setBackgroundBrush(Qt::black);
}

/*
 * slot - Use this when graphics view changes. Recompute the grid and rearrange
 * the thumbnails.
 */
void LightTable::set_scene_width(unsigned int width)
{
    grid_width = width - 2*table_x_margin;
    rearrange();
}

/*
 * slot - Use this when we change the thumbnail size. Recompute the grid and
 * rearrange the thumbnails.
 */
void LightTable::set_thumbnail_size(unsigned int width)
{
    thumbnail_size = width;
    rearrange();
}

/*
 * slot - Add a thumbnail to the scene. Place it at the end of the grid and
 * update the sceneRect as needed
 */
void LightTable::add(QGraphicsItem *item)
{
    QList<QGraphicsItem *> thumbnails = items();
    int x = 0, y = 0, n_items = thumbnails.count();
    for (int i = 0; i < n_items; i++) {
        x += thumbnail_size;
        if((x>grid_width) && (mode==GRID)) {
            x = 0;
            y += thumbnail_size;
        }
    }
    item->setPos(x,y);
    addItem(item);
    setSceneRect(0, 0, grid_width, y+2*table_y_margin);
}

/*
 * slot - Remove thumbnail from the scene. Rearrange thumbnails and update
 * the sceneRect as needed
 */
void LightTable::remove(QGraphicsItem *item)
{
    removeItem(item);
    rearrange();
}

/*
 * slot - Removes all thumbnailitems from the scene, releasing them to whoever
 * is in charge
 */
void LightTable::reset()
{
    QList<QGraphicsItem *> thumbnails = items();
    QList<QGraphicsItem *>::iterator i;
    for (i = thumbnails.begin(); i != thumbnails.end(); ++i) {
        removeItem(*i);
    }
    setSceneRect(0, 0, 0, 0);
}

/*
 * slot - Called whenever the scene width changes, or a thumbnail is deleted or
 * thumbnail size is changed. This rearranges the items on the scene and sets
 * the scene rect
 */
void LightTable::rearrange()
{
    grid_width = views()[0]->width() - 2*table_x_margin;
    qDebug() << grid_width;
    QList<QGraphicsItem *> thumbnails = items();
    QList<QGraphicsItem *>::iterator i;
    int x = table_x_margin, y = table_y_margin;
    for (i = thumbnails.begin(); i != thumbnails.end(); ++i) {
        (*i)->setPos(x,y);
        x += thumbnail_size;
        if((x>grid_width - thumbnail_size) && (mode==GRID)) {
            x = table_x_margin;
            y += thumbnail_size;
        }
    }
    setSceneRect(0, 0, grid_width, y+2*table_y_margin);
}
