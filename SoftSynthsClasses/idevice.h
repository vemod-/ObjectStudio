#ifndef IDEVICE_H
#define IDEVICE_H

#include "ijack.h"
#include "csoftsynthsform.h"
#include "ihost.h"
#include "cpresets.h"
#include <QFileDialog>
#include <QPixmap>

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
    virtual ~IDevice()
    {
        if (m_Form != NULL)
        {
            m_Form->hide();
            try
            {
                delete m_Form;
                qDebug() << m_DeviceID << "Form Deleted";
            }
            catch (...)
            {}
            m_Form=NULL;
        }
        foreach (IJack* j,m_Jacks) delete j;
        //qDeleteAll(m_Jacks);
        qDebug() << "Exit " + m_DeviceID;
    }
    inline const ParameterType Parameter(const int Index) const { return m_Parameters.at(Index); }
    inline int ParameterIndex(const QString& Name) const
    {
        for (int i=0;i<m_ParameterCount;i++) if (m_Parameters.at(i).Name.compare(Name,Qt::CaseInsensitive)==0) return i;
        return -1;
    }
    inline int GetParameterValue(const int Index) const { return m_ParameterValues[Index]; }
    inline int GetParameterValue(const QString& Name) const
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
    inline int ParameterCount(void) const { return m_ParameterCount; }
    void SetHost(IHost* Host) { m_Host=Host; }
    virtual const QString OpenFile(const QString& Filter)
    {
        QFileDialog d((QWidget*)m_MainWindow);
        d.setFileMode(QFileDialog::ExistingFile);
        d.setNameFilter(Filter);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        d.setDirectory(QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
#endif
        if (!m_FileName.isEmpty()) d.selectFile(m_FileName);
        if (d.exec()!=QDialog::Accepted) return QString();
        return d.selectedFiles().first();
    }
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
    IJack* GetJack(const int Index) { return m_Jacks.at(Index); }
    IJack* GetJack(const IJack::Directions Direction, const IJack::AttachModes Attachmode)
    {
        foreach(IJack* j,m_Jacks)
        {
            if (j->Direction==Direction)
            {
                if (j->AttachMode & Attachmode) return j;
            }
        }
        return NULL;
    }
    int JackCount(void) const { return m_Jacks.size(); }
    int Index(void) const { return m_Index; }
    const QString Name(void) const { return m_Name; }
    virtual const void* Picture(void) const {
        if (m_Form)
        {
            QPixmap* p=new QPixmap(m_Form->grab(m_Form->rect()));
            return (void*)p;
        }
        return NULL;
    }
    virtual const QString FileName(void) { return m_FileName; }
    const QString DeviceID(void) const { return m_DeviceID; }

    virtual float* GetNextA(const int ProcIndex) {
        if (m_Process)
        {
            m_Process=false;
            Process();
        }
        return AudioBuffers.at(ProcIndex)->Buffer;
    }
    virtual void Init(const int Index, void* MainWindow)
    {
        m_Index=Index;
        m_MainWindow=MainWindow;
        if (m_Index==0) m_DeviceID=m_Name;
        else m_DeviceID=m_Name + " " + QString::number(m_Index);
        qDebug() << ("Create " + m_DeviceID);
        m_Initialized=true;
        m_Process=false;
    }
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
    bool HasUI() const { return (m_Form != NULL); }
protected:
    bool m_Initialized;
    QString m_DeviceID;
    int m_Index;
    void* m_MainWindow;
    QString m_FileName;
    QString m_Name;
    QList<IJack*> m_Jacks;
    int m_ParameterCount;
    QVector<ParameterType> m_Parameters;
    QVector<int> m_ParameterValues;
    inline float Fetch(const int ProcIndex) { return ((CInJack*)m_Jacks.at(ProcIndex))->GetNext(); }
    inline void* FetchP(const int ProcIndex) { return ((CInJack*)m_Jacks.at(ProcIndex))->GetNextP(); }
    inline float* FetchA(const int ProcIndex) { return ((CInJack*)m_Jacks.at(ProcIndex))->GetNextA(); }
    QVector<CAudioBuffer*> AudioBuffers;
    int m_BufferSize;
    IHost* m_Host;
    PresetsType m_Presets;
    CSoftSynthsForm* m_Form;
    void AddParameter(ParameterType::ParameterTypes Type,const QString& Name,const QString& Unit,int Min,int Max,int DecimalFactor,const QString& ListString,int Value)
    {
        m_Parameters.resize(m_ParameterCount+1);
        m_ParameterValues.resize(m_ParameterCount+1);
        m_Parameters[m_ParameterCount].Type=Type;
        m_Parameters[m_ParameterCount].Name=Name;
        m_Parameters[m_ParameterCount].Unit=Unit;
        m_Parameters[m_ParameterCount].Min=Min;
        m_Parameters[m_ParameterCount].Max=Max;
        m_Parameters[m_ParameterCount].DecimalFactor=DecimalFactor;
        m_Parameters[m_ParameterCount].List=ListString;
        m_ParameterValues[m_ParameterCount]=Value;
        m_ParameterCount++;
    }
    IJack* AddJack(const QString& sName,IJack::AttachModes tAttachMode,IJack::Directions tDirection,int ProcIndex=-1)
    {
        if (tDirection==IJack::In) return (IJack*)AddInJack(sName,tAttachMode);
        return (IJack*)AddOutJack(sName,tAttachMode,ProcIndex);
    }
    COutJack* AddOutJack(const QString& sName,IJack::AttachModes tAttachMode,int ProcIndex=-1)
    {
        if (ProcIndex==-1) ProcIndex=m_Jacks.size();
        COutJack* OJ=new COutJack(sName,m_DeviceID,tAttachMode,IJack::Out,this,ProcIndex);
        if (ProcIndex >= AudioBuffers.size()) AudioBuffers.resize(ProcIndex+1);
        AudioBuffers[ProcIndex]=OJ->AudioBuffer;
        m_Jacks.push_back(OJ);
        return OJ;
    }
    CInJack* AddInJack(const QString& sName,IJack::AttachModes tAttachMode)
    {
        CInJack* IJ=new CInJack(sName,m_DeviceID,tAttachMode,IJack::In,this);
        m_Jacks.push_back(IJ);
        return IJ;
    }
    COutJack* AddJackWaveOut(int ProcIndex, const QString& sName="Out")
    {
        return AddOutJack(sName,IJack::Wave,ProcIndex);
    }
    CInJack* AddJackWaveIn(const QString& sName="In")
    {
        return AddInJack(sName,IJack::Wave);
    }
    void AddJackDualMonoOut(int ProcIndex, const QString& sName="Out")
    {
        AddOutJack(sName+" Left",IJack::Wave,ProcIndex);
        AddOutJack(sName+" Right",IJack::Wave,ProcIndex+1);
    }
    void AddJackDualMonoIn(const QString& sName="In")
    {
        AddInJack(sName+" Left",IJack::Wave);
        AddInJack(sName+" Right",IJack::Wave);
    }
    COutJack* AddJackStereoOut(int ProcIndex, const QString& sName="Out")
    {
        return AddOutJack(sName,IJack::Stereo,ProcIndex);
    }
    CInJack* AddJackStereoIn(const QString& sName="In")
    {
        return AddInJack(sName,IJack::Stereo);
    }
    CInJack* AddJackMIDIIn(const QString& sName="MIDI In")
    {
        return AddInJack(sName,IJack::MIDI);
    }
    COutJack* AddJackMIDIOut(int ProcIndex, const QString& Name="MIDI Out")
    {
        return AddOutJack(Name,IJack::MIDI,ProcIndex);
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
