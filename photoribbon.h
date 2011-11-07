#ifndef PHOTORIBBON_H
#define PHOTORIBBON_H

#include <QtGui>

enum RibbonTileState {NORMAL, PREVIEW};
class RibbonTile : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

    unsigned int id;
    RibbonTileState state;

public:
    explicit RibbonTile(unsigned int tile_width);
    void set_id(unsigned int c_id) {id=c_id;}
    unsigned int get_id() {return id;}
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget);
    void set_preview() {state=PREVIEW;}
    void unset_preview() {state=NORMAL;}

private:
    void hoverEnterEvent(QGraphicsSceneHoverEvent * event);

signals:
    void preview(RibbonTile *);
};

class PhotoRibbon : public QGraphicsScene
{
    Q_OBJECT

    unsigned int item_count, tile_width;
    RibbonTile *preview_tile;

public:
    explicit PhotoRibbon(QObject *parent = 0);
    void set_ids(QList<unsigned int> ids);

signals:
    void preview_id(unsigned int);//Emitted whenever we choose a tile

public slots:
    void set_preview_tile(RibbonTile *);
};

#endif // PHOTORIBBON_H
