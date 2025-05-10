#ifndef CSPECTRUMCONTROL_H
#define CSPECTRUMCONTROL_H

#include <QFrame>
#include <QPainter>
#include "cspectralwindow.h"
#include "cfastcircularbuffer.h"
#include "cfft.h"
#include "cpresets.h"

namespace Ui {
    class CSpectrumControl;
}

class CSpectrumControl : public QFrame, protected IPresetRef
{
    Q_OBJECT

public:
    explicit CSpectrumControl(QWidget *parent = nullptr);
    ~CSpectrumControl();
    void process(float* Buffer);
    void process(float* data, uint size);
public slots:
    void SetVol(int Vol);
    void SetMode(int m);
    void SetWindow(int w);
    void SetScale(int s);
    void SetRange(int r);
    void SetUpdateRate(int s);
protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
private:
    static const uint FFTPoints=8192;
    static const uint FFTPoints2=FFTPoints/2;
    Ui::CSpectrumControl *ui;
    CFFTtwiddleInterleaved<float> m_fft;
    void Update();
    void Fake();
    void PaintScale();
    void SetBounds(int Width,int Height,int ScaleWidth);
    int Mode = 0;
    int WindowType = 0;
    int ScaleMode = 0;
    int Range = 8000;
    long RangeCent = freq2Cent(Range);
    int Speed = 1;
    int _xRecord = 0;
    int	_width;
    int	_height;
    float m_Vol = 1.f;
    int m_UpdateRate = 0;
    QImage WorkImage;
    QImage BackImage;
    QImage ScaleImage;
    uint freq2point(double freq);
    double point2freq(uint point);
    int freq2y(double freq);
    double y2freq(int y);
    std::vector<int> pointMap;
    std::vector<int> peakMap;
    std::vector<long64> avgMap;
    long64 avgCount;
    CSpectralWindow window;
    std::vector<QColor> colorMap;
    QRecursiveMutex mutex;
    CFastCircularBuffer m_CB;
};

#endif // CSPECTRUMCONTROL_H
