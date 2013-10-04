#include "cdevicecontainer.h"

CDeviceContainer::CDeviceContainer(const QString &Name) : IDevice()
{
    m_Name=Name;
    m_Device=NULL;
    m_Bypass=false;
    ClearDevice();
}

CDeviceContainer::~CDeviceContainer()
{
    ClearDevice();
    if (m_Initialized) qDeleteAll(JacksCreated);
}

void CDeviceContainer::Process()
{
    InBuffer=FetchA(jnIn);
    MIDIBuffer=(CMIDIBuffer*)FetchP(jnMIDIIn);
}

float* CDeviceContainer::GetNextA(const int ProcIndex)
{
    if (m_Bypass)
    {
        return NULL;
        /*
        if (m_Process)
        {
            m_Process=false;
            Process();
            return InBuffer;
        }
        */
    }
    if (m_Device)
    {
        if (ProcIndex==jnInsideIn)
        {
            if (m_Process)
            {
                m_Process=false;
                Process();
            }
            return InBuffer;
        }
        if (ProcIndex==jnOut) return InsideOut->GetNextA();
    }
    return NULL;
}

void* CDeviceContainer::GetNextP(const int /*ProcIndex*/)
{
    if (m_Device)
    {
        if (m_Process)
        {
            m_Process=false;
            Process();
        }
        return MIDIBuffer;
    }
    return NULL;
}

void CDeviceContainer::Tick()
{
    m_Process=true;
    if (m_Device) m_Device->Tick();
}

void CDeviceContainer::Play(const bool FromStart)
{
    if (m_Device) m_Device->Play(FromStart);
}

void CDeviceContainer::Pause()
{
    if (m_Device) m_Device->Pause();
}

void CDeviceContainer::UpdateHost()
{
    if (m_Device) m_Device->UpdateHost();
}

void CDeviceContainer::Activate()
{
    if (m_Device) m_Device->Activate();
}

void CDeviceContainer::Init(const int Index, void *MainWindow)
{
    if (m_Name.isEmpty()) m_Name="DeviceContainer";
    IDevice::Init(Index,MainWindow);
    AddJackStereoIn();
    AddJackMIDIIn();
    AddJackStereoOut(jnOut);
    InsideIn=new COutJack("InsideIn","This",IJack::Stereo,IJack::Out,this,jnInsideIn);
    InsideMIDIIn=new COutJack("InsideMIDIIn","This",IJack::MIDI,IJack::Out,this,jnInsideMIDIIn);
    InsideOut=new CInJack("InsideOut","This",IJack::Stereo,IJack::In,this);
    JacksCreated.push_back(InsideIn);
    JacksCreated.push_back(InsideMIDIIn);
    JacksCreated.push_back(InsideOut);
}

void CDeviceContainer::Load(const QString &XML)
{
    QDomLiteElement Custom("Custom");
    Custom.fromString(XML);
    OpenFile(Custom.attribute("DeviceType"));
    if (m_Device)
    {
        m_Bypass=true;
        if (!Custom.compare(Save()))
        {
            m_Device->Load(XML);
            QDomLiteElement* Device=Custom.elementByTag("Device");
            if (Device)
            {
                for (int i=0;i<Device->attributeCount();i++)
                {
                    QString n=Device->attributeName(i);
                    m_Device->SetParameterValue(n,Device->attributeValue(n));
                }
            }
        }
        m_Bypass=false;
    }
}

const QString CDeviceContainer::Save()
{
    QDomLiteElement Custom("Custom");
    QDomLiteElement* Device=new QDomLiteElement("Device");
    if (m_Device)
    {
        Custom.fromString(m_Device->Save());
        Custom.setAttribute("DeviceType",m_DeviceType);
        for (int i=0;i<m_Device->ParameterCount();i++)
        {
            Device->setAttribute(m_Device->Parameter(i).Name,m_Device->GetParameterValue(i));
        }
        Custom.appendChild(Device);
    }
    return Custom.toString();
}

void CDeviceContainer::Execute(const bool Show)
{
    if (m_Device) m_Device->Execute(Show);
}

void CDeviceContainer::RaiseForm()
{
    if (m_Device) m_Device->RaiseForm();
}

void CDeviceContainer::HideForm()
{
    if (m_Device) m_Device->HideForm();
}

const QString CDeviceContainer::OpenFile(const QString &Filter)
{
    qDebug() << Filter << m_DeviceType;
    if (m_Device)
    {
        if (m_DeviceType==Filter) return m_DeviceType;
    }
    ClearDevice();
    m_Bypass=true;
#ifdef CAUDIOUNITHOST_H
    if (Filter=="AudioUnitHost") m_Device=new CAudioUnitHost;
#endif
#ifdef CVSTHOST_H
    if (Filter=="VSTHost") m_Device=new CVSTHost;
#endif
#ifdef CSF2PLAYER_H
    if (Filter=="SF2Player") m_Device=new CSF2Player;
#endif
    if (m_Device)
    {
        m_DeviceType=Filter;
        m_Device->Init(0,m_MainWindow);
        m_Device->SetHost((IHost*)m_MainWindow);
        DeviceOut=(COutJack*)m_Device->GetJack(IJack::Out,IJack::Audio);
        if (DeviceOut) InsideOut->ConnectToOut(DeviceOut);
        DeviceIn=(CInJack*)m_Device->GetJack(IJack::In,IJack::Audio);
        if (DeviceIn) DeviceIn->ConnectToOut(InsideIn);
        DeviceMIDIIn=(CInJack*)m_Device->GetJack(IJack::In,IJack::MIDI);
        if (DeviceMIDIIn) DeviceMIDIIn->ConnectToOut(InsideMIDIIn);
    }
    m_Bypass=false;
    return QString();
}

void CDeviceContainer::ClearDevice()
{
    m_Bypass=true;
    if (m_Device)
    {
        if (DeviceIn) DeviceIn->DisconnectFromOut(InsideIn);
        if (DeviceMIDIIn) DeviceMIDIIn->DisconnectFromOut(InsideMIDIIn);
        if (DeviceOut) InsideOut->DisconnectFromOut(DeviceOut);
        delete m_Device;
    }
    m_Device=NULL;
    m_DeviceType.clear();
    m_PresetName.clear();
    DeviceIn=NULL;
    DeviceMIDIIn=NULL;
    DeviceOut=NULL;
    m_Bypass=false;
}

const QString CDeviceContainer::FileName()
{
    if (m_Device) return m_Device->FileName();
    return QString();
}

const QString CDeviceContainer::CurrentPresetName(const short channel)
{
#ifdef CSF2PLAYER_H
    if (m_DeviceType=="SF2Player")
    {
        CSF2Device* d=&(((CSF2Player*)m_Device)->SF2Device);
        short p=d->currentPreset(channel);
        short b=d->currentBank(channel);
        if ((bank != b) | (m_PresetName.isEmpty()))
        {
            bank=b;
            preset=p;
            m_PresetName="Bank "+QString::number(b)+"\n"+d->banks[b].presets[p].name;
        }
        if (preset != p)
        {
            preset=p;
            m_PresetName="Bank "+QString::number(b)+"\n"+d->banks[b].presets[p].name;
        }
    }
#endif
#ifdef CAUDIOUNITHOST_H
    if (m_DeviceType=="AudioUnitHost")
    {
        CAudioUnitHost* d=((CAudioUnitHost*)m_Device);
        m_PresetName=d->PresetName();
    }
#endif
#ifdef CVSTHOST_H
    if (m_DeviceType=="VSTHost")
    {
        CVSTHost* d=((CVSTHost*)m_Device);
        m_PresetName=d->PresetName();
    }
#endif
    return m_PresetName;
}

const QStringList CDeviceContainer::Banks()
{
    QStringList l;
#ifdef CSF2PLAYER_H
    if (m_DeviceType=="SF2Player")
    {
        CSF2Device* d=&(((CSF2Player*)m_Device)->SF2Device);
        foreach (int i,d->banks.keys())
        {
            l.append(QString("000" + QString::number(i)).right(3));
        }
    }
#endif
#ifdef CAUDIOUNITHOST_H
#endif
#ifdef CVSTHOST_H
#endif
    return l;
}

const QStringList CDeviceContainer::Presets(const int Bank)
{
    QStringList l;
#ifdef CSF2PLAYER_H
    if (m_DeviceType=="SF2Player")
    {
        CSF2Device* d=&(((CSF2Player*)m_Device)->SF2Device);
        foreach (int i,d->banks[Bank].presets.keys())
        {
            QString PresetNum="000" + QString::number(i);
            PresetNum=PresetNum.right(3)  + " " + d->banks[Bank].presets[i].name;
            l.append(PresetNum);
        }
        l.sort();
    }
#endif
#ifdef CAUDIOUNITHOST_H
    if (m_DeviceType=="AudioUnitHost")
    {
        CAudioUnitHost* d=((CAudioUnitHost*)m_Device);
        l=d->PresetNames();
    }
#endif
#ifdef CVSTHOST_H
    if (m_DeviceType=="VSTHost")
    {
        CVSTHost* d=((CVSTHost*)m_Device);
        l=d->PresetNames();
    }
#endif
    return l;
}

int CDeviceContainer::CurrentBank(const short channel)
{
#ifdef CSF2PLAYER_H
    if (m_DeviceType=="SF2Player")
    {
        CSF2Device* d=&(((CSF2Player*)m_Device)->SF2Device);
        return d->currentBank(channel);
    }
#endif
#ifdef CAUDIOUNITHOST_H
#endif
#ifdef CVSTHOST_H
#endif
    return 0;
}

int CDeviceContainer::CurrentPreset(const short channel)
{
#ifdef CSF2PLAYER_H
    if (m_DeviceType=="SF2Player")
    {
        CSF2Device* d=&(((CSF2Player*)m_Device)->SF2Device);
        return d->currentPreset(channel);
    }
#endif
#ifdef CAUDIOUNITHOST_H
    if (m_DeviceType=="AudioUnitHost")
    {
        CAudioUnitHost* d=((CAudioUnitHost*)m_Device);
        return d->PresetNames().indexOf(d->PresetName());
    }
#endif
#ifdef CVSTHOST_H
    if (m_DeviceType=="VSTHost")
    {
        CVSTHost* d=((CVSTHost*)m_Device);
        return d->PresetNames().indexOf(d->PresetName());
    }
#endif
    return 0;
}

void CDeviceContainer::SetCurrentPreset(const int Bank, const int Preset)
{
#ifdef CSF2PLAYER_H
    if (m_DeviceType=="SF2Player")
    {
        CSF2Player* d=((CSF2Player*)m_Device);
        d->SetProgram(Bank,Preset);
    }
#endif
#ifdef CAUDIOUNITHOST_H
    if (m_DeviceType=="AudioUnitHost")
    {
        CAudioUnitHost* d=((CAudioUnitHost*)m_Device);
        d->SetProgram(Preset);
    }
#endif
#ifdef CVSTHOST_H
    if (m_DeviceType=="VSTHost")
    {
        CVSTHost* d=((CVSTHost*)m_Device);
        d->SetProgram(Preset);
    }
#endif

}

void CDeviceContainer::SetParameterValue(const QString &Name, const int Value)
{
    if (m_Device) m_Device->SetParameterValue(Name,Value);
}

int CDeviceContainer::GetParameterValue(const QString &Name)
{
    if (m_Device) return m_Device->GetParameterValue(Name);
    return 0;
}
