#include "photoribbon.h"

RibbonPhoto::RibbonPhoto(unsigned int tile_width) : QGraphicsRectItem()
{
    setRect(0,0,tile_width,tile_width);
    //setBrush(Qt::white);
    setFlag(QGraphicsItem::ItemIsSelectable);
}

void RibbonPhoto::paint(QPainter *painter,
           const QStyleOptionGraphicsItem *option,
           QWidget *widget)
{
    if(isSelected())
        setBrush(Qt::gray);
    else
        setBrush(Qt::white);
    QGraphicsRectItem::paint(painter, option, widget);
}

PhotoRibbon::PhotoRibbon(QObject *parent) :
    QGraphicsScene(parent)
{
    tile_width = 10;
    setBackgroundBrush(Qt::black);
}

void PhotoRibbon::set_ids(QList<unsigned int> ids)
{
    QList<unsigned int>::iterator i;
    int x = 0;
    for (i = ids.begin(); i != ids.end(); ++i) {
        RibbonPhoto *rp = new RibbonPhoto(tile_width);
        rp->set_id(*i);
        rp->setPos(x,0);
        addItem((QGraphicsItem *)rp);
        x += tile_width;
    }
}
