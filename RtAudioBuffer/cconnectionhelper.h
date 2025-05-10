#ifndef CCONNECTIONHELPER_H
#define CCONNECTIONHELPER_H

#include <QGraphicsView>
#include <QGraphicsPathItem>
#include <QGraphicsTextItem>
#include "softsynthsdefines.h"

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
    static QList<QGraphicsItem*> DrawArrow(const QPoint& OutPoint, const QPoint& InPoint, QColor Color, QGraphicsScene* Scene, const int lineWidth=1)
    {
        QList<QGraphicsItem*> items;
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
        items.append(Scene->addPath(path,QPen(Color,lineWidth),Qt::NoBrush));
        path=QPainterPath(QPoint(0,0));
        path.moveTo(p3);
        path.lineTo(Mid);
        path.lineTo(p4);
        items.append(Scene->addPath(path,QPen(Color,lineWidth),Color));
        return items;
    }
    static QList<QGraphicsItem*> DrawShadowText(const QString& text, const QFont& font, const QPoint& pos, QGraphicsScene* Scene)
    {
        QList<QGraphicsItem*> items;
        QGraphicsSimpleTextItem* item = Scene->addSimpleText(text,font);
        item->setPos(pos);
        item->setBrush(QBrush("#ddd"));
        item->setPen(Qt::NoPen);
        items.append(item);
        item = Scene->addSimpleText(text,font);
        item->setPos(pos+QPoint(-1,-1));
        item->setBrush(QBrush("#222"));
        item->setPen(Qt::NoPen);
        items.append(item);

        return items;
    }
    static QList<QGraphicsItem*> DrawShadowTextCenter(const QString& text, const QFont& font, const QPoint& pos, const QSize& size, const Qt::Alignment& alignment, QGraphicsScene* Scene)
    {
        QList<QGraphicsItem*> items;
        DiagramTextItem* item = new DiagramTextItem();
        item->setFont(font);
        item->setBoundingRect(QRect(pos,size));
        item->setBrush(QBrush("#ddd"));
        item->setPen(QPen(QColor("#ddd"),1));
        item->setText(text);
        item->setAlignment(alignment);
        Scene->addItem(item);
        items.append(item);
        item = new DiagramTextItem();
        item->setFont(font);
        item->setBoundingRect(QRect(pos+QPoint(-1,-1),size));
        item->setBrush(QBrush("#222"));
        item->setPen(QPen(QColor("#222"),1));
        item->setText(text);
        item->setAlignment(alignment);
        Scene->addItem(item);
        items.append(item);

        return items;
    }
};

#endif // CCONNECTIONHELPER_H
