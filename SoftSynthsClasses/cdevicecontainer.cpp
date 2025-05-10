#include "cdevicecontainer.h"
#include "caddins.h"

CDeviceContainer::CDeviceContainer(const QString &Name) : IDevice()
{
    QMutexLocker locker(&mutex);
    m_Name=Name;
    m_Device=nullptr;
    m_Bypass=false;
    ClearDevice();
    m_Program=-1;
}

CDeviceContainer::~CDeviceContainer()
{
    qDebug() << "~CDeviceContainer";
    ClearDevice();
    m_Bypass=true;
    if (m_Initialized) qDeleteAll(JacksCreated);
}

void CDeviceContainer::process()
{
    if (m_Device)
    {
        InBuffer=FetchAStereo(jnIn);
        MIDIBuffer=FetchP(jnMIDIIn);
        if (!MIDIInBuffer.isEmpty())
        {
            if (MIDIBuffer)
            {
                MIDIBuffer->append(&MIDIInBuffer);
            }
            else
            {
                tempBuffer.clear();
                MIDIBuffer=&tempBuffer;
                tempBuffer.append(&MIDIInBuffer);
            }
            MIDIInBuffer.clear();
        }
    }
}

CAudioBuffer* CDeviceContainer::getNextA(const int ProcIndex)
{
    if (m_Bypass) return nullptr;//&m_NullBufferStereo;
    if (m_Device)
    {
        if (m_Process)
        {
            m_Process=false;
            process();
        }
        if (ProcIndex==jnInsideIn) return InBuffer;
        if (ProcIndex==jnOut) return m_Device->getNextA(outProcIndex); //return InsideOut->getNextA();
    }
    return nullptr;//&m_NullBufferStereo;
}

CMIDIBuffer* CDeviceContainer::getNextP(const int /*ProcIndex*/)
{
    if (m_Device)
    {
        if (m_Process)
        {
            m_Process=false;
            process();
        }
        return MIDIBuffer;
    }
    return nullptr;
}
/*
void CDeviceContainer::tick()
{
    //IDevice::tick();
    m_Process=true;
    if (m_Device) m_Device->tick();
}
*/
void CDeviceContainer::updateHostParameter(const CParameter* p)
{
    if (m_Device) m_Device->updateHostParameter(p);
}

void CDeviceContainer::setHost(IHost *host)
{
    QMutexLocker locker(&mutex);
    m_Host=host;
    if (m_Device) m_Device->setHost(host);
}

void CDeviceContainer::activate()
{
    if (m_Device) m_Device->activate();
}

void CDeviceContainer::init(const int Index, QWidget* MainWindow)
{
    QMutexLocker locker(&mutex);
    if (m_Name.isEmpty()) m_Name="DeviceContainer";
    IDevice::init(Index,MainWindow);
    addJackStereoIn();
    addJackMIDIIn();
    addJackStereoOut(jnOut);
    InsideIn=new COutJack("InsideIn","This",IJack::Stereo,IJack::Out,this,jnInsideIn);
    InsideMIDIIn=new COutJack("InsideMIDIIn","This",IJack::MIDI,IJack::Out,this,jnInsideMIDIIn);
    //InsideOut=new CInJack("InsideOut","This",IJack::Stereo,IJack::In,this);
    JacksCreated.append(InsideIn);
    JacksCreated.append(InsideMIDIIn);
    //JacksCreated.append(InsideOut);
}

void CDeviceContainer::unserializeCustom(const QDomLiteElement* xml)
{
    if (!xml) return;
    QMutexLocker locker(&mutex);
    //qDebug() << "CDeviceContainer unserializeCustom" << m_Device << m_DeviceType << xml->toString();
    setDeviceType(xml->attribute("DeviceType"));
    //qDebug() << m_Device << m_DeviceType;
    if (m_Device)
    {
        m_Bypass=true;
        QDomLiteElement e("Custom");
        serializeCustom(&e);
        //qDebug() << "Old:" << e.toString() << "New:" << xml->toString();
        if (!xml->compare(&e))
        {
            m_Device->unserializeCustomParameters(xml);
            m_Device->unserializeUI(xml);
            m_Device->unserializeStandardParameters(xml->elementByTag("Device"));
        }
        m_Bypass=false;
    }
    //qDebug() << "CDeviceContainer unserializeCustom end" << m_Device << m_DeviceType << xml->toString();
}

void CDeviceContainer::serializeCustom(QDomLiteElement* xml) const
{
    //qDebug() << "CDeviceContainer serializeCustom" << m_Device << m_DeviceType;
    if (m_Device)
    {
        m_Device->serializeCustomParameters(xml);
        m_Device->serializeUI(xml);
        xml->setAttribute("DeviceType",m_DeviceType);
        m_Device->serializeStandardParameters(xml->appendChild(new QDomLiteElement("Device")));
    }
    //qDebug() << "CDeviceContainer serializeCustom end" << m_Device << m_DeviceType << xml->toString();
}

void CDeviceContainer::execute(const bool Show)
{
    if (m_Device) m_Device->execute(Show);
}

void CDeviceContainer::raiseForm()
{
    if (m_Device) m_Device->raiseForm();
}

void CDeviceContainer::hideForm()
{
    if (m_Device) m_Device->hideForm();
}

void CDeviceContainer::cascadeForm(QPoint& p)
{
    if (m_Device) m_Device->cascadeForm(p);
}

bool CDeviceContainer::hasUI() const
{
    return (m_Device) ? m_Device->hasUI() : false;
}

QWidget* CDeviceContainer::UI() const
{
    return (m_Device) ? m_Device->UI() : nullptr;
}

const QPixmap* CDeviceContainer::picture() const
{
    return (m_Device) ? m_Device->picture() : nullptr;
}

IDevice* CDeviceContainer::childDevice(const int index) const
{
    return (m_Device) ? m_Device->childDevice(index) : nullptr;
}

int CDeviceContainer::childDeviceCount() const
{
    return (m_Device) ? m_Device->childDeviceCount() : 0;
}

void CDeviceContainer::setDeviceType(const QString &Filter)
{
    QMutexLocker locker(&mutex);
    if (m_Device) if (m_DeviceType==Filter) return;
    ClearDevice();
    m_Bypass=true;
    const int MenuIndex=CAddIns::indexOf(Filter);
    if (MenuIndex>-1)
    {
        m_Device = instancefn(MenuIndex)();
        if (m_Device)
        {
            m_DeviceType=Filter;
            m_Device->init(0,m_MainWindow);
            addTickerDevice(m_Device);
            m_Device->setHost(m_Host);
            outProcIndex=m_Device->outJack(IJack::Audio)->procIndex;
            DeviceIn=m_Device->inJack(IJack::Audio);
            if (DeviceIn) DeviceIn->connectTo(InsideIn);
            DeviceMIDIIn=m_Device->inJack(IJack::MIDI);
            if (DeviceMIDIIn) DeviceMIDIIn->connectTo(InsideMIDIIn);
        }
    }
    m_Bypass=false;
}

const QStringList CDeviceContainer::instrumentList()
{
    return {"VSTHost","AudioUnitHost","SF2Player","Sampler","PlugInBox"};
}

const QStringList CDeviceContainer::effectList()
{
    return {"VSTHost","AudioUnitHost","EffectRack","PlugInBox","StereoBox","StereoSplitBox"};
}

const QString CDeviceContainer::deviceType() const
{
    return (m_Device) ? m_DeviceType : QString();
}

const QString CDeviceContainer::caption() const
{
    QString s;
    if (m_Device)
    {
        s=QFileInfo(m_Device->filename()).completeBaseName();
        if (s.isEmpty()) s=m_DeviceType;
    }
    return s;
}

void CDeviceContainer::ClearDevice()
{
    QMutexLocker locker(&mutex);
    //qDebug() << "CDeviceContainer ClearDevice";
    m_Bypass=true;
    if (m_Device)
    {
        if (DeviceIn) DeviceIn->disconnectFrom(InsideIn);
        if (DeviceMIDIIn) DeviceMIDIIn->disconnectFrom(InsideMIDIIn);
        delete m_Device;
    }
    m_Device=nullptr;
    clearTickerDevices();
    m_DeviceType.clear();
    DeviceIn=nullptr;
    DeviceMIDIIn=nullptr;
    m_Bypass=false;
}

const QString CDeviceContainer::filename() const
{
    return (m_Device) ? m_Device->filename() : QString();
}

const QString CDeviceContainer::currentBankPresetName(const short channel) const
{
    return (m_Device) ? m_Device->currentBankPresetName(channel) : QString();
}

const QStringList CDeviceContainer::bankNames() const
{
    return (m_Device) ? m_Device->bankNames() : QStringList();
}

const QStringList CDeviceContainer::presetNames(const int bank) const
{
    return (m_Device) ? m_Device->presetNames(bank) : QStringList();
}

long CDeviceContainer::currentBankPreset(const short channel) const
{
    return (m_Device) ? m_Device->currentBankPreset(channel) : 0;
}

int CDeviceContainer::bankPresetNumber(const int bank, const int preset) const
{
    return (m_Device) ? m_Device->bankPresetNumber(bank,preset) : preset;
}

void CDeviceContainer::setCurrentBankPreset(const int index)
{
    QMutexLocker locker(&mutex);
    if (m_Device) m_Device->setCurrentBankPreset(index);
}

void CDeviceContainer::NoteOn(byte Pitch, byte Channel, byte Velocity, byte Patch, byte Bank)
{
    QMutexLocker locker(&mutex);
    if (m_Device)
    {
        if (parameterValue("Patch Change"))
        {
            MIDIInBuffer.append(Channel+0xB0,0,Bank);
            MIDIInBuffer.append(Channel+0xC0,Patch);
        }
        MIDIInBuffer.append(Channel+0x90,Pitch,Velocity);
    }
}

void CDeviceContainer::NoteOff(byte Pitch, byte Channel)
{
    QMutexLocker locker(&mutex);
    if (m_Device) MIDIInBuffer.append(Channel+0x80,Pitch);
}

void CDeviceContainer::setParameterValue(const QString &name, const int value)
{
    QMutexLocker locker(&mutex);
    if (m_Device)
    {
        for (int i=0;i<m_Device->parameterCount();i++)
        {
            if (m_Device->parameter(i)->Name==name)
            {
                m_Device->parameter(i)->setValue(value);
                return;
            }
        }

    }
}
int CDeviceContainer::parameterValue(const QString &name) const
{
    if (m_Device)
    {
        for (int i=0;i<m_Device->parameterCount();i++)
        {
            if (m_Device->parameter(i)->Name==name)
            {
                return m_Device->parameter(i)->Value;
            }
        }
    }
    return 0;
}
