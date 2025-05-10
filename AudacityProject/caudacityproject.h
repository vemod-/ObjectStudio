#ifndef CAUDACITYPROJECT_H
#define CAUDACITYPROJECT_H

#include "softsynthsclasses.h"
#include "cwavegenerator.h"

class AudacityBlock : protected IPresetRef
{
private:
    uint Channel;
    ulong AliasStart;
    int Rate;
    CWaveGenerator wa;
public:
    ulong Start;
    AudacityBlock();
    float* GetNext();
    void Reset();
    bool Init(const QString& Filename, ulong StartPtr, uint Channels, ulong AliasPointer,int RateOverride);
    ulong size() { return wa.size(); }
    inline ulong end() { return size()+Start; }
    void skip(ulong samples);
    inline uint channels() { return wa.channels(); }
};

class AudacityClip : protected IPresetRef
{
private:
    ulong Counter;
    QList<AudacityBlock*> Blocks;
    CMonoBuffer Buffer;
    int BlockIndex;
    uint BufferPointer;
    CMonoBuffer auBuffer;
public:
    AudacityClip();
    ~AudacityClip();
    ldouble Offset;
    void Reset();
    void AddBlock(const QString& Filename,ulong Start,int RateOverride);
    void AddAliasBlock(const QString& Filename,ulong Start,uint Channel,ulong AliasStart);
    void loadClip(const QDomLiteElement* xml, const QString& ProjectPath,int RateOverride);
    void loadSequence(const QDomLiteElement* xml, const QString& ProjectPath,int RateOverride);
    float* GetNext();
    ulong milliSeconds();
    void skip(const ulong64 samples);
};

class AudacityTrack : protected IPresetRef
{
private:
    ldouble Time;
    QList<AudacityClip*> Clips;
public:
    QString Name;
    int Channel;
    bool Linked;
    ldouble Offset;
    bool Mute;
    bool Solo;
    int Rate;
    float Gain;
    float Pan;
    int ClipIndex;
    bool Playing;
    float FactorL;
    float FactorR;
    AudacityTrack();
    ~AudacityTrack();
    void Reset();
    void loadTrack(const QDomLiteElement* xml, const QString& ProjectPath);
    float* GetNext();
    ulong milliSeconds();
    void skip(const ulong64 samples);
};

class CAudacityProject : public IDevice, public IFileLoader
{
private:
    enum JackNames
    {jnOut};
    enum ParameterNames
    {pnVolume};
    float ModFactor;
    ldouble Time;
    ulong m_MilliSeconds;
    QList<AudacityTrack*> Tracks;
    //bool Playing;
    bool Loading;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    bool loadFile(const QString& ProjectFile);
    void process();
public:
    CAudacityProject();
    ~CAudacityProject();
    void execute(const bool Show);
    void init(const int Index, QWidget* MainWindow);
    void play(const bool FromStart);
    //void pause();
    ulong milliSeconds() const;
    ulong64 samples() const;
    void skip(const ulong64 samples);
    CAudioBuffer* getNextA(const int ProcIndex);
};

#endif // CAUDACITYPROJECT_H
