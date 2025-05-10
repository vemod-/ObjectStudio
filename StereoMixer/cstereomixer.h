#ifndef STEREOMIXER_H
#define STEREOMIXER_H

#include "idevice.h"
#include "../Preamp/c3bandfilter.h"
#include "../Chorus/biquad.h"
#include "../EffectRack/ceffectrack.h"

class CStereoMixerChannel : protected IPresetRef
{
public:
    CStereoMixerChannel(const int sends=3);
    ~CStereoMixerChannel();
    void mixChannel(CStereoBuffer& Signal, CStereoBuffer* Out, std::vector<CStereoBuffer*>& Send, const bool First, CStereoBuffer& WorkBuffer);
    float Level;
    float PanL;
    float PanR;
    bool Mute;
    bool EffectMute;
    float* Effect;
    float PeakL;
    float PeakR;
    uint sendCount;
    bool EQ;
    C3BandFilter f3L;
    C3BandFilter f3R;
    CBiquad hpL;
    CBiquad hpR;
    float Gain;
    bool LoCut;
    CEffectRack EffectRack;
};

class CStereoMixer : public IDevice
{
public:
    enum JackNames
    {jnOut,jnSend};
    int jnIn;
    int jnReturn;
    CStereoMixer(const uint channelCount=12, const uint sendCount=3);
    ~CStereoMixer();
    void init(const int Index, QWidget* MainWindow);
    void addEffectRacksToDeviceList(CDeviceList* dl, QWidget* mainWindow);
    void removerEffectRacksFromDeviceList(CDeviceList* dl);
    CAudioBuffer* getNextA(const int ProcIndex);
    void play(const bool FromStart);
    void connectionChanged();

    CStereoMixerChannel** channels;
    int SoloChannel;
    float MasterLeft;
    float MasterRight;
    float PeakL;
    float PeakR;
    float* Sends;
    inline uint sendCount() const { return m_SendCount; }
    inline uint channelCount() const { return m_ChannelCount; }
    inline bool disabled() const { return m_Disabled; }
    void setDisabled(bool Disabled) { m_Disabled = Disabled; }
    std::vector<COutJack*> SendJacks;
    std::vector<CInJack*> ReturnJacks;
    std::vector<CStereoBuffer*> SendBuffers;
    CStereoBuffer* OutBuffer;
private:
    uint m_SendCount;
    uint m_ChannelCount;
    bool m_Disabled;
    float MixFactor;
    void process();
    CStereoBuffer WorkBuffer;
    CStereoBuffer** Signal;
    std::vector<uint> OrigChannel;
};

#endif // STEREOMIXER_H
