#include "cstereocontainerbase.h"
#include "../PluginLoader/caddins.h"
#include "cdevicelist.h"

monoItems::monoItems(CStereoContainerBase *c) {
    setAttribute(Qt::WA_DeleteOnClose);
    container = c;
    connect(this,qOverload<QString>(&QSignalMenu::menuClicked),this,&monoItems::setDeviceType);
    QStringList l = CAddIns::addInNames(Effect | SynthModule, "monoout");
    for (int i = 0; i < l.size(); i++) {
        QString s = l[i];
        this->addAction(s,s);
    }
}

void monoItems::setDeviceType(QString s) {
    container->setDeviceType(s);
}

CStereoContainerBase::CStereoContainerBase(const QString& name) {
    m_Name = name;
    m_Devices[0] = nullptr;
    m_Devices[1] = nullptr;
    m_Bypass = true;
}

CStereoContainerBase::~CStereoContainerBase() {
    ClearDevice();
    if (m_Initialized) {
        qDeleteAll(JacksCreated);
        //JacksCreated.clear();
    }
}

void CStereoContainerBase::init(const int Index, QWidget* MainWindow)
{
    IDevice::init(Index,MainWindow);
    addJackStereoIn();
    addJackMIDIIn();
    //addJackModulationIn();
    addJackStereoOut(stereoout);
    InsideIn[0]=new COutJack("InsideInLeft","This",IJack::Audio,IJack::Out,this,jnInsideInLeft);
    InsideIn[1]=new COutJack("InsideInRight","This",IJack::Audio,IJack::Out,this,jnInsideInRight);
    InsideMIDIIn=new COutJack("InsideMIDIIn","This",IJack::MIDI,IJack::Out,this,jnInsideMIDIIn);
    //InsideModulationIn=new COutJack("InsideMIDIIn","This",IJack::MIDI,IJack::Out,this,jnInsideModulationIn);
    JacksCreated.append(InsideIn[0]);
    JacksCreated.append(InsideIn[1]);
    JacksCreated.append(InsideMIDIIn);
    //JacksCreated.append(InsideModulationIn);
}

void CStereoContainerBase::setDeviceType(const QString &Filter)
{
    QMutexLocker locker(&mutex);
    if (hasDevice()) if (m_DeviceType == Filter) return;
    ClearDevice();
    m_Bypass=true;
    const int MenuIndex = CAddIns::indexOf(Filter);
    if (MenuIndex > -1)
    {
        m_Devices[0] = instancefn(MenuIndex)();
        m_Devices[1] = instancefn(MenuIndex)();
        if (hasDevice())
        {
            m_DeviceType=Filter;
            setAlias(m_DeviceType);
            m_Devices[0]->init(0,m_MainWindow);
            m_Devices[1]->init(0,m_MainWindow);
            const COutJack* o = m_Devices[0]->outJack(IJack::Mono);
            if (o) {
                if (o->attachMode != IJack::Mono) {
                    ClearDevice();
                    return;
                }
            }
            else {
                ClearDevice();
                return;
            }
            for (int i = 0; i < m_Devices[0]->inJackCount(); i++) {
                CInJack* j = m_Devices[0]->inJack(i);
                if (j->attachMode <= IJackBase::Trigger) {
                    extraInJack e;
                    e.index = m_Jacks.size();
                    e.inJack = (CInJack*)addJack(j->name(),j->attachMode,IJack::In,e.index);
                    int procIndex = qMax(m_Devices[0]->jackCount(),jackCount()) + JacksCreated.size() + ExtraJacks.size();
                    e.insideInJack = new COutJack("Inside" + j->name(),"This",j->attachMode,IJack::Out,this,procIndex);
                    e.deviceInJack[0] = j;
                    e.deviceInJack[1] = m_Devices[1]->inJack(i);
                    ExtraJacks.append(e);
                }
            }
            if (ExtraJacks.size()) {
                for (int i = 0; i < ExtraJacks.size(); i++) {
                    m_OwnerList->addJack(ExtraJacks[i].inJack);
                }
                m_Host->updateDeviceJacks();
            }
            //for (int i = 0; i < m_Jacks.size(); i++) m_OwnerList->addJack(m_Jacks[i]);
            //m_Host->updateDeviceJacks();
            outProcIndex = o->procIndex;
            for (int c = 0; c < 2; c++) {
                addTickerDevice(m_Devices[c]);
                m_Devices[c]->setHost(m_Host);
                DeviceIn[c]=m_Devices[c]->inJack(IJack::Audio);
                if (DeviceIn[c]) DeviceIn[c]->connectTo(InsideIn[c]);
                DeviceMIDIIn[c]=m_Devices[c]->inJack(IJack::MIDI);
                if (DeviceMIDIIn[c]) DeviceMIDIIn[c]->connectTo(InsideMIDIIn);
                for (int i = 0; i < ExtraJacks.size(); i++) {
                    ExtraJacks[i].deviceInJack[c]->connectTo(ExtraJacks[i].insideInJack);
                }
                //DeviceModulationIn[c]=m_Devices[c]->inJack(IJack::Voltage);
                //if (DeviceModulationIn[c]) DeviceModulationIn[c]->connectTo(InsideModulationIn);
#ifdef DUALMONO
                    for (int i = 0; i < m_Devices[c]->parameterCount(); i++) {
                        CParameter* p = m_Devices[c]->parameter(i);
                        int gi = m_Devices[0]->parameterGroupID(i);
                        if (gi > -1) {
                            CParameterGroup* g = m_Devices[c]->parameterGroup(gi);
                            if (g->startIndex == i) makeParameterGroup(g->endIndex + 1 - i, g->Name, g->color);
                        }
                        QString name = p->Name + ((c == 0) ? " L" : " R");
                        m_OwnerList->addCustomParameter(this,p->Type,name,p->Unit,p->Min,p->Max,p->DecimalFactor,p->List,p->Value); //DesktopComponent->deviceList()->addCustomParameter(m_Device,p->Type,c->defaultCaption(),p->Unit,p->Min,p->Max,p->DecimalFactor,p->List,p->Value);
                    }
#endif
            }
#ifndef DUALMONO
                for (int i = 0; i < m_Devices[0]->parameterCount(); i++) {
                    CParameter* p = m_Devices[0]->parameter(i);
                    int gi = m_Devices[0]->parameterGroupID(i);
                    if (gi > -1) {
                        CParameterGroup* g = m_Devices[0]->parameterGroup(gi);
                        if (g->startIndex == i) makeParameterGroup(g->endIndex + 1 - i, g->Name, g->color);
                    }
                    m_OwnerList->addCustomParameter(this,p->Type,p->Name,p->Unit,p->Min,p->Max,p->DecimalFactor,p->List,p->Value); //DesktopComponent->deviceList()->addCustomParameter(m_Device,p->Type,c->defaultCaption(),p->Unit,p->Min,p->Max,p->DecimalFactor,p->List,p->Value);
                }
#endif
            updateHostParameter();
        }
    }
    m_Bypass=false;
}

void CStereoContainerBase::ClearDevice()
{
    QMutexLocker locker(&mutex);
    //qDebug() << "CDeviceContainer ClearDevice";
    m_Bypass=true;
    if (hasDevice())
    {
        if (ExtraJacks.size()) {
            for (int i = 0; i < ExtraJacks.size(); i++) {
                for (int c = 0; c < 2; c++) {
                    ExtraJacks[i].deviceInJack[c]->disconnectFrom(ExtraJacks[i].insideInJack);
                }
                removeJack(ExtraJacks[i].inJack);
                m_OwnerList->disconnectJack(ExtraJacks[i].inJack);
                m_OwnerList->removeJack(ExtraJacks[i].inJack);
                delete ExtraJacks[i].inJack;
                delete ExtraJacks[i].insideInJack;
            }
            ExtraJacks.clear();
            //m_Host->updateDeviceJacks();
        }

#ifndef DUALMONO
            for (int i = 0; i < m_Devices[0]->parameterCount(); i++) {
                m_OwnerList->removeCustomParameter(this,m_Devices[0]->parameter(i)->Name);
            }
            for (int c = 0; c < 2; c++) {
                if (DeviceIn[c]) DeviceIn[c]->disconnectFrom(InsideIn[c]);
                if (DeviceMIDIIn[c]) DeviceMIDIIn[c]->disconnectFrom(InsideMIDIIn);
                //if (DeviceModulationIn[c]) DeviceModulationIn[c]->disconnectFrom(InsideModulationIn);
                delete m_Devices[c];
            }
#else
            for (int c = 0; c < 2; c++) {
                for (int i = 0; i < m_Devices[c]->parameterCount(); i++) {
                    QString name = m_Devices[c]->parameter(i)->Name + ((c == 0) ? " L" : " R");
                    m_OwnerList->removeCustomParameter(this,name);
                }
                if (DeviceIn[c]) DeviceIn[c]->disconnectFrom(InsideIn[c]);
                if (DeviceMIDIIn[c]) DeviceMIDIIn[c]->disconnectFrom(InsideMIDIIn);
                //if (DeviceModulationIn[c]) DeviceModulationIn[c]->disconnectFrom(InsideModulationIn);
                delete m_Devices[c];
            }
#endif
    }
    //qDeleteAll(m_Parameters);
    //m_Parameters.clear();
    clearTickerDevices();
    m_DeviceType.clear();
    setAlias(m_DeviceType);
    for (int c = 0; c < 2; c++) {
        m_Devices[c]=nullptr;
        DeviceIn[c]=nullptr;
        DeviceMIDIIn[c]=nullptr;
        //DeviceModulationIn[c]=nullptr;
    }
    m_Bypass=false;
    updateHostParameter();
}

void CStereoContainerBase::selectDevice(){
    monoItems* i = new monoItems(this);
    i->popup(QCursor::pos());
}

void CStereoContainerBase::process()
{
    if (hasDevice())
    {
        InBuffer=FetchAStereo(stereoin);
        MIDIBuffer=FetchP(midiin);
        //modulation=Fetch(jnModulationIn);
        for (int i = 0; i < ExtraJacks.size(); i++) {
            ExtraJacks[i].value = Fetch(ExtraJacks[i].index);
        }
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

bool CStereoContainerBase::hasDevice() const {
    return m_Devices[0] != nullptr;
}

CAudioBuffer* CStereoContainerBase::getNextA(const int ProcIndex)
{
    if (m_Bypass) return nullptr;//&m_NullBufferStereo;
    if (hasDevice())
    {
        if (m_Process)
        {
            m_Process=false;
            process();
        }
        if (ProcIndex==jnInsideInLeft) return InBuffer->leftBuffer;
        if (ProcIndex==jnInsideInRight) return InBuffer->rightBuffer;
        if (ProcIndex==stereoout) {
            CAudioBuffer* L = m_Devices[0]->getNextA(outProcIndex);
            CAudioBuffer* R = m_Devices[1]->getNextA(outProcIndex);
            float* pl = (L) ? L->data() : nullptr;
            float* pr = (R) ? R->data() : nullptr;
            StereoBuffer(stereoout)->fromDualMono(pl,pr);
            return m_AudioBuffers[stereoout];
        }
    }
    return nullptr;//&m_NullBufferStereo;
}

CMIDIBuffer* CStereoContainerBase::getNextP(const int /*ProcIndex*/)
{
    if (hasDevice())
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

float CStereoContainerBase::getNext(const int ProcIndex)
{
    if (hasDevice())
    {
        if (m_Process)
        {
            m_Process=false;
            process();
        }
        for (int i = 0; i < ExtraJacks.size(); i++) {
            //qDebug() << ProcIndex << ExtraJacks[i].index << ExtraJacks[i].procIndex << ExtraJacks[i].value;
            if (ProcIndex == ExtraJacks[i].insideInJack->procIndex) return ExtraJacks[i].value;
        }
        //return modulation;
    }
    return 0;
}

void CStereoContainerBase::unserializeParameters(const QDomLiteElement* xml)
{
    if (!xml) return;
    QMutexLocker locker(&mutex);
    setDeviceType(xml->attribute("DeviceType"));
    IDevice::unserializeParameters(xml);
    if (hasDevice()) {
        if (QDomLiteElement* L = xml->elementByTag("CustomLeft")) {
            m_Devices[0]->unserializeCustomParameters(L);
        }
        if (QDomLiteElement* R = xml->elementByTag("CustomRight")) {
            m_Devices[1]->unserializeCustomParameters(R);
        }
    }
    updateDeviceParameter();
}

void CStereoContainerBase::serializeParameters(QDomLiteElement* xml) const
{
    IDevice::serializeParameters(xml);
    if (hasDevice()) {
        xml->setAttribute("DeviceType",m_DeviceType);
        m_Devices[0]->serializeCustomParameters(xml->appendChild("CustomLeft"));
        m_Devices[1]->serializeCustomParameters(xml->appendChild("CustomRight"));
    }
}

void CStereoContainerBase::execute(bool show){
    if (hasDevice()) {
        m_Devices[0]->execute(show);
#ifdef DUALMONO
            m_Devices[1]->execute(show);
#endif
        return;
    }
    selectDevice();
}

void CStereoContainerBase::activate()
{
    if (hasDevice()) {
        for (int c = 0; c < 2; c++) m_Devices[c]->activate();
    }
}

void CStereoContainerBase::raiseForm()
{
    if (hasDevice()) {
        for (int c = 0; c < 2; c++) m_Devices[c]->raiseForm();
    }
}

void CStereoContainerBase::hideForm()
{
    if (hasDevice()) {
        for (int c = 0; c < 2; c++) m_Devices[c]->hideForm();
    }
}

void CStereoContainerBase::cascadeForm(QPoint &p)
{
    if (hasDevice()) {
        for (int c = 0; c < 2; c++) m_Devices[c]->cascadeForm(p);
    }
}

bool CStereoContainerBase::hasUI() const
{
    return (hasDevice()) ? m_Devices[0]->hasUI() : false;
}

QWidget *CStereoContainerBase::UI() const
{
    return (hasDevice()) ? m_Devices[0]->UI() : nullptr;
}

const QPixmap *CStereoContainerBase::picture() const
{
    return (hasDevice()) ? m_Devices[0]->picture() : nullptr;
}

const QString CStereoContainerBase::filename() const
{
    return (hasDevice()) ? m_Devices[0]->filename() : QString();
}

const QString CStereoContainerBase::currentBankPresetName(const short channel) const
{
    return (hasDevice()) ? m_Devices[0]->currentBankPresetName(channel) : QString();
}

const QStringList CStereoContainerBase::bankNames() const
{
    return (hasDevice()) ? m_Devices[0]->bankNames() : QStringList();
}

const QStringList CStereoContainerBase::presetNames(const int bank) const
{
    return (hasDevice()) ? m_Devices[0]->presetNames(bank) : QStringList();
}

long CStereoContainerBase::currentBankPreset(const short channel) const
{
    return (hasDevice()) ? m_Devices[0]->currentBankPreset(channel) : 0;
}

int CStereoContainerBase::bankPresetNumber(const int bank, const int preset) const
{
    return (hasDevice()) ? m_Devices[0]->bankPresetNumber(bank,preset) : preset;
}

void CStereoContainerBase::setCurrentBankPreset(const int index)
{
    QMutexLocker locker(&mutex);
    if (hasDevice()) {
        for (int c = 0; c < 2; c++) m_Devices[c]->setCurrentBankPreset(index);
    }
}

void CStereoContainerBase::NoteOn(byte Pitch, byte Channel, byte Velocity, byte Patch, byte Bank)
{
    QMutexLocker locker(&mutex);
    if (hasDevice())
    {
        if (parameterValue("Patch Change"))
        {
            MIDIInBuffer.append(Channel+0xB0,0,Bank);
            MIDIInBuffer.append(Channel+0xC0,Patch);
        }
        MIDIInBuffer.append(Channel+0x90,Pitch,Velocity);
    }
}

void CStereoContainerBase::NoteOff(byte Pitch, byte Channel)
{
    QMutexLocker locker(&mutex);
    if (hasDevice()) MIDIInBuffer.append(Channel+0x80,Pitch);
}

void CStereoContainerBase::setParameterValue(const QString &name, const int value)
{
    QMutexLocker locker(&mutex);
    if (hasDevice())
    {
        for (int c = 0; c < 2; c++) if (CParameter* p = m_Devices[c]->parameter(name)) p->setValue(value);
    }
}

int CStereoContainerBase::parameterValue(const QString &name) const
{
    if (hasDevice())
    {
        if (CParameter* p = m_Devices[0]->parameter(name)) return p->Value;
    }
    return 0;
}

IDevice *CStereoContainerBase::childDevice(const int index) const
{
    return (hasDevice()) ? m_Devices[0]->childDevice(index) : nullptr;
}

int CStereoContainerBase::childDeviceCount() const
{
    return (hasDevice()) ? m_Devices[0]->childDeviceCount() : 0;
}

void CStereoContainerBase::updateDeviceParameter(const CParameter* p)
{
    if (p) {
        const int i = m_Parameters.indexOf(p);
        if (i > -1) setDeviceParameters(i);
    }
    else {
        for (int i = 0; i < m_Parameters.count(); i++) {
            setDeviceParameters(i);
        }
#ifndef DUALMONO
        QDomLiteElement e;
        m_Devices[0]->serializeCustomParameters(&e);
        m_Devices[1]->unserializeCustomParameters(&e);
#endif
    }
}

void CStereoContainerBase::setDeviceParameters(const int i) {
#ifndef DUALMONO
        if (m_Parameters[i]->Value != m_Devices[0]->parameter(i)->Value) {
            for (int c = 0; c < 2; c++) {
                m_Devices[c]->parameter(i)->setValue(m_Parameters[i]->Value);
            }
        }
#else
        if (i < m_Devices[0]->parameterCount()) {
            if (m_Parameters[i]->Value != m_Devices[0]->parameter(i)->Value) {
                m_Devices[0]->parameter(i)->setValue(m_Parameters[i]->Value);
            }
        }
        else {
            int iR = i - m_Devices[0]->parameterCount();
            if (m_Parameters[i]->Value != m_Devices[1]->parameter(iR)->Value) {
                m_Devices[1]->parameter(iR)->setValue(m_Parameters[i]->Value);
            }
        }
#endif
}

