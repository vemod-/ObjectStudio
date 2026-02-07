#ifndef QGRAPHICSITEMLIST_H
#define QGRAPHICSITEMLIST_H

#include <QGraphicsItem>
#include <QList>
#include <QGraphicsScene>


class QGraphicsItemList : public QList<QGraphicsItem*> {
public:
    QGraphicsItemList() : QList<QGraphicsItem*>(){}
    QGraphicsItemList(const QList<QGraphicsItem*>& other) : QList<QGraphicsItem*>(other){}
    void erase(QGraphicsScene* s) {
        for (QGraphicsItem* i : *this) {
            if (i->scene()) s->removeItem(i);
            delete i;
        }
        clear();
        m_Pos = QPointF();
    }
    void removeFromScene(QGraphicsScene* s) {
        for (QGraphicsItem* i : *this) {
            //i->setPos(i->pos() - m_Pos);
            if (i->scene()) s->removeItem(i);
        }
    }
    void remove(const QGraphicsItemList& l) {
        for (QGraphicsItem* i : l) removeOne(i);
    }
    void removeOne(QGraphicsItem* i) {
        if (contains(i)) {
            i->setPos(i->pos() - m_Pos);
            QList::removeOne(i);
        }
    }
    void addToScene(QGraphicsScene* s) {
        for (QGraphicsItem* i : *this) {
            s->addItem(i);
        }
    }
    void setVisible(bool v) {
        for (QGraphicsItem* i : *this) {
            i->setVisible(v);
        }
    }
    void setPos(const QPointF& pos) {
        const QPointF move = pos - m_Pos;
        for (QGraphicsItem* i : *this) {
            i->setPos(i->pos() + move);
        }
        m_Pos = pos;
    }
    void setPos(qreal x, qreal y) {
        setPos(QPointF(x,y));
    }
    QPointF pos() const {
        return m_Pos;
    }
    void setZValue(int z) {
        for (QGraphicsItem* i : *this) {
            i->setZValue(z);
        }
    }
    void stackBefore(const QGraphicsItem* b) {
        const qreal zValue = b->zValue();
        for (QGraphicsItem* i : *this) {
            if (i->zValue() == zValue) i->stackBefore(b);
        }
    }
private:
    QPointF m_Pos;
};

class QGraphicsContainerItem : public QGraphicsItem
{
public:
    QRectF boundingRect() const override {
        QRectF r;
        for (auto* c : childItems()) r |= c->mapToParent(c->boundingRect()).boundingRect();
        return r;
    }
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override {}
    void append(QGraphicsItem* i) {
        i->setParentItem(this);
    }
    void append(QGraphicsItemList l) {
        for (auto* i : l) append(i);
    }
    void removeOne(QGraphicsItem* i) {
        if (childItems().contains(i)) i->setParentItem(nullptr);
    }
    void remove(QGraphicsItemList& l) {
        for (auto* i : l) removeOne(i);
    }
    void clear() {
        while (!childItems().isEmpty()) delete childItems().takeLast();
    }
    void addToScene(QGraphicsScene* s) {
        if (scene() != s) s->addItem(this);
    }
};

#endif // QGRAPHICSITEMLIST_H
