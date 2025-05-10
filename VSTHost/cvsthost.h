#ifndef CVSTHOST_H
#define CVSTHOST_H

#include "idevice.h"

#define VSTPLUGINCLASS dynamic_cast<CVSTHostClass*>(FORMFUNC(CVSTForm)->plugIn)

class CVSTHost : public IDevice
{
public:
    CVSTHost();
    ~CVSTHost();
    void init(const int Index, QWidget* MainWindow);
    void pause();
    void execute(const bool Show);
    void serializeCustom(QDomLiteElement* xml) const;
    void unserializeCustom(const QDomLiteElement* xml);
    const QString currentBankPresetName(const short channel=-1) const;
    const QStringList presetNames(const int bank=0) const;
    void setCurrentBankPreset(const int index);
    long currentBankPreset(const short channel=-1) const;
    void parseEvent(CMIDIEvent* Event);
    const QPixmap* picture() const;
private:
    enum JackNames
    {jnIn,jnMIDIIn,jnOut};
    enum ParameterNames
    {pnVolume,pnMIDIChannel,pnTranspose,pnPatchChange};
    float VolFactor;
    int OldBuffers;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    void process();
};

#endif // CVSTHOST_H
