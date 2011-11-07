#ifndef PHOTORIBBON_H
#define PHOTORIBBON_H

#include <QtGui>

class RibbonPhoto : public QGraphicsRectItem
{
    unsigned int id;
public:
    explicit RibbonPhoto(unsigned int tile_width);
    void set_id(unsigned int c_id) {id=c_id;}
    unsigned int get_id() {return id;}
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget);
};

class PhotoRibbon : public QGraphicsScene
{
    Q_OBJECT

    unsigned int item_count, tile_width;

public:
    explicit PhotoRibbon(QObject *parent = 0);
    void set_ids(QList<unsigned int> ids);

signals:

public slots:
};

#endif // PHOTORIBBON_H
