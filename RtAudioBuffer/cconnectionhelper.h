#ifndef CCONNECTIONHELPER_H
#define CCONNECTIONHELPER_H

#include <QGraphicsView>
#include <QGraphicsPathItem>
#include <QGraphicsTextItem>
#include "softsynthsdefines.h"
#include "qgraphicsitemlist.h"
#include "ijack.h"
#include <QApplication>

class DiagramTextItem : public QGraphicsTextItem
{
    Q_OBJECT

public:
    enum { Type = UserType + 3 };
    DiagramTextItem(QGraphicsItem *parent = 0);
    void setBoundingRect( qreal x, qreal y, qreal w, qreal h);
    void setBoundingRect( const QRectF& r );
    void setText( const QString &inText );
    void setFont( const QFont& f );
    void setPen( const QPen& p );
    void setBrush( const QBrush& b);
    void setAlignment( const Qt::Alignment a );
    void setWrapMode( const QTextOption::WrapMode w );
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    QRectF boundingRect() const;
private:
    QRectF myBoundRect;
    QTextOption textOp;
    QString text;
    QFont font;
    QPen pen;
    QBrush brush;
};

class CConnectionHelper
{
public:
    static QGraphicsItemList DrawArrow(const QPoint& OutPoint, const QPoint& InPoint, QColor Color, QGraphicsScene* Scene, int zValue = 0, const qreal lineWidth = 2.0)
    {
        QGraphicsItemList items;
        QPointF s(InPoint-OutPoint);
        QPoint Mid(InPoint -((InPoint-OutPoint)/2));
        QPoint Mid14(InPoint-((InPoint-OutPoint)/4));
        QPoint Mid34(InPoint-(((InPoint-OutPoint)*3)/4));

        QPoint Dist(InPoint-OutPoint);
        double Distance=sqrt((Dist.x()*Dist.x())+(Dist.y()*Dist.y()))/10.0;

        if ((InPoint.x()<OutPoint.x()) && (InPoint.y()>OutPoint.y())) Distance=-Distance;
        if ((InPoint.x()<OutPoint.x()) && (InPoint.y()<OutPoint.y())) Distance=-Distance;

        double theta;
        if (!isZero(s.x()))
        {
            theta = atan(s.y() / s.x());
            if (s.x() < 0) theta = theta + M_PI;
        }
        else
        {
            if (s.y() < 0)
            {
                theta = 3 * M_PI / 2;
            }
            else
            {
                theta = M_PI / 2;
            }
        }
        //'rotate direction
        double theta1 = theta - 3 * M_PI / 4;
        //'find end of one side of arrow:
        int L = 6;
        QPointF p3((L * cos(theta1)) + Mid.x(),(L * sin(theta1)) + Mid.y());
        //'rotate other way for other arrow line
        theta1 = theta1 - M_PI / 2;
        QPointF p4((L * cos(theta1)) + Mid.x(),(L * sin(theta1)) + Mid.y());

        theta1 = theta - 2 * M_PI / 4;
        QPointF p14((Distance * cos(theta1)) + Mid14.x(),(Distance * sin(theta1)) + Mid14.y());
        theta1 = theta1 - M_PI;
        QPointF p34((Distance * cos(theta1)) + Mid34.x(),(Distance * sin(theta1)) + Mid34.y());

        //'draw the lines
        QPainterPath path(QPoint(0,0));
        path.moveTo(OutPoint);
        path.cubicTo(p34,p14,InPoint);
        items.append(Scene->addPath(path,QPen(Color,lineWidth,Qt::SolidLine,Qt::RoundCap),Qt::NoBrush));
        path=QPainterPath(QPoint(0,0));
        path.moveTo(p3);
        path.lineTo(Mid);
        path.lineTo(p4);
        items.append(Scene->addPath(path,QPen(Color,lineWidth,Qt::SolidLine,Qt::RoundCap),Color));
        if (zValue) items.setZValue(zValue);
        return items;
    }
    static QGraphicsItemList DrawCord(QPoint p1, QPoint p2, const QColor& color, QGraphicsScene* Scene, const qreal linewidth = 5.0)
    {
        QGraphicsItemList l;
        QPainterPath p;
        QRect r(p1,p2);
        r=r.normalized();
        int adjust = 60 - r.width();
        if (adjust < 0) adjust=0;
        r.adjust(adjust,(r.height()/5)+50,-adjust,(r.height()/5)+100);
        if (p1.x() > p2.x()) std::swap(p1,p2);
        p.moveTo(p1);
        if (p1.y() < p2.y())
        {
            p.cubicTo(r.bottomLeft(),p2+((r.bottomRight()-p2)/2),p2);
        }
        else
        {
            p.cubicTo(p1+((r.bottomLeft()-p1)/2),r.bottomRight(),p2);
        }
        l.append(Scene->addPath(p.translated(5,5),QPen(QColor(0,0,0,40),linewidth,Qt::SolidLine,Qt::RoundCap)));
        QColor c(color);
        l.append(Scene->addPath(p,QPen(c,linewidth,Qt::SolidLine,Qt::RoundCap)));
        l.setZValue(2);
        //for (QGraphicsItem* i : l)  i->setZValue(2);
        return l;
    }
    static QGraphicsContainerItem* DrawShadowText(const QString& text, const QFont& font, const QPoint& pos, int zValue = 0)
    {
        QGraphicsContainerItem* items = new QGraphicsContainerItem;
        QGraphicsSimpleTextItem* item = new QGraphicsSimpleTextItem(text);
        item->setFont(font);
        item->setPos(pos);
        item->setBrush(QBrush(QColor(0xdd,0xdd,0xdd)));
        item->setPen(Qt::NoPen);
        items->append(item);
        item = new QGraphicsSimpleTextItem(text);
        item->setFont(font);
        item->setPos(pos+QPoint(-1,-1));
        item->setBrush(QColor(0x22,0x22,0x22));
        item->setPen(Qt::NoPen);
        items->append(item);
        if (zValue) items->setZValue(zValue);
        return items;
    }
    static QGraphicsContainerItem* DrawShadowTextCenter(const QString& text, const QFont& font, const QPoint& pos, const QSize& size, const Qt::Alignment& alignment, int zValue = 0)
    {
        QGraphicsContainerItem* items = new QGraphicsContainerItem;
        DiagramTextItem* item = new DiagramTextItem();
        item->setFont(font);
        item->setBoundingRect(QRect(pos,size));
        item->setBrush(QBrush(QColor(0xdd,0xdd,0xdd)));
        item->setPen(QPen(QColor(QColor(0xdd,0xdd,0xdd)),1));
        item->setText(text);
        item->setAlignment(alignment);
        //Scene->addItem(item);
        items->append(item);
        item = new DiagramTextItem();
        item->setFont(font);
        item->setBoundingRect(QRect(pos+QPoint(-1,-1),size));
        item->setBrush(QBrush(QColor(0x22,0x22,0x22)));
        item->setPen(QPen(QColor(QColor(0x22,0x22,0x22)),1));
        item->setText(text);
        item->setAlignment(alignment);
        //Scene->addItem(item);
        items->append(item);
        if (zValue) items->setZValue(zValue);
        return items;
    }
    static Qt::CursorShape connectCursor(QWidget* w, IJack* J1, IJack* J2)
    {
        if (J1 != J2)
        {
            w->setToolTip(J2->captionX());
            return  ((J1->canConnectTo(J2)) && (!J1->isConnectedTo(J2))) ? Qt::PointingHandCursor : Qt::ForbiddenCursor;
        }
        w->setToolTip(QString());
        return Qt::OpenHandCursor;
    }

    static void SetConnectCursor(QWidget* w, IJack* HoverJack, IJack* DragJack)
    {
        if (HoverJack)
        {
            QApplication::restoreOverrideCursor();
            QApplication::setOverrideCursor(connectCursor(w,DragJack,HoverJack));
            return;
        }
        QApplication::restoreOverrideCursor();
        w->setToolTip(QString());
    }

};

#endif // CCONNECTIONHELPER_H
