#include "photoribbon.h"

RibbonTile::RibbonTile(unsigned int tile_width) : QGraphicsRectItem()
{
    state = NORMAL;
    setRect(0,0,.75*tile_width,tile_width);
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
    tile_width = 10;
    preview_tile = NULL;
    setBackgroundBrush(Qt::black);
}

void PhotoRibbon::set_ids(QList<unsigned int> ids)
{
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
}

void PhotoRibbon::set_preview_tile(RibbonTile *prt)
{
    if(preview_tile != NULL)
        preview_tile->unset_preview();
    preview_tile = prt;
    preview_tile->set_preview();
    emit preview_id(preview_tile->get_id());
}
