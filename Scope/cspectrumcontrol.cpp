#include "cspectrumcontrol.h"
#include "ui_cspectrumcontrol.h"
#include <array>

//#define MaxBuffers 8192

//class Fft;
//class SampleIter;
/*
uint ColorScale [] =
{
    0x800000,	//RGB(0, 0, 128), // dk blue
    0xFF0000,//RGB(0, 0, 255), // lt blue
    0xFF00,//RGB(0, 255, 0), // lt green
    0xFF00,//RGB(0, 255, 0),
    0xFFFF,//RGB(255, 255, 0), // lt yellow
    0xFFFF,//RGB(255, 255, 0),
    0xFFFF,//RGB(255, 255, 0),
    0xFFFF,//RGB(255, 255, 0),
    0x80FF,//RGB(255, 128, 0), // orange
    0x80FF,//RGB(255, 128, 0),
    0x80FF,//RGB(255, 128, 0),
    0x80FF,//RGB(255, 128, 0),
    0x80FF,//RGB(255, 128, 0),
    0x80FF,//RGB(255, 128, 0),
    0x80FF,//RGB(255, 128, 0),
    0x80FF,//RGB(255, 128, 0),
    0xFF//RGB(255, 0, 0) // lt red
};

uint inline MapColor (uint s)
{
    s /= 16;
    if (s >= sizeof (ColorScale))
        s = sizeof (ColorScale) - 1;
    return ColorScale [s];
}
*/
// ViewFreq -------------

//static int SCALE_WIDTH = 30;
//static int NOTCH_1 = 5;
//static int NOTCH_2 = 10;
//static int NOTCH_3 = SCALE_WIDTH;
/*
ViewFreq::ViewFreq():
    _xRecord (1),
    _points2 (0),
    Mode(0)
{
}
*/

CSpectrumControl::CSpectrumControl(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CSpectrumControl),
    m_fft(FFTPoints),
    window(FFTPoints),
    m_CB(FFTPoints*10)
{
    ui->setupUi(this);
    QMutexLocker locker(&mutex);
    for (int i=0;i<=240;i++)
    {
        int hiVal=-i;
        hiVal+=240+360;
        int g = sqrt(i/240.0)*400.0;
        if (g>255) g=255;
        QColor c;
        c.setHsv(hiVal,255,g);
        colorMap.push_back(c);
    }
    SetBounds(width(),height(),26);
    SetUpdateRate(50);
}

CSpectrumControl::~CSpectrumControl()
{
    delete ui;
}

void CSpectrumControl::SetBounds(int Width,int Height,int ScaleWidth)
{
    QMutexLocker locker(&mutex);
    BackImage=QImage(Width-ScaleWidth,Height,QImage::Format_RGB32);
    ScaleImage=QImage(ScaleWidth,Height,QImage::Format_RGB32);
    _width = Width-ScaleWidth;
    _height = Height;
    WorkImage=QImage(1,_height,QImage::Format_RGB32);
    pointMap.clear();
    pointMap.resize(freq2point(Range));
    for (uint i=0; i<pointMap.size();i++) pointMap[i]=freq2y(point2freq(i));
    peakMap.clear();
    peakMap.resize(freq2point(Range),0);
    avgMap.clear();
    avgMap.resize(freq2point(Range),0);
    avgCount = 0;
}

void CSpectrumControl::PaintScale()
{
    static const int notches[11] = {10,20,50,100,200,500,1000,2000,5000,10000,20000};
    static const QString notchtext[11] = {"10Hz","20Hz","50Hz","100Hz","200Hz","500Hz","1kHz","2kHz","5kHz","10kHz","20kHz"};
    //if (_points2 == 0) return;
    QPainter p(&ScaleImage);
    p.fillRect(ScaleImage.rect(),Qt::black);
    const int x0 = ScaleImage.rect().width();
    for (int y=0;y<_height;y++)
    {
        const uint i=(double)(_height-y)*colorMap.size()/_height;
        p.setPen(colorMap[i]);
        p.drawLine(x0-2,y,x0,y);
    }
    p.setPen(Qt::lightGray);
    QFont f(p.font()); f.setPointSizeF(8); p.setFont(f);
    for (int i=0;i<11;i++)
    {
        const int y=freq2y(notches[i]);
        p.drawLine(0,y,x0,y);
        p.drawText(0,y-1,notchtext[i]);
    }
    QPainter(&BackImage).fillRect(BackImage.rect(),Qt::black);
    QPainter(&WorkImage).fillRect(WorkImage.rect(),Qt::black);
}

int CSpectrumControl::freq2y(double freq)
{
    if (ScaleMode==0) return _height - (_height * freq2Cent(freq) / RangeCent);
    return _height - (_height * freq2point(freq) / pointMap.size());
}

double CSpectrumControl::y2freq(int y)
{
    if (ScaleMode==0) return cent2Freq((_height - y) * RangeCent /_height);
    return point2freq((_height - y) * pointMap.size() /_height);
}

double CSpectrumControl::point2freq(uint point)
{
    return double(presets.SampleRate * point) / FFTPoints;
}

uint CSpectrumControl::freq2point(double freq)
{
    return FFTPoints * freq / presets.SampleRate;
}

void CSpectrumControl::Update()
{
    QPainter imgPainter(&BackImage);
    int lasty=-1;
    int hiVal=0;
    int* pm = pointMap.data();
    int* peak = peakMap.data();
    long64* avg = avgMap.data();
    if (Mode>=2)
    {
        if (avgCount == 0) avgCount++;
        BackImage.fill(0);
        const int xwidth=(_width*2)/3;
        const float FFTwidth = (float)xwidth/(float)FFTPoints2;
        QPen pen(Qt::darkGray,1,Qt::DotLine);
        imgPainter.setPen(pen);
        imgPainter.drawLine(xwidth,0,xwidth,_height);
        imgPainter.setPen(Qt::yellow);
        bool sign = false;
        for (uint i = 0; i < pointMap.size(); i++ )
        {
            const int y=pm[i];
            const int s = m_fft.magn(i)*FFTwidth;
            if (y==lasty)
            {
                hiVal=qMax<int>(s,hiVal);
            }
            else
            {
                if (lasty>-1)
                {
                    if (hiVal>0)
                    {
                        avg[i] += hiVal;
                        sign = true;
                        peak[i] = qMax<int>(peak[i],hiVal);
                        if (hiVal>xwidth)
                        {
                            imgPainter.fillRect(0,lasty,xwidth,y-lasty,Qt::green);
                            imgPainter.fillRect(xwidth,lasty,hiVal-xwidth,y-lasty,Qt::red);
                        }
                        else
                        {
                            imgPainter.fillRect(0,lasty,hiVal,y-lasty,Qt::green);
                        }
                    }
                    if (Mode == 3)
                    {
                        if (peak[i] > 0) imgPainter.drawLine(peak[i],lasty,peak[i],y);
                    }
                    else if (Mode == 4)
                    {
                        const int a = avg[i] / avgCount;
                        imgPainter.drawLine(a,lasty,a,y);
                    }
                }
                lasty=y;
                hiVal=s;
            }
            _xRecord=0;
        }
        if (sign) avgCount++;
    }
    else
    {
        const QColor* cm = colorMap.data();
        WorkImage.fill(Qt::black);
        QPainter p(&WorkImage);
        for (uint i = 0; i < pointMap.size(); i++ )
        {
            const int y=pm[i];
            const double v = sqrt(m_fft.magn(i)/FFTPoints2);
            const int s = v*180.0;//*levelFactor;
            if (y==lasty)
            {
                hiVal=qMax<int>(s,hiVal);
            }
            else
            {
                if (lasty>-1)
                {
                    if (hiVal>0)
                    {
                        //if (hiVal>240) hiVal=240;
                        p.setPen(cm[qMin<int>(hiVal,240)]);
                        p.drawLine(0,lasty,0,y);
                    }
                }
                lasty=y;
                hiVal=s;
            }
        }
        if (Mode==0)
        {
            imgPainter.drawImage(QRect(_xRecord,0,Speed,_height),WorkImage,WorkImage.rect());
        }
        else if (Mode==1)
        {
            imgPainter.drawImage(QRect(0,0,_width-Speed,_height),BackImage,QRect(Speed,0,_width-Speed,_height));
            imgPainter.drawImage(QRect(_width-Speed,0,Speed,_height),WorkImage,WorkImage.rect());
            _xRecord=0;
        }
    }
    _xRecord+=Speed;
    if (_xRecord >= _width) _xRecord = 0;
}

void CSpectrumControl::Fake()
{
    QPainter p(&BackImage);
    if (Mode>=2)
    {
        BackImage.fill(Qt::black);
        int xwidth=(_width*2)/3;
        QPen pen(Qt::darkGray,1,Qt::DotLine);
        p.setPen(pen);
        p.drawLine(xwidth,0,xwidth,_height);
        _xRecord=0;
    }
    else if (Mode==1)
    {
        p.drawImage(QRect(0,0,_width-Speed,_height),BackImage,QRect(2,0,_width-Speed,_height));
        p.fillRect(_width-Speed,0,Speed,_height,Qt::black);
        _xRecord=0;
    }
    else if (Mode==0)
    {
        p.fillRect(_xRecord,0,Speed,_height,Qt::black);
    }
    _xRecord += Speed;
    if (_xRecord >= _width) _xRecord =  0;
}

void CSpectrumControl::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    QMutexLocker locker(&mutex);
    SetBounds(width(),height(),26);
    PaintScale();
}

void CSpectrumControl::paintEvent(QPaintEvent* /*event*/)
{
    if (!pointMap.empty())
    {
        QPainter p(this);
        p.drawImage(BackImage.rect().translated(24,0),BackImage);
        p.drawImage(ScaleImage.rect(),ScaleImage);
    }
}

void CSpectrumControl::process(float* data, uint size)
{
    m_CB.write(data,size);
    while (m_CB.isAvail(FFTPoints)) process(m_CB.read(FFTPoints));
}

void CSpectrumControl::process(float* Buffer)
{
    if (!pointMap.empty())
    {
        if (Buffer==nullptr)
        {
            Fake();
        }
        else
        {
            float* Win=window.WinCoeff;
            if (Win)
            {
                m_fft.Forward(Buffer,Win);
            }
            else
            {
                m_fft.Forward(Buffer);
            }
            Update();
        }
    }
}

void CSpectrumControl::SetVol(int Vol)
{
    m_Vol=Vol*0.01f;
}

void CSpectrumControl::SetMode(int m)
{
    QMutexLocker locker(&mutex);
    if (m != Mode)
    {
        Mode=m;
        PaintScale();
        for (uint i = 0; i < peakMap.size(); i++) peakMap[i] = 0;
        for (uint i = 0; i < avgMap.size(); i++) avgMap[i] = 0;
        avgCount = 0;
    }
}

void CSpectrumControl::SetWindow(int w)
{
    QMutexLocker locker(&mutex);
    WindowType=w;
    switch (WindowType)
    {
    case 1:
        window.SetWindow(CSpectralWindow::wtHANNING,0,0,true);
        break;
    case 2:
        window.SetWindow(CSpectralWindow::wtGAUSS,0,0,true);
        break;
    case 3:
        window.SetWindow(CSpectralWindow::wtFLATTOP,0,0,true);
        break;
    default:
        window.SetWindow(CSpectralWindow::wtNONE,0,0,true);
        break;
    }
}

void CSpectrumControl::SetScale(int s)
{
    QMutexLocker locker(&mutex);
    if (ScaleMode != s)
    {
        ScaleMode=s;
        SetBounds(_width+26,_height,26);
        PaintScale();
    }
}

void CSpectrumControl::SetRange(int r)
{
    QMutexLocker locker(&mutex);
    if (Range != r)
    {
        Range=r;
        RangeCent = freq2Cent(Range);
        SetBounds(_width+26,_height,26);
        PaintScale();
    }
}

void CSpectrumControl::SetUpdateRate(int s)
{
    QMutexLocker locker(&mutex);
    s = qBound<int>(1,s,1000);
    if (m_UpdateRate != s)
    {
        m_UpdateRate = s;
        double stepSize = s * presets.SamplesPermSec;
        if (stepSize > FFTPoints) stepSize = FFTPoints;
        if (stepSize < 1) stepSize = 1;
        m_CB.setStepSize(stepSize);
        Speed = qMax<int>(1,qRound(4.0*(stepSize / FFTPoints)));
        _xRecord=0;
        PaintScale();
        qDebug() << stepSize << Speed;
    }
}
