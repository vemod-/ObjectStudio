#ifndef CSCOPECONTROL_H
#define CSCOPECONTROL_H

#include "qcanvas.h"
#include "cpresets.h"
#include "../PitchTracker/cpitchdetect.h"
#include "csyncbuffer.h"

namespace Ui {
class CScopeControl;
}

class CScopeControl : public QCanvas, protected IPresetRef
{
    Q_OBJECT

public:
    explicit CScopeControl(QWidget *parent = nullptr);
    ~CScopeControl();
    void SetVol(int Vol);
    void SetFreq(float Freq);
    void SetDetectPitch(bool v);
    void SetRate(int r);
    void drawScope(float* Buffer);
    void process(float* data, uint size);
    void processVoltage(float val, uint size);
protected:
    void resizeEvent(QResizeEvent *event);
private:
    Ui::CScopeControl *ui;
    void processBuffer();
    void inline CalcHeight();
    int inline ScaleY(float Y);

    int m_Amplitude;
    int MiddleY;
    float CurrentHeight;
    int PixWidth;
    int PixHeight;
    int ImgHeight;
    double m_Frequency;
    double DisplayFactor;
    QRecursiveMutex mutex;
    double WindowBufferSize;
    CPitchDetect PD;
    bool m_DetectPitch;
    QTimer updateTimer;
    CSyncBuffer SB;
private slots:
    void timerUpdate();
};

#endif // CSCOPECONTROL_H
