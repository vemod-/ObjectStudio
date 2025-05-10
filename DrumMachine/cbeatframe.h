#ifndef CBEATFRAME_H
#define CBEATFRAME_H

#include <QFrame>
#include "sequenserclasses.h"

namespace Ui {
class CBeatFrame;
}

class CBeatFrame : public QFrame
{
    Q_OBJECT

public:
    explicit CBeatFrame(QWidget *parent = 0);
    ~CBeatFrame();
    void Init(BeatType* Beat,int Index,int SoundIndex,bool HideLength,bool HideVolume,bool HidePitch);
    void Flash();
private:
    Ui::CBeatFrame *ui;
    void UpdateBeat();	// User declarations
    BeatType* m_Beat;
    int m_SoundIndex;
    int m_TimerID;
private slots:
    void LenChanged(int Value);
    void PitchChanged(int Value);
    void VolChanged(int Value);
    void timerStart();
protected:
    void timerEvent(QTimerEvent *);
signals:
    void flashed();
};

#endif // CBEATFRAME_H
