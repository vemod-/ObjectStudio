#include "ctunerwidget.h"
#include "ui_ctunerwidget.h"

CTunerWidget::CTunerWidget(QWidget *parent) :
    QCanvas(parent),
    ui(new Ui::CTunerWidget)
{
    ui->setupUi(this);
    c = 0;
    memset(&r,0,sizeof(r));
    clear(Qt::black);
}

CTunerWidget::~CTunerWidget()
{
    delete ui;
}

void CTunerWidget::setTune(CPitchDetect::PitchRecord rec, double calib)
{
    if (!closeEnough(calib,c))
    {
        QRect freqRect(5,5,60,15);
        setPenBrush(Qt::black);
        drawRectangle(freqRect);
        setPenBrush(Qt::gray);
        setLayerFontSize(12);
        QString txt=QString::number(calib)+" Hz";
        drawText(freqRect.topLeft(),txt);
        update(freqRect);
    }
    c=calib;
    if (!closeEnough(r.Pitch,rec.Pitch))
    {
        const int w = width();
        const QRect indRect(10,85,w-20,30);
        const QRect freqRect(30,120,w-60,15);
        const QRect noteRect(65,10,w-130,70);
        setPenBrush(Qt::black);
        drawRectangle(freqRect);
        drawRectangle(noteRect);
        setPen(Qt::darkGray);
        drawRectangle(indRect);
        const int x = qRound(cent2X(rec.MidiCents));
        const int xc=qRound(cent2X(0));
        if (rec.MidiKey)
        {
            if (rec.MidiCents==0)
            {
                QColor c("#4F4");
                setPenBrush(c);
                drawRectangle(x-3,indRect.top()+1,7,indRect.height()-2);
                setPenBrush(Qt::yellow);
                drawRectangle(x-1,indRect.top()+1,3,indRect.height()-2);
                setPenBrush(c);
            }
            else
            {
                QColor c("#F22");
                if (rec.MidiCents < 0) c=QColor("#AA0");
                QRect diffRect(QPoint(xc,indRect.top()+1),QPoint(x,indRect.bottom()-1));
                setPenBrush(c);
                drawRectangle(diffRect.normalized());
                setPenBrush(Qt::yellow);
                drawRectangle(x-1,indRect.top()+1,3,indRect.height()-2);
                setPenBrush(c);
            }
            setLayerFontSize(12);
            QString txt=QString::number(rec.Pitch)+" Hz";
            int hw = QFontMetrics(layerFont()).horizontalAdvance(txt)/2;
            drawText(freqRect.center().x()-hw,freqRect.top(),txt);
            setLayerFontSize(60);
            txt=CPitchDsp::GetNoteName(rec.MidiKey,true,true);
            hw = QFontMetrics(layerFont()).horizontalAdvance(txt)/2;
            drawText(noteRect.center().x()-hw,noteRect.top(),txt);
        }
        //update(freqRect);
        //update(indRect);
        //update(noteRect);
        update();
    }
    r=rec;
}

double CTunerWidget::cent2X(int cent)
{
    return ((width() - 20)*(-cent+50)*0.01)+10;
}

