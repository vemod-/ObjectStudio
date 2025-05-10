#ifndef CSF2PLAYER_H
#define CSF2PLAYER_H

#include "idevice.h"
#include "csf2device.h"

class CSF2Player : public IDevice
{
public:
    enum ParameterNames
    {pnMIDIChannel,pnTranspose,pnPatchChange,pnTune,pnVolume};
    CSF2Player();
    void init(const int Index, QWidget* MainWindow);
    void pause();
    void play(const bool FromStart);
    void setCurrentBankPreset(const int Program);
    const QString currentBankPresetName(const short channel) const;
    const QStringList bankNames() const;
    const QStringList presetNames(const int bank=0) const;
    long currentBankPreset(const short channel=-1) const;
    int bankPresetNumber(const int bank, const int preset) const;
    CSF2Device SF2Device;
private:
    enum JackNames
    {jnIn,jnOut};
    int LastTrigger;
    float LastFreq;
    float VolumeFactor;
    void process();
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    bool inline matchChannel(int message,int command);
};

#endif // CSF2PLAYER_H
