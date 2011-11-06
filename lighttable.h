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

/*
 This class inherits QGraphicsScene and is responsible for taking thumbnails and
 arranging them nicely on the scene. Thumbnails can be added or deleted
 Lighttable will rearrange them as needed. Thumbnails can be selected/unselected
 and Lighttable will keep track of that for mass actions.
 */

#ifndef LIGHTTABLE_H_
#define LIGHTTABLE_H_

#include <QtGui>

/*
 This class inherits QGraphicsScene and is responsible for taking thumbnails and
 arranging them nicely on the scene. Thumbnails can be added or deleted
 Lighttable will rearrange them as needed. Thumbnails can be selected/unselected
 and Lighttable will keep track of that for mass actions.
 */
enum LT_MODE {GRID, ROW};

class LightTable : public QGraphicsScene {
Q_OBJECT

    LT_MODE mode; //how do we arrange the thumbnails
    unsigned int grid_width; //Ignored if mode is ROW
    unsigned int table_x_margin, table_y_margin;//margins look nice
    unsigned int thumbnail_size;//Max dimension of whole thumbnail widget

public:
    LightTable(QObject *parent = 0);

public slots:
    void set_scene_width(unsigned int width);
    void set_thumbnail_size(unsigned int width);

    //Add a thumbnail to the scene. Place it in the apropriate position (at the
    //end of the grid
    void add(QGraphicsItem *item);

    //Remove thumbnail. Make sure the other thumbnails are rearranged accordingly
    void remove(QGraphicsItem *item);

    //Empty the table
    void reset();

    //Call when we resize our view, change thumbnail size or do anything else
    //that changes the size of the grid
    void rearrange();
};

#endif /*LIGHTTABLE_H_*/
