#ifndef CVSTHOST_H
#define CVSTHOST_H

#include "softsynthsclasses.h"
//#include "cvsthostclass.h"

class CVSTHost : public IDevice
{
public:
    CVSTHost();
    ~CVSTHost();
    void Init(const int Index,void* MainWindow);
    void Pause();
    //void RaiseForm();
    void Execute(const bool Show);
    //void HideForm();
    const QString Save();
    void Load(const QString& XML);
    const QString FileName();
    const QString PresetName();
    const QStringList PresetNames();
    void SetProgram(const int index);
    void UpdateHost();
private:
    enum JackNames
    {jnIn,jnMIDIIn,jnOut};
    enum ParameterNames
    {pnVolume,pnMIDIChannel,pnPatchChange};
    float VolFactor;
    int OldBuffers;
    void inline CalcParams();
    //TVSTHost* VST;
    int InBufferCount;
    int OutBufferCount;
    void Process();
};

#endif // CVSTHOST_H
