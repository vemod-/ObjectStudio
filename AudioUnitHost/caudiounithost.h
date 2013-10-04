#ifndef CAUDIOUNITHOST_H
#define CAUDIOUNITHOST_H

#include "softsynthsclasses.h"

class CAudioUnitHost : public IDevice
{
    
public:
    CAudioUnitHost();
    ~CAudioUnitHost();
    void Init(const int Index, void *MainWindow);
    void Play(const bool fromStart);
    void Pause();
    void Execute(const bool Show);
    const QString Save();
    void Load(const QString& XML);
    const QString FileName();
    const QString PresetName();
    const QStringList PresetNames();
    void SetProgram(const int index);
    const void* Picture() const;
private:
    enum JackNames
    {jnIn,jnMIDIIn,jnOut};
    enum ParameterNames
    {pnVolume,pnMIDIChannel,pnPatchChange};
    float VolFactor;
    void inline CalcParams();
    void Process();
};

#endif // CAUDIOUNITHOST_H
