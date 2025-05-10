#include "cwavetrack.h"
#include <QGraphicsTextItem>
#include <QGraphicsPathItem>

CWaveTrack::CWaveTrack(const QString &Filename, ulong64 StartPointer)
{
    name=Filename;
    start=StartPointer;
    isValid=waveGenerator.load(Filename);
    loopParameters.reset(waveGenerator.size());
}

void CWaveTrack::paint(QGraphicsScene& Scene, ldouble ZoomFactor, QRect visibleRect,int edge)
{
    if (!geometry.intersects(visibleRect)) return;
    QPen p;
    QLinearGradient lg(0,geometry.top(),0,geometry.height()+geometry.top());
    if (isActive)
    {
        p=QPen(Qt::black);
        lg.setColorAt(0,"#eee");
        lg.setColorAt(0.49999,"#bbb");
        lg.setColorAt(0.5,"#afafaf");
        lg.setColorAt(1,"#999");
    }
    else
    {
        p=QPen(Qt::gray);
        lg.setColorAt(0,"#ddd");
        lg.setColorAt(0.49999,"#aaa");
        lg.setColorAt(0.5,"#8f8f8f");
        lg.setColorAt(1,"#777");
    }
    QPainterPath path(QPoint(0,0));
    path.addRoundedRect(geometry,6,6);
    Scene.addPath(path,p,lg);

    if (edge == 1) {
        Scene.addLine(geometry.left()+3,geometry.top()+2,geometry.left()+3,geometry.bottom()-2,QPen(Qt::darkGray,4,Qt::SolidLine,Qt::RoundCap));
    }
    else if (edge == 2) {
        Scene.addLine(geometry.right()-3,geometry.top()+2,geometry.right()-3,geometry.bottom()-2,QPen(Qt::darkGray,4,Qt::SolidLine,Qt::RoundCap));
    }
    QFont f = Scene.font();
    f.setPointSize(9);
    QFontMetrics fm(f);
    QString Caption=QFileInfo(name).baseName();
    while (fm.horizontalAdvance(Caption)>geometry.width())
    {
        Caption.chop(1);
    }
    QGraphicsTextItem* t = Scene.addText(Caption,f);
    t->setPos(geometry.topLeft());

    waveGenerator.paint(Scene,geometry,visibleRect,ZoomFactor,&loopParameters);

    QPen redPen = QPen(Qt::red);
    float volHeight = geometry.height()*loopParameters.Volume*0.01;
    qreal fadeInWidth = loopParameters.FadeIn*ZoomFactor/loopParameters.Speed;
    qreal fadeOutWidth = loopParameters.FadeOut*ZoomFactor/loopParameters.Speed;
    Scene.addLine(geometry.left(),geometry.bottom(),geometry.left()+fadeInWidth,geometry.bottom()-volHeight,redPen);
    Scene.addLine(geometry.left()+fadeInWidth,geometry.bottom()-volHeight,geometry.width()-fadeOutWidth,geometry.bottom()-volHeight,redPen);
    Scene.addLine(geometry.right()-fadeOutWidth,geometry.bottom()-volHeight,geometry.right(),geometry.bottom(),redPen);
}
