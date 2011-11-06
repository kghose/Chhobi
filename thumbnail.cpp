#include "thumbnail.h"

void ThumbnailCaption::paint(QPainter *painter)
{
    if(state==SHOWN) {//We are currently showing the caption
        QRectF textbox = QRectF(x_offset, y_offset, width, height);
        painter->setBrush(Qt::yellow);
        painter->drawRoundedRect(x_offset, y_offset, width, height, 10, 10);
        painter->drawText(textbox, caption, Qt::AlignLeft|Qt::AlignTop);
    } else if(state==HIDING) {
        //We want to hide the caption, but first we must undraw it (which will
        //happen once this function is called) and then we switch the state back
        state=HIDDEN;
    }
}

QRectF ThumbnailCaption::boundingRect() const
{
    int x_size, y_size;
    if(state!=HIDDEN) {
        //If we either showing the caption, or about to hide it and we want to
        //draw the full extents of the caption
        x_size = width;
        y_size = height;
    } else {
        //The caption has nothing to do with us
        x_size = 0;
        y_size = 0;
    }
    return QRectF(0, 0, x_size, y_size);
}

Thumbnail::Thumbnail() : QGraphicsItem()
{
    thumbnail_size = 90;
    tile_size = 100;
    margin = (tile_size - thumbnail_size)/2;
    //http://doc.trolltech.com/4.3/resources.html
    pm = QPixmap(":/Images/Icons/chhobi-icon.png");
    tc.caption = "Captions should not be very long; about one to two lines.";
    tc.width = 200;
    tc.height = 50;
    tc.x_offset = tile_size;
    tc.y_offset = tile_size;
    setAcceptsHoverEvents(true);
}

QRectF Thumbnail::boundingRect() const
{
    QRectF caption_bb = tc.boundingRect();
    int x_size = tile_size + caption_bb.width(), y_size = tile_size + caption_bb.height();
    return QRectF(0, 0, x_size, y_size);
}

void Thumbnail::paint(QPainter *painter,
           const QStyleOptionGraphicsItem *option,
           QWidget *widget)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(Qt::white);
    painter->drawRoundedRect(0, 0, tile_size, tile_size, 2*margin, 2*margin);
    painter->drawPixmap(margin, margin, thumbnail_size, thumbnail_size, pm);
    tc.paint(painter);
}

void Thumbnail::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    tc.state = SHOWN;
    prepareGeometryChange();
    QGraphicsItem::hoverEnterEvent(event);
}

void Thumbnail::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    tc.state = HIDING;
    prepareGeometryChange();
    QGraphicsItem::hoverLeaveEvent(event);
}
