#include "cequalizerform.h"
#include "ui_cequalizerform.h"
#include <QPainter>

CEqualizerForm::CEqualizerForm(IDevice* Device, QWidget *parent) :
    CSoftSynthsForm(Device,true,parent),
    ui(new Ui::CEqualizerForm)
{
    ui->setupUi(this);
    for (int i=0;i<8;i++)
    {
        frames.append(findChild<CEqualizerFrame*>("frame_"+QString::number(i)));
    }
    connect(ui->GraphFrame,&CEqualizerGraph::Changed,this,&CEqualizerForm::Draw);
    Canvas=ui->GraphFrame;
    m_TimerID=startTimer(50);
}

CEqualizerForm::~CEqualizerForm()
{
    killTimer(m_TimerID);
    m_TimerID=0;
    delete ui;
}

void CEqualizerForm::Init()
{
    ui->frame_0->init(EQUALIZERCLASS,0,40,280,100);
    ui->frame_1->init(EQUALIZERCLASS,1,100,500,200);
    ui->frame_2->init(EQUALIZERCLASS,2,200,1000,400);
    ui->frame_3->init(EQUALIZERCLASS,3,400,2800,1000);
    ui->frame_4->init(EQUALIZERCLASS,4,1000,5000,3000);
    ui->frame_5->init(EQUALIZERCLASS,5,3000,9000,6000);
    ui->frame_6->init(EQUALIZERCLASS,6,6000,18000,12000);
    ui->frame_7->init(EQUALIZERCLASS,7,10000,20000,15000);
}

void CEqualizerForm::serializeCustom(QDomLiteElement* xml) const
{
    for (int i=0;i<8;i++)
    {
        frames[i]->serialize(xml->appendChild("Band" + QString::number(i+1))->appendChild("Band"));
    }
}

void CEqualizerForm::unserializeCustom(const QDomLiteElement* xml)
{
    if (!xml) return;
    for (int i=0;i<8;i++)
    {
        if (const QDomLiteElement* Channel = xml->elementByTag("Band" + QString::number(i+1)))
        {
            frames[i]->unserialize(Channel->elementByTag("Band"));
        }
    }
}

void CEqualizerForm::Draw()
{
    DrawBg();
    DrawGraph();
}

void CEqualizerForm::DrawBg()
{
    Canvas->clearGradient();
    Canvas->setBrush(QBrush(Qt::NoBrush));
    Canvas->setPen(Qt::black);
    Canvas->setLayerFontSize(10);
    const QFontMetrics fm(Canvas->layerFont());
    int P=((freq2MIDIkeyf(100.f)-23)*ui->GraphFrame->width()*1.56f)/(200.f-22.f);
    Canvas->drawLine(P,ui->GraphFrame->height(),P,ui->GraphFrame->height()-5);
    Canvas->drawText(P-(fm.horizontalAdvance("100Hz")/2),ui->GraphFrame->height()-14,"100Hz");
    P=((freq2MIDIkeyf(1000)-23)*ui->GraphFrame->width()*1.56f)/(200.f-22.f);
    Canvas->drawLine(P,ui->GraphFrame->height(),P,ui->GraphFrame->height()-5);
    Canvas->drawText(P-(fm.horizontalAdvance("1000Hz")/2),ui->GraphFrame->height()-14,"1000Hz");
    P=((freq2MIDIkeyf(10000)-23)*ui->GraphFrame->width()*1.56f)/(200.f-22.f);
    Canvas->drawLine(P,ui->GraphFrame->height(),P,ui->GraphFrame->height()-5);
    Canvas->drawText(P-(fm.horizontalAdvance("10kHz")/2),ui->GraphFrame->height()-14,"10kHz");
    P=((freq2MIDIkeyf(20000)-23)*ui->GraphFrame->width()*1.56f)/(200.f-22.f);
    Canvas->drawLine(P,ui->GraphFrame->height(),P,ui->GraphFrame->height()-5);
    Canvas->drawText(P-(fm.horizontalAdvance("20kHz")/2),ui->GraphFrame->height()-14,"20kHz");
}

void CEqualizerForm::DrawGraph()
{
    QCanvasLayer* CanvasLayer=Canvas->canvasLayers[0];
    CanvasLayer->clearTransparent();
    CanvasLayer->setPen(Qt::red);
    QPoint LastPoint(-1,40);
    for (int x = 22;x < 200;x++)
    {
        const auto F=uint(MIDIkey2Freqf(x));
        float WPos=0;
        float Max=0;

        CBiquad filters[8];
        filters[0].lsSetParams(EQUALIZERCLASS->Freq[0], EQUALIZERCLASS->Level[0], 1, presets.SampleRate);
        for (int i=1; i<7; i++)
        {
            filters[i].eqSetParams(EQUALIZERCLASS->Freq[i], EQUALIZERCLASS->Level[i], BWIDTH, presets.SampleRate);
        }
        filters[7].hsSetParams(EQUALIZERCLASS->Freq[7], EQUALIZERCLASS->Level[7], 1, presets.SampleRate);
        for (uint i=0;i<presets.DoubleRate/F;i++)
        {
            WPos+=F;
            while (WPos >= presets.SampleRate) WPos-=presets.SampleRate;
            float samp=W.getNext(uint(WPos),CWaveBank::Sine);
            for (int j=0;j<8;j++) if (!isZero(EQUALIZERCLASS->Level[j])) samp = filters[j].run(samp);
            Max=qMax<float>(qAbs<float>(samp),Max);
        }
        const auto X1=int(((x-23)*ui->GraphFrame->width()*1.56)/(200.0-22.0));
        const QPoint EndPoint(X1,40+int((1.f-Max)*30.f));
        CanvasLayer->drawLine(LastPoint,EndPoint);
        LastPoint=EndPoint;
        if (X1>ui->GraphFrame->width()) break;
    }
    ui->GraphFrame->update();
}

void CEqualizerForm::Reset()
{
    for (int i=0;i<8;i++)
    {
        PeakVal[i]=0;
        frames.at(i)->reset();
    }
}

void CEqualizerForm::Peak()
{
    for (int i=0;i<8;i++)
    {
        frames.at(i)->peak(PeakVal[i]);
    }
}

void CEqualizerForm::timerEvent(QTimerEvent *)
{
    if (!m_TimerID) return;
    if (isVisible())
    {
        for (int i=0;i<8;i++)
        {
            frames.at(i)->peak(PeakVal[i]);
            PeakVal[i]=0;
        }
    }
}
