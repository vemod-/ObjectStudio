#ifndef CAUDIOUNITHOST_H
#define CAUDIOUNITHOST_H

#include "caudiounitclass.h"
#include "idevice.h"

#define AUPLUGINCLASS static_cast<CAudioUnitClass*>(FORMFUNC(CVSTForm)->plugIn)

class CAudioUnitHost : public IDevice
{
public:
    CAudioUnitHost();
    ~CAudioUnitHost();
    void init(const int Index, QWidget* MainWindow) override;
    void play(const bool fromStart) override;
    void pause() override;
    void execute(const bool Show) override;
    void serializeCustom(QDomLiteElement* xml) const override;
    void unserializeCustom(const QDomLiteElement* xml) override;
    const QString filename() const override;
    const QString currentBankPresetName(const short channel=-1) const override;
    const QStringList presetNames(const int bank=0) const override;
    void setCurrentBankPreset(const int index) override;
    long currentBankPreset(const short channel=-1) const override;
    void parseEvent(const CMIDIEvent* Event);
    const QPixmap* picture() const override;
private:
    enum JackNames
    {jnIn,jnMIDIIn,jnOut};
    enum ParameterNames
    {pnVolume,pnMIDIChannel,pnTranspose,pnPatchChange};
    float VolFactor;
    void inline updateDeviceParameter(const CParameter* p = nullptr) override;
    void process() override;
    CStereoBuffer m_StereoBuffer;
    CMonoBuffer m_MonoBuffer;
};

#endif // CAUDIOUNITHOST_H
