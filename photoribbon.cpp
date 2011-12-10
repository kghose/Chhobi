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

/*
 We moved the setPen commands (to draw thw ribbon tile outlines differently for
 different item states) to the next three functions - set_preview, unset_preview
 and itemChange - because putting the setPen in the paint function led to an
 infinite recursion.
*/
void RibbonTile::set_preview()
{
    state=PREVIEW;
    setPen(QPen(Qt::yellow));
}

void RibbonTile::unset_preview()
{
    state=NORMAL;
    if(isSelected())
        setPen(QPen(Qt::white));
    else
        setPen(QPen(Qt::black));
}

QVariant RibbonTile::itemChange(GraphicsItemChange change, const QVariant &value)
 {
     if (change == QGraphicsItem::ItemSelectedHasChanged) {
         if(value.toBool())
             setPen(QPen(Qt::white));
         else {
            if(state==PREVIEW)
                setPen(QPen(Qt::yellow));
            else
                setPen(QPen(Qt::black));
         }
     }
     return QGraphicsItem::itemChange(change, value);
 }

void RibbonTile::paint(QPainter *painter,
           const QStyleOptionGraphicsItem *option,
           QWidget *widget)
{
    //we'll do our own painting of the selection indicator thank you.
    QStyleOptionGraphicsItem myoption = (*option);
    myoption.state &= !QStyle::State_Selected;
    QGraphicsRectItem::paint(painter, &myoption, widget);
}

void RibbonTile::hoverEnterEvent(QGraphicsSceneHoverEvent * /*event*/)
{
    emit preview(this);
}

void RibbonTile::set_pi(PhotoInfo c_pi)
{
    pi.id=c_pi.id;
    pi.relative_file_path=c_pi.relative_file_path;
    pi.photo_date=c_pi.photo_date;
    pi.tile_color=c_pi.tile_color;
    pi.type=c_pi.type;
}

PhotoRibbon::PhotoRibbon(QObject *parent, bool is_holding_table) :
    QGraphicsScene(parent)
{
    holding_table = is_holding_table;//if true then we are allowed to delete items
    tile_size = 10;
    columns = 16; //do this from measurement of view port? or set this?
    preview_tile = NULL; //We check if this is NULL as a way of telling if it has been initialized
    preview_locked = false;
    setBackgroundBrush(Qt::black);
    QObject::connect(this, SIGNAL(selectionChanged()),
            this, SLOT(toggle_preview_tile_lock()));
    create_date_item(); //The date text we hover while scrolling
}

//Replace the current list with this one
void PhotoRibbon::replace_tiles(QList<PhotoInfo> new_tiles)
{
    clearSelection();//Some clean up we need
    preview_tile = NULL;
    if(the_date->scene() == this)
        removeItem(the_date);//clear() will remove AND delete the item
                             //by removing it, we preserve it for later
    clear();
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
    int current_row = 0;
    QList<PhotoInfo>::iterator i;
    int x = 0, y = 0, col = 0;
    if(!old_tiles.isEmpty()) {
        col = old_tiles.count()%columns;
        x = col*tile_size;
        current_row = old_tiles.count()/columns;
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
            current_row++;
            col = 0;
            y += tile_size;
        }
    }
    setSceneRect(-5, -5, columns*tile_size+5, y+20);
}

//Call this when the selection changes, since that is when we want to keep the
//previewed photo constant and not changing as we move the mouse
void PhotoRibbon::toggle_preview_tile_lock()
{
    if(selectedItems().count() == 0) {//We wanna unlock things
        preview_locked = false;
        if(preview_tile != NULL)
            preview_tile->unset_preview();
    } else {//We might wanna change the selected tile
        if(selectedItems().count() == 1) {//Yes, we really do
            preview_locked = false;
            set_preview_tile((RibbonTile *)selectedItems()[0]);
            preview_locked = true;
        }
    }
}

void PhotoRibbon::set_preview_tile(RibbonTile *prt)
{
    if(preview_locked==true) return;
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

//Need to reimplement this to check if we wanna hold the photos
void PhotoRibbon::keyPressEvent(QKeyEvent *keyEvent)
{
    if(keyEvent->key() == Qt::Key_H)
        emit hold();
    else if(keyEvent->key() == Qt::Key_Escape)
        clearSelection();
    else if(keyEvent->key() == Qt::Key_Right)
        select_adjacent_tile(false);
    else if(keyEvent->key() == Qt::Key_Left)
        select_adjacent_tile(true);
    else if(keyEvent->key() == Qt::Key_Backspace)
        delete_selected();

    QGraphicsScene::keyPressEvent(keyEvent);
}

//Move selection around by the keyboard
void PhotoRibbon::select_adjacent_tile(bool backward)
{
    QList<QGraphicsItem *> selected_tiles = selectedItems();
    if(selected_tiles.count()==0) return;
    QPointF pt = selected_tiles[0]->pos();
    QPainterPath sel_path = selected_tiles[0]->shape().translated(pt),
                 old_sel_path = sel_path;
    qreal x, y;
    if(!backward) {//forward
        if(pt.x() >= (columns-1)*tile_size) {
            x = -pt.x();
            y = tile_size;
        } else {
            x = tile_size;
            y = 0;
        }
    } else {//backward
        if(pt.x() < tile_size) {
            x = (columns-1)*tile_size;
            y = -tile_size;
        } else {
            x = -tile_size;
            y = 0;
        }
    }
    setSelectionArea(sel_path.translated(x,y));
    //Check to see if we are off the reservation
    selected_tiles = selectedItems();
    if(selected_tiles.count()==0) //Yep, off the reservation
        setSelectionArea(old_sel_path);
}

void PhotoRibbon::delete_selected()
{
    if(!holding_table) return; //We only delete for holding table
    QList<QGraphicsItem *> selected_tiles = selectedItems();
    if(selected_tiles.count()==0) return;
    QPointF pt = selected_tiles[0]->pos();//Would like to return selection to this pos
    QPainterPath sel_path = selected_tiles[0]->shape().translated(pt);
    QList<QGraphicsItem *>::iterator i;
    for (i = selected_tiles.begin(); i != selected_tiles.end(); ++i) {
        removeItem(*i);
        //delete *i;//presumably this will avoid leaks
    }
    reorder_tiles();
    setSelectionArea(sel_path);//try and set selection back to this pt
}

void PhotoRibbon::reorder_tiles()
{
    int current_row = 0;
    int x = 0, y = 0, col = 0;
    QList<QGraphicsItem *> tiles = items(Qt::AscendingOrder);
    QList<QGraphicsItem *>::iterator i;
    for (i = tiles.begin(); i != tiles.end(); ++i) {
        (*i)->setPos(x,y);
        x += tile_size;
        col += 1;
        if(col == columns) {
            x = 0;
            current_row++;
            col = 0;
            y += tile_size;
        }
    }
    setSceneRect(-5, -5, columns*tile_size+5, y+20);
}

void PhotoRibbon::slider_value_changed(int val)
{
    qreal w = tile_size, h = tile_size;
    the_date->setVisible(false);//Don't want the date thingy to be counted
    if(the_date->scene() != this)//eg, we've done a clear()
        addItem(the_date);
    QList<QGraphicsItem *> tiles = items(0,val,w,h);
    if(tiles.count() > 0) {
        QString date_str = ((RibbonTile*)tiles[0])->get_id().photo_date.toString("dd\nMMM\nyyyy");
        int half_height = views()[0]->height()/2.0;
        the_date->setText(date_str);
        the_date->setPos(0, val + half_height - the_date->boundingRect().height()/2.0);
        the_date->setVisible(true);//Now we can see it
    }
}

//Remove the date we show
void PhotoRibbon::slider_released()
{
    if(the_date->scene() == this)
        the_date->setVisible(false);
}

void PhotoRibbon::create_date_item()
{
    QSettings settings;
    if(!settings.contains("hover date/font size"))
        settings.setValue("hover date/font size", 50);
    if(!settings.contains("hover date/font outline"))
        settings.setValue("hover date/font outline", 1);
    if(!settings.contains("hover date/font family"))
        settings.setValue("hover date/font family", QString("Helvetica"));
    QFont the_font(settings.value("hover date/font family").toString(),
                   settings.value("hover date/font size").toInt(),QFont::Bold);
    the_font.setStyleStrategy(QFont::PreferAntialias);

    //We show the date using this
    the_date = new QGraphicsSimpleTextItem("ABCD");
    the_date->setBrush(QBrush(Qt::white));
    int fnt_outline = settings.value("hover date/font outline").toInt();
    if(fnt_outline)
        the_date->setPen(QPen(QBrush(Qt::black), fnt_outline));
    the_date->setFont(the_font);
}
