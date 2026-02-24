#ifndef QGRAPHICSITEMLIST_H
#define QGRAPHICSITEMLIST_H

#include <QGraphicsItem>
#include <QList>
#include <QGraphicsScene>
#include <QPainter>
#include <QPropertyAnimation>
#include <QGraphicsView>

inline QGraphicsItem* pathItem(const QPainterPath& p, const QPen& pen, const QBrush& brush, const QPointF& pos = QPointF()) {
    QGraphicsPathItem* path = new QGraphicsPathItem(p);
    path->setPen(pen);
    path->setBrush(brush);
    path->setPos(pos);
    return path;
}

inline QGraphicsItem* rectItem(const QRectF& p, const QPen& pen, const QBrush& brush, const QPointF& pos = QPointF()) {
    QGraphicsRectItem* path = new QGraphicsRectItem(p);
    path->setPen(pen);
    path->setBrush(brush);
    path->setPos(pos);
    return path;
}

inline QGraphicsItem* rectItem(qreal x, qreal y, qreal w, qreal h, const QPen& pen, const QBrush& brush, const QPointF& pos = QPointF()) {
    return rectItem(QRectF(x,y,w,h),pen,brush,pos);
}

inline QGraphicsItem* ellipseItem(const QRectF& p, const QPen& pen, const QBrush& brush, const QPointF& pos = QPointF()) {
    QGraphicsEllipseItem* path = new QGraphicsEllipseItem(p);
    path->setPen(pen);
    path->setBrush(brush);
    path->setPos(pos);
    return path;
}

inline QGraphicsItem* ellipseItem(qreal x, qreal y, qreal w, qreal h, const QPen& pen, const QBrush& brush, const QPointF& pos = QPointF()) {
    return ellipseItem(QRectF(x,y,w,h),pen,brush,pos);
}

inline QGraphicsItem* lineItem(const QLineF& p, const QPen& pen, const QPointF& pos = QPointF()) {
    QGraphicsLineItem* path = new QGraphicsLineItem(p);
    path->setPen(pen);
    path->setPos(pos);
    return path;
}

inline QGraphicsItem* lineItem(qreal x1, qreal y1, qreal x2, qreal y2, const QPen& pen, const QPointF& pos = QPointF()) {
    return lineItem(QLineF(x1,y1,x2,y2),pen,pos);
}

class QGraphicsItemList : public QList<QGraphicsItem*> {
public:
    QGraphicsItemList() : QList<QGraphicsItem*>(){}
    QGraphicsItemList(const QList<QGraphicsItem*>& other) : QList<QGraphicsItem*>(other){}
    void append(QGraphicsItem* i) {
        if (i == nullptr) return;
        QList::append(i);
    }
    void append(const QGraphicsItemList& l) {
        for (QGraphicsItem* i : l) append(i);
    }
    void erase(QGraphicsScene* s) {
        s->blockSignals(true);
        for (QGraphicsItem* i : *this) {
            if (i->scene() == s) delete i;
        }
        clear();
        s->blockSignals(false);
    }
    void removeFromScene(QGraphicsScene* s) {
        s->blockSignals(true);
        for (QGraphicsItem* i : *this) {
            if (i->scene() == s) s->removeItem(i);
        }
        s->blockSignals(false);
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
    QGraphicsContainerItem(QGraphicsScene* s = nullptr) {
        if (s) s->addItem(this);
    }
    QGraphicsContainerItem(const QGraphicsItemList& l, QGraphicsScene* s = nullptr) {
        if (s) s->addItem(this);
        append(l);
    }
    QRectF boundingRect() const override {
        QRectF r;
        for (auto* c : childItems()) r |= c->mapToParent(c->boundingRect()).boundingRect();
        return r;
    }
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override {}
    void append(QGraphicsItem* i) {
        if (i == nullptr) return;
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

class QGraphicsToolButton : public QGraphicsObject
{
    Q_OBJECT
public:
    QGraphicsToolButton(QGraphicsItem* parent = nullptr)
        : QGraphicsObject(parent)
    {
        setAcceptHoverEvents(true);
        setAcceptedMouseButtons(Qt::LeftButton);
        setVisible(true);
        setOpacity(0.01);
    }
    QGraphicsToolButton(const QPixmap& pixmap, QGraphicsItem* parent = nullptr)
        : QGraphicsObject(parent)
    {
        setPixmap(pixmap);
        m_Rect = pixmap.rect();
        setAcceptHoverEvents(true);
        setAcceptedMouseButtons(Qt::LeftButton);
        setVisible(true);
        setOpacity(0.01);
    }
    QRectF boundingRect() const override {
        return m_Rect;
    }

    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override {
        p->setRenderHint(QPainter::Antialiasing);
        QRectF r = m_Pixmap.rect();
        r.moveCenter(m_Rect.center());
        p->drawPixmap(r,m_Pixmap,m_Pixmap.rect());
    }

    void setPixmap(const QPixmap& pixmap) {
        m_Pixmap = pixmap;
    }
    void setSize(QSizeF s) {
        m_Rect.setSize(s);
    }
    void setSize(qreal x, qreal y) {
        setSize(QSize(x,y));
    }
signals:
    void clicked();
private:
    QPixmap m_Pixmap;
    QRectF m_Rect;
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent*) override {
        setOpacity(1.0);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override {
        setOpacity(0.01);
    }

    void mousePressEvent(QGraphicsSceneMouseEvent*) override {
        emit clicked();
        if ((!isEnabled()) || (!isUnderMouse())) setOpacity(0.01);
    }
};

class QGraphicsRubberBand : public QGraphicsItem {
public:
    QGraphicsRubberBand(QGraphicsView* view, QGraphicsItem* parent = nullptr) : QGraphicsItem(parent) {
        QGraphicsItem::hide();
        m_View = view;
        view->scene()->addItem(this);
    }
    QRectF boundingRect() const override {
        return m_Rect.adjusted(-m_PenWidth / 2, -m_PenWidth / 2, m_PenWidth / 2, m_PenWidth / 2);
    }
    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override {
        if (m_Rect.isEmpty()) return;
        if (!isVisible()) return;
        /*
        p->setBrush(QColor(100,100,0,15));
        QColor c(QColor(0xd8,0xf1,0));
        c.setAlpha(220);
        p->setPen(QPen(c,m_PenWidth,Qt::SolidLine,Qt::SquareCap,Qt::RoundJoin));
*/
        p->setBrush(QColor(0,0,0,30));
        p->setPen(QPen(Qt::yellow,m_PenWidth,Qt::SolidLine,Qt::SquareCap,Qt::RoundJoin));
        p->drawRoundedRect(m_Rect,m_PenWidth * 5, m_PenWidth * 5);
    }
    void setRect(QRectF r) {
        m_Rect = r;
    }
    QRectF rect() {
        return m_Rect;
    }
    bool isVisible() {
        if (m_Rect.isEmpty()) return false;
        return QGraphicsItem::isVisible();
    }
    void show(QRectF r) {
        m_Rect = r;
        if (!isVisible()) QGraphicsItem::show();
        m_View->viewport()->update();
    }
    void hide() {
        if (isVisible()) {
            QGraphicsItem::hide();
            m_View->viewport()->update();
        }
    }
    /*
    void animate() {
        auto *anim = new QPropertyAnimation(this, "rect");
        anim->setEasingCurve(QEasingCurve::SineCurve);
        //anim->setDuration(150);
        anim->setDuration(120);
        anim->setStartValue(m_Rect);
        anim->setEndValue(m_Rect.adjusted(-10,-10,10,10));
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
*/
    void setPenWidth(qreal p) {
        m_PenWidth = p;
    }
private:
    qreal m_PenWidth = 5;
    QRectF m_Rect;
    QGraphicsView* m_View;
};

class QGraphicsIPhotoRubberband : public QGraphicsItem {
public:
    QGraphicsIPhotoRubberband(QGraphicsView* view, QGraphicsItem* parent = nullptr) : QGraphicsItem(parent) {
        QGraphicsItem::hide();
        m_View = view;
        view->scene()->addItem(this);
    }
    QRectF boundingRect() const override {
        return m_BoundingRect;
    }
    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override {
        if (!isVisible()) return;
        QPainterPath path(QPoint(0,0));
        path.addRect(m_BoundingRect);
        if (!m_WindowGeometry.isEmpty()) path.addRect(m_WindowGeometry);
        p->setPen(Qt::NoPen);
        p->setBrush(QColor(0,0,0,30));
        p->drawPath(path);
        if (!m_WindowGeometry.isEmpty()) {
            p->setPen(QPen(Qt::yellow,m_PenWidth,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
            p->setBrush(Qt::NoBrush);
            p->drawRoundedRect(m_WindowGeometry,m_PenWidth * 5,m_PenWidth * 5);
        }
    }
    void setWindowGeometry(QRectF r) {
        m_WindowGeometry = r.normalized();
    }
    QRectF windowGeometry() {
        return m_WindowGeometry;
    }
    void show(QRectF r) {
        m_BoundingRect = m_View->mapToScene(m_View->viewport()->rect()).boundingRect();
        setWindowGeometry(r);
        if (!isVisible()) QGraphicsItem::show();
        m_View->viewport()->update();
    }
    void hide() {
        if (isVisible()) {
            QGraphicsItem::hide();
            m_View->viewport()->update();
        }
    }
    void setPenWidth(qreal p) {
        m_PenWidth = p;
    }
private:
    qreal m_PenWidth = 5;
    QRectF m_BoundingRect;
    QRectF m_WindowGeometry;
    QGraphicsView* m_View;
};

#endif // QGRAPHICSITEMLIST_H
