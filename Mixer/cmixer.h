#ifndef CMIXER_H
#define CMIXER_H

#include "idevice.h"

namespace Mixer
{
const int mixerchannels=8;
}


class CMixerChannel
{
public:
    CMixerChannel() {
        Level=1;
        Effect=0;
        PanL=1;
        PanR=1;
        Mute=false;
        EffectMute=false;
        Peak=0;
    }
    inline float volL() { return Level*PanL; }
    inline float volR() { return Level*PanR; }
    inline float effVolL() { return Level*PanL*Effect; }
    inline float effVolR() { return Level*PanR*Effect; }
    void mixChannel(CMonoBuffer& Signal, CStereoBuffer* Out, CStereoBuffer* Send, const bool First) {
        if (First)
        {
            if (Level != 0)
            {
                Out->fromMono(Signal.data(),volL(),volR());
                if (!EffectMute)
                    Send->fromMono(Signal.data(),effVolL(),effVolR());
                else
                    Send->zeroBuffer();
                Signal.peakBuffer(&Peak,Level);
            }
            else
            {
                Out->zeroBuffer();
                Send->zeroBuffer();
                Peak=0;
            }
        }
        else
        {
            if (Level != 0)
            {
                Out->addMono(Signal.data(),volL(),volR());
                if (!EffectMute) Send->addMono(Signal.data(),effVolL(),effVolR());
                Signal.peakBuffer(&Peak,Level);
            }
            else
            {
                Peak=0;
            }
        }
    }
    float Level;
    float Effect;
    float PanL;
    float PanR;
    float Peak;
    bool Mute;
    bool EffectMute;
};

class CMixer : public IDevice
{
public:
    CMixer();
    void init(const int Index, QWidget* MainWindow);
    void play(const bool FromStart);
    void connectionChanged();
    CMixerChannel Channel[Mixer::mixerchannels];
    int SoloChannel;
    float MasterLeft;
    float MasterRight;
    float PeakL;
    float PeakR;
private:
    enum JackNames
    {jnReturn,jnOut,jnSend,jnIn};
    float MixFactor;
    void process();
    CMonoBuffer* Signal[Mixer::mixerchannels]={nullptr};
    std::vector<int> OrigChannel;
    uint prevChannel;
};

#endif // CMIXER_H
