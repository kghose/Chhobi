#include "thumbnail.h"

Thumbnail::Thumbnail() : QGraphicsItem()
{
    thumbnail_size = 90;
    tile_size = 100;
    margin = (tile_size - thumbnail_size)/2;
    //http://doc.trolltech.com/4.3/resources.html
    pm = QPixmap(":/Images/Icons/chhobi-icon.png");
}

QRectF Thumbnail::boundingRect() const
{
    return QRectF(0, 0, tile_size, tile_size);
}

void Thumbnail::paint(QPainter *painter,
           const QStyleOptionGraphicsItem *option,
           QWidget *widget)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(Qt::white);
    painter->drawRoundedRect(0, 0, tile_size, tile_size, 2*margin, 2*margin);
    painter->drawPixmap(margin, margin, thumbnail_size, thumbnail_size, pm);
}
