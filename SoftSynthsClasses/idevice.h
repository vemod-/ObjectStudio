#ifndef IDEVICE_H
#define IDEVICE_H

#include "ijack.h"
#include "csoftsynthsform.h"
#include "ihost.h"
#include "cpresets.h"

namespace ParameterList
{
const QString ParameterListSeparator("§");
}

struct ParameterType
{
    enum ParameterTypes
    {Numeric,SelectBox,dB};
    int Min;
    int Max;
    ParameterTypes Type;
    QString Name;
    QString List;
    QString Unit;
    int DecimalFactor;
};

class IDevice : public IDeviceBase
{
public:
    IDevice() : m_Initialized(false), m_DeviceID("No ID"), m_Name("Generic Device Class"), m_ParameterCount(0),
        m_BufferSize(CPresets::Presets.ModulationRate), m_Host(NULL), m_Presets(PresetsType(CPresets::Presets)), m_Form(NULL) {}
    virtual ~IDevice();
    inline const ParameterType Parameter(const int Index) { return m_Parameters[Index]; }
    inline const int ParameterIndex(const QString& Name)
    {
        for (int i=0;i<m_ParameterCount;i++) if (m_Parameters[i].Name.compare(Name,Qt::CaseInsensitive)==0) return i;
        return -1;
    }
    inline const int GetParameterValue(const int Index) { return m_ParameterValues[Index]; }
    inline const int GetParameterValue(const QString& Name)
    {
        int i=ParameterIndex(Name);
        if (i>-1) return GetParameterValue(i);
        return 0;
    }
    virtual void SetParameterValue(const int Index, const int Value)
    {
        if (m_ParameterValues[Index]!=Value)
        {
            m_ParameterValues[Index]=Value;
            CalcParams();
        }
    }
    virtual void SetParameterValue(const QString& Name, const int Value)
    {
        int i=ParameterIndex(Name);
        if (i>-1) SetParameterValue(i,Value);
    }
    inline const int ParameterCount(void) { return m_ParameterCount; }
    void SetHost(IHost* Host) { m_Host=Host; }
    virtual const QString OpenFile(const QString& Filter);
    virtual void RaiseForm() {
        if (m_Form != NULL)
        {
            if (m_Form->isVisible())
            {
                m_Form->raise();
                m_Form->activateWindow();
            }
        }
    }
    virtual void HideForm() { if (m_Form != NULL) m_Form->setVisible(false); }
    IJack* GetJack(const int Index) { return (IJack*)m_Jacks[Index]; }
    IJack* GetJack(const IJack::Directions Direction, const IJack::AttachModes Attachmode)
    {
        for (unsigned int i=0;i<m_Jacks.size();i++)
        {
            if (((IJack*)m_Jacks[i])->Direction==Direction)
            {
                if (((IJack*)m_Jacks[i])->AttachMode & Attachmode) return (IJack*)m_Jacks[i];
            }
        }
        return NULL;
    }
    const int JackCount(void) { return m_Jacks.size(); }
    const int Index(void) { return m_Index; }
    const QString Name(void) { return m_Name; }
    const QString Picture(void) { return QString(); }
    virtual const QString FileName(void) { return m_FileName; }
    const QString DeviceID(void) { return m_DeviceID; }

    virtual float* GetNextA(const int ProcIndex) {
        if (m_Process)
        {
            m_Process=false;
            Process();
        }
        return AudioBuffers[ProcIndex]->Buffer;
    }
    virtual void Init(const int Index, void* MainWindow);
    virtual void Load(const QString& XML) { if (m_Form != NULL) m_Form->Load(XML); }
    virtual const QString Save(void) {
        if (m_Form != NULL) return m_Form->Save();
        return QString();
    }
    virtual void Execute(const bool Show) {
        if (m_Form != NULL)
        {
            if (Show) m_Form->show();
            else m_Form->setVisible(false);
        }
    }
    virtual void Play(const bool /*FromStart*/) {}
    virtual void Pause(void) {}
    virtual void Tick(void) { m_Process=true; }
    virtual void UpdateHost() { if (m_Host != NULL) m_Host->ParameterChange(); }
    virtual void Activate() { if (m_Host != NULL) m_Host->Activate(this); }
    const bool HasUI() { return (m_Form != NULL); }
protected:
    bool m_Initialized;
    QString m_DeviceID;
    int m_Index;
    void* m_MainWindow;
    QString m_FileName;
    QString m_Name;
    CFastPointerList m_Jacks;
    int m_ParameterCount;
    std::vector<ParameterType> m_Parameters;
    std::vector<int> m_ParameterValues;
    inline float Fetch(const int ProcIndex) { return ((CInJack*)m_Jacks[ProcIndex])->GetNext(); }
    inline void* FetchP(const int ProcIndex) { return ((CInJack*)m_Jacks[ProcIndex])->GetNextP(); }
    inline float* FetchA(const int ProcIndex) { return ((CInJack*)m_Jacks[ProcIndex])->GetNextA(); }
    std::vector<CAudioBuffer*> AudioBuffers;
    int m_BufferSize;
    IHost* m_Host;
    PresetsType m_Presets;
    CSoftSynthsForm* m_Form;
    void AddParameter(ParameterType::ParameterTypes Type,const QString& Name,const QString& Unit,int Min,int Max,int DecimalFactor,const QString& ListString,int Value);
    void AddJack(IJack* J,IJack::Directions tDirection);
    IJack* AddJack(const QString& sName,IJack::AttachModes tAttachMode,IJack::Directions tDirection,int ProcIndex=-1);
    COutJack* AddJackWaveOut(int ProcIndex, const QString& sName="Out")
    {
        return (COutJack*)AddJack(sName,IJack::Wave,IJack::Out,ProcIndex);
    }
    CInJack* AddJackWaveIn(const QString& sName="In")
    {
        return (CInJack*)AddJack(sName,IJack::Wave,IJack::In);
    }
    void AddJackDualMonoOut(int ProcIndex, const QString& sName="Out")
    {
        AddJack(sName+" Left",IJack::Wave,IJack::Out,ProcIndex);
        AddJack(sName+" Right",IJack::Wave,IJack::Out,ProcIndex+1);
    }
    void AddJackDualMonoIn(const QString& sName="In")
    {
        AddJack(sName+" Left",IJack::Wave,IJack::In);
        AddJack(sName+" Right",IJack::Wave,IJack::In);
    }
    COutJack* AddJackStereoOut(int ProcIndex, const QString& sName="Out")
    {
        return (COutJack*)AddJack(sName,IJack::Stereo,IJack::Out,ProcIndex);
    }
    CInJack* AddJackStereoIn(const QString& sName="In")
    {
        return (CInJack*)AddJack(sName,IJack::Stereo,IJack::In);
    }
    CInJack* AddJackMIDIIn(const QString& sName="MIDI In")
    {
        return (CInJack*)AddJack(sName,IJack::MIDI,IJack::In);
    }
    COutJack* AddJackMIDIOut(int ProcIndex, const QString& Name="MIDI Out")
    {
        return (COutJack*)AddJack(Name,IJack::MIDI,IJack::Out,ProcIndex);
    }
    void AddParameterVolume(const QString& Name="Volume")
    {
        AddParameter(ParameterType::dB,Name,"dB",0,200,0,"",100);
    }
    void AddParameterMIDIChannel(const QString& Name="MIDI Channel")
    {
        QString s("All");
        for (int i=0;i<16;i++) s+="§"+QString::number(i+1);
        AddParameter(ParameterType::SelectBox,Name,"",0,16,0,s,0);
    }
    void AddParameterTrack(const QString& Name="Track")
    {
        QString s("All");
        for (int i=0;i<64;i++) s+="§"+QString::number(i+1);
        AddParameter(ParameterType::SelectBox,Name,"",0,64,0,s,0);
    }
    void AddParameterTranspose(const QString& Name="Transpose")
    {
        AddParameter(ParameterType::Numeric,Name,"Semitones",-24,24,0,"",0);
    }
    void AddParameterTune(const QString& Name="Tune")
    {
        AddParameter(ParameterType::Numeric,Name,"Hz",43600,44800,100,"",44000);
    }
    void AddParameterPercent(const QString& Name="Modulation", const int Default=0)
    {
        AddParameter(ParameterType::Numeric,Name,"%",0,100,0,"",Default);
    }
    virtual void CalcParams() {}
    virtual void Process() {}
    bool m_Process;
};

typedef IDevice*(*instancefunc)();

#endif // IDEVICE_H
