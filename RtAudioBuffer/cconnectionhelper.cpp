#include "cconnectionhelper.h"

DiagramTextItem::DiagramTextItem(QGraphicsItem *parent)
    : QGraphicsTextItem(parent)
{
    myBoundRect.setRect( 0, 0, 0, 0 );

    textOp.setAlignment( Qt::AlignCenter | Qt::AlignHCenter );
    textOp.setWrapMode( QTextOption::WrapAtWordBoundaryOrAnywhere );
}

void DiagramTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem
*option, QWidget *widget)
{
    //painter->drawRect( boundingRect() );
    painter->setPen(pen);
    painter->setBrush(brush);

    painter->setFont( font );

    painter->drawText(	boundingRect(),
                        text,
                        textOp);

    QGraphicsTextItem::paint(painter, option, widget);
}

QRectF DiagramTextItem::boundingRect() const
{
    return myBoundRect;
}

void DiagramTextItem::setBoundingRect( qreal x, qreal y, qreal w, qreal h)
{
    myBoundRect.setRect( x, y, w, h );
}

void DiagramTextItem::setBoundingRect( const QRectF& r)
{
    myBoundRect = r;
}

void DiagramTextItem::setText( const QString &inText )
{
    text = inText;
    //update();
}

void DiagramTextItem::setFont( const QFont& f )
{
    font = f;
}

void DiagramTextItem::setPen( const QPen& p )
{
    pen = p;
}

void DiagramTextItem::setBrush( const QBrush& b )
{
    brush = b;
}

void DiagramTextItem::setAlignment( const Qt::Alignment a )
{
    textOp.setAlignment(a);
}

void DiagramTextItem::setWrapMode( const QTextOption::WrapMode w )
{
    textOp.setWrapMode(w);
}
