#ifndef CSF2DEVICE_H
#define CSF2DEVICE_H

#include "csf2generator.h"
#include "csounddevice.h"
#include "cfileparameter.h"

namespace SF2Device
{
const int sf2voices=64;
}

class CSF2Device : public ISoundDevice, public IFileLoader
{
public:
    CSF2Device();
    ~CSF2Device();
    void noteOn(const short channelMode, const short pitch, const short velocity);
    void noteOff(const short channelMode, const short pitch);
    void aftertouch(const short channelMode, const short pitch, const short value);
    void patch(const short channelMode, const short value);
    void controller(const short channelMode, const short controller, const short value);
    float* getNext(const int voice);
    short voiceChannel(const int voice) const;
    int voiceCount() const;
    void reset();
    const QString bankPresetName(const int program) const;
    int bankPresetNumber(const int bank, const int preset) const;
    int banknumber(const int program) const;
    int presetnumber(const int program) const;
    int currentPreset(const short channelMode) const;
    int currentBank(const short channelMode) const;
    int currentBankPreset(const short channelMode) const;
    void setBankPreset(const int program);
    void setBankPreset(const int bank, const int preset);
    bool loadFile(const QString& filename);
    void allNotesOff();
    const QString bankCaption(const int bank) const;
    const QString presetCaption(const int bank, const int preset) const;
    const QStringList bankCaptions() const;
    const QStringList presetCaptions(const int bank) const;
    void setTune(const float tune=440);
private:
    short lastChannel;
    int drumBank;
    CSF2Generator SF2Generator[SF2Device::sf2voices];
    void findDrumBank();
    bool loaded;
    QRecursiveMutex mutex;
};


#endif // CSF2DEVICE_H
