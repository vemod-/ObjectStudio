#include "cscopecontrol.h"
#include "ui_cscopecontrol.h"

CScopeControl::CScopeControl(QWidget *parent) :
    QCanvas(parent,2),
    ui(new Ui::CScopeControl),
    PD(presets.SampleRate),
    SB(presets.DoubleRate+1000)
{
    ui->setupUi(this);
    m_Frequency = 440;
    m_Amplitude = 100;
    WindowBufferSize=presets.ModulationRate;
    m_DetectPitch=false;
    PD.setDetectLevelThreshold(0);
    startTimer(20);
    connect(&updateTimer,&QTimer::timeout,this,&CScopeControl::timerUpdate);
    updateTimer.start(20);
}

CScopeControl::~CScopeControl()
{
    delete ui;
}

void inline CScopeControl::CalcHeight()
{
    CurrentHeight=m_Amplitude*(MiddleY)*0.01f*0.625f;
}

int inline CScopeControl::ScaleY(float Y)
{
    QMutexLocker locker(&mutex);
    return MiddleY-(Y*CurrentHeight);
}

void CScopeControl::process(float* data, uint size)
{
    QMutexLocker locker(&mutex);
    if (data)
    {
        SB.write(data,size);
        if (m_DetectPitch) PD.ProcessBuffer(data,size);
    }
    else
    {
        SB.write(nullptr,size);
    }
}

void CScopeControl::processVoltage(float val, uint size)
{
    QMutexLocker locker(&mutex);
    for (uint i=0;i<size;i++) SB.write(&val,1);
}

void CScopeControl::timerUpdate()
{
    QMutexLocker locker(&mutex);
    if (!SB.isAvail()) return;
    if (m_DetectPitch)
    {
        CPitchDetect::PitchRecord r=PD.CurrentPitchRecord();
        const double f=r.Pitch;
        if ((f > 0) && (f < presets.HalfRate))
        {
            if (!closeEnough(f,m_Frequency))
            {
                qDebug() << f;
                SetFreq(f);
            }
        }
    }
    drawScope(SB.data());
}

void CScopeControl::drawScope(float* Buffer)
{
    QMutexLocker locker(&mutex);
    setUpdatesEnabled(false);
    QCanvasLayer* CanvasLayer=canvasLayers[0];
    CanvasLayer->eraseTransparent(0,0,PixWidth,ImgHeight);
    CanvasLayer->moveTo(0,ScaleY(Buffer[0]));
    double ScopeCounter=DisplayFactor;
    for (uint i=1;i<WindowBufferSize+1;i++,ScopeCounter+=DisplayFactor)
    {
        CanvasLayer->lineTo(ScopeCounter,ScaleY(Buffer[i]));
    }
    setUpdatesEnabled(true);
}

void CScopeControl::resizeEvent(QResizeEvent *event)
{
    QMutexLocker locker(&mutex);
    QCanvas::resizeEvent(event);
    PixWidth=width();
    ImgHeight=height();
    PixHeight=ImgHeight-24;
    MiddleY = PixHeight * 0.5;
    CalcHeight();
    SetVol(m_Amplitude);
    SetFreq(m_Frequency);
    clearGradient();
    canvasLayers[0]->clearTransparent();
    canvasLayers[0]->setPen(QPen(Qt::yellow,3));
    canvasLayers[1]->clearTransparent();
    canvasLayers[1]->setPen(QPen(Qt::darkGray,1));
    double x = 0;
    for (int i = 0; x < PixWidth - 2; i++)
    {
        x = (i * ((PixWidth-4) / 12.0)) + 2;
        canvasLayers[1]->drawLine(x,2,x,PixHeight - 2);
    }
    canvasLayers[1]->drawLine(PixWidth - 2,2,PixWidth - 2,PixHeight - 2);
    const int UpperCenter=(0.25*(PixHeight-4))+2;
    const int LowerCenter=(0.75*(PixHeight-4))+2;
    x = 0;
    for (int i = 0; x < PixWidth - 2; i++)
    {
        x = (i * ((PixWidth-4)/60.0)) + 2;
        canvasLayers[1]->drawLine(x,MiddleY-3,x,MiddleY+3);
        canvasLayers[1]->drawLine(x,UpperCenter-2,x,UpperCenter+2);
        canvasLayers[1]->drawLine(x,LowerCenter-2,x,LowerCenter+2);
    }
    double y = 0;
    for (int i = 0; y < PixHeight - 2; i++)
    {
        y = (i * ((PixHeight-4)/8.0)) + 2;
        canvasLayers[1]->drawLine(2,y,PixWidth - 2,y);
    }
    canvasLayers[1]->drawLine(2,PixHeight-2,PixWidth-2,PixHeight-2);
    const int MiddleX=PixWidth*0.5;
    y = 0;
    for (int i = 0; y < PixHeight - 2; i++)
    {
        y = (i * ((PixHeight-4) / 40.0)) + 2;
        canvasLayers[1]->drawLine(MiddleX-3,y,MiddleX+3,y);
    }
    const int UpperZero=(0.1875*(PixHeight-4))+2;
    const int LowerZero=(0.8125*(PixHeight-4))+2;
    x = 0;
    for (int i = 1; x < PixWidth - 2; i++)
    {
        x = (i* ((PixWidth-4) / 60.0)) + 2;
        canvasLayers[1]->drawLine(x-1,UpperZero,x+1,UpperZero);
        canvasLayers[1]->drawLine(x-1,LowerZero,x+1,LowerZero);
    }
}

void CScopeControl::SetVol(int Vol)
{
    QMutexLocker locker(&mutex);
    m_Amplitude=Vol;
    CalcHeight();
    ui->VolLabel->setText("A "+QString::number(Vol)+" Percent");
}

void CScopeControl::SetFreq(float Freq)
{
    QMutexLocker locker(&mutex);
    m_Frequency=Freq;
    QString Time="T " + QString::number((1.0/m_Frequency)*1000,'f',2) + " US  ";
    QString Frequency="F " +  QString::number(m_Frequency,'f',2) + " HZ";
    ui->FreqLabel->setText(Time + Frequency);

    DisplayFactor = (double)PixWidth/((double)presets.SampleRate/m_Frequency);
    WindowBufferSize=(double)presets.SampleRate/m_Frequency;
    SB.setReadSize(WindowBufferSize);
    qDebug() << WindowBufferSize << presets.SampleRate << m_Frequency;
}

void CScopeControl::SetDetectPitch(bool v)
{
    m_DetectPitch=v;
}

void CScopeControl::SetRate(int r)
{
    QMutexLocker locker(&mutex);
    updateTimer.start(r);
}
