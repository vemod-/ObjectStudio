#include "ctunerwidget.h"
//#include "cpitchdsp.h"
#include "cpitchtextconvert.h"
#include <QPainter>
//#include "ui_ctunerwidget.h"
/*
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

void CTunerWidget::setTune(CYIN::PitchRecord rec, double calib)
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
                QColor c(0x44,0xff,0x44);
                setPenBrush(c);
                drawRectangle(x-3,indRect.top()+1,7,indRect.height()-2);
                setPenBrush(Qt::yellow);
                drawRectangle(x-1,indRect.top()+1,3,indRect.height()-2);
                setPenBrush(c);
            }
            else
            {
                QColor c(0xff,0x22,0x22);
                if (rec.MidiCents < 0) c=QColor(0xa,0xa,0);
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
        update();
    }
    r=rec;
}

double CTunerWidget::cent2X(int cent)
{
    return ((width() - 20)*(-cent+50)*0.01)+10;
}
*/
CTunerWidget::CTunerWidget(QWidget *parent)
    : QWidget(parent)
{
    m_bigFont.setPointSize(48);
    m_smallFont.setPointSize(12);
}

void CTunerWidget::setPitchRecord(const CYIN::PitchRecord& rec)
{
    m_rec = rec;
    if (memcmp(&m_rec, &m_lastDrawnRec, sizeof(m_rec)) == 0) return;
    if (fabs(m_rec.Pitch - m_lastDrawnRec.Pitch) > 0.05f) {
        m_pitchText = QString::number(m_rec.Pitch, 'f', 1) + " Hz";
    }
    if (m_rec.MidiKey != m_lastDrawnRec.MidiKey) {
        m_noteText = CPitchTextConvert::pitch2Text(m_rec.MidiKey);
    }
    //QMetaObject::invokeMethod(this,&CTunerWidget::update,Qt::QueuedConnection);
    QMetaObject::invokeMethod(
        this,
        [this]() {
            update();
        },
        Qt::QueuedConnection
        );
}

void CTunerWidget::setCalib(double calib){
    m_calib = calib;
    if (fabs(m_calib - m_lastDrawnCalib) > 0.1) {
        m_calibText = QString::number(m_calib, 'f', 1) + " Hz";
    }
}

void CTunerWidget::resizeEvent(QResizeEvent *) {
    const int w = width();
    xc = cent2X(0);
    indRect = QRect(10,85,w-20,30);
    freqRect = QRect(30,120,w-60,20);
    noteRect = QRect(65,10,w-130,70);
    calibRect = QRect(5,5,80,15);
}

int CTunerWidget::cent2X(int cent) const
{
    return ((width() - 20) * (cent + 50) * 0.01) + 10;
}

void CTunerWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    // ================= BACKGROUND =================
    p.fillRect(rect(), Qt::black);

    // ================= CACHE TEXT =================
    m_lastDrawnRec = m_rec;
    m_lastDrawnCalib = m_calib;

    // ================= CALIB =================
    p.setPen(Qt::gray);
    p.setFont(m_smallFont);
    p.drawText(calibRect, Qt::AlignLeft | Qt::AlignVCenter, m_calibText);

    // ================= INDICATOR BOX =================
    p.setPen(Qt::darkGray);
    p.drawRect(indRect);

    const int x = cent2X(m_rec.MidiCents);

    if (m_rec.MidiKey > 0)
    {
        if (m_rec.MidiCents == 0)
        {
            p.fillRect(x-3, indRect.top()+1, 7, indRect.height()-2, QColor(0x44,0xff,0x44));
            p.fillRect(x-1, indRect.top()+1, 3, indRect.height()-2, Qt::yellow);
        }
        else
        {
            const QColor c = (m_rec.MidiCents < 0)
            ? QColor(0xaa,0xaa,0)
            : QColor(0xff,0x22,0x22);
            QRect diffRect(QPoint(xc, indRect.top()+1),
                           QPoint(x, indRect.bottom()-1));

            p.fillRect(diffRect.normalized(), c);
            p.fillRect(x-1, indRect.top()+1, 3, indRect.height()-2, Qt::yellow);
        }

        // ================= FREQ TEXT =================
        p.setFont(m_smallFont);
        p.setPen(Qt::white);
        p.drawText(freqRect, Qt::AlignCenter, m_pitchText);

        // ================= NOTE =================
        p.setFont(m_bigFont);
        p.drawText(noteRect, Qt::AlignCenter, m_noteText);
    }
}