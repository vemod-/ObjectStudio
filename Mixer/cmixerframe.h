#ifndef CMIXERFRAME_H
#define CMIXERFRAME_H

#include <QFrame>
#include "cmixer.h"

namespace Ui {
    class CMixerFrame;
}

class CMixerFrame : public QFrame
{
    Q_OBJECT

public:
    explicit CMixerFrame(QWidget *parent = nullptr);
    ~CMixerFrame();
    void init(CMixer* MixerClass,int ChannelIndex);
    int Index;
    void serialize(QDomLiteElement* xml) const;
    void unserialize(const QDomLiteElement* xml);
    void peak(float Value);
    void reset();
    void setSolo(bool Value);
    void setSender(const QString& s);
signals:
    void SoloClicked(bool Pressed, int Index);
private:
    Ui::CMixerFrame *ui;
    CMixer* Mixer;
private slots:
    void PanChanged(int Value);
    void EffectChanged(int Value);
    void VolChanged(int Value);
    void MuteButtonClicked(bool Value);
    void SoloButtonClicked(bool Value);
    void BypassButtonClicked(bool Value);
protected:
    void showEvent(QShowEvent *);
};

#endif // CMIXERFRAME_H
