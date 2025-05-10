#include "caudiounithost.h"
#include "cvstform.h"
#undef devicename
#define devicename "AudioUnitHost"

CAudioUnitHost::CAudioUnitHost()
{
}

CAudioUnitHost::~CAudioUnitHost()
{

}

void CAudioUnitHost::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackStereoIn();
    addJackMIDIIn();
    addJackStereoOut(jnOut);
    addParameterVolume();
    startParameterGroup("MIDI", Qt::yellow);
    addParameterMIDIChannel();
    addParameterTranspose();
    addParameterPatchChange();
    endParameterGroup();
    m_Form=new CVSTForm(new CAudioUnitClass(), this,MainWindow);
    VolFactor=1.0;
    updateDeviceParameter();
}

void inline CAudioUnitHost::updateDeviceParameter(const CParameter* /*p*/)
{
    VolFactor=m_Parameters[pnVolume]->PercentValue;
    AUPLUGINCLASS->setChannelMode(m_Parameters[pnMIDIChannel]->Value);
    AUPLUGINCLASS->setTranspose(m_Parameters[pnTranspose]->Value);
    AUPLUGINCLASS->setPatchResponse(m_Parameters[pnPatchChange]->Value);
}

void CAudioUnitHost::play(const bool FromStart)
{
    AUPLUGINCLASS->play();
    IDevice::play(FromStart);
}

void CAudioUnitHost::pause()
{
    AUPLUGINCLASS->allNotesOff();
    AUPLUGINCLASS->stop();
    IDevice::pause();
}

const QString CAudioUnitHost::filename() const
{
    return AUPLUGINCLASS->filename();
}

void CAudioUnitHost::execute(const bool Show)
{
    if (Show)
    {
        if (AUPLUGINCLASS->filename().isEmpty())
        {
            AUPLUGINCLASS->popup(QCursor::pos());
        }
        else
        {
            m_Form->show();
            //AUPLUGINCLASS->setFixedSize(AUPLUGINCLASS->UISize());
            //m_Form->setFixedSize(m_Form->sizeHint());;
        }
    }
    else
    {
        m_Form->hide();
    }
}

void CAudioUnitHost::serializeCustom(QDomLiteElement* xml) const
{
    if (AUPLUGINCLASS->filename().isEmpty()) return;
    xml->setAttribute("Type",QVariant::fromValue(AUPLUGINCLASS->type()));
    xml->setAttribute("Subtype",QVariant::fromValue(AUPLUGINCLASS->subType()));
    xml->setAttribute("Manufacturer",QVariant::fromValue(AUPLUGINCLASS->manufacturer()));
    xml->setAttribute("Program",QVariant::fromValue(AUPLUGINCLASS->currentBankPreset()));
    QDomLiteDocument d;
    d.fromString(AUPLUGINCLASS->serializeString());
    xml->appendClone(d.documentElement);
    //AUPLUGINCLASS->serialize(xml);
}

void CAudioUnitHost::unserializeCustom(const QDomLiteElement* xml)
{
    if (!xml) return;
    QMutexLocker locker(&mutex);
    const OSType type=xml->attribute("Type").toUInt();
    const OSType subtype=xml->attribute("Subtype").toUInt();
    const OSType manufacturer=xml->attribute("Manufacturer").toUInt();
    if ((type != AUPLUGINCLASS->type()) || (subtype != AUPLUGINCLASS->subType()) || (manufacturer != AUPLUGINCLASS->manufacturer()))
    {
        AUPLUGINCLASS->selectAU(type,subtype,manufacturer);
    }
    const int p=xml->attributeValueInt("Program");
    if (p > -1) AUPLUGINCLASS->setBankPreset(p);
    FORMFUNC(CVSTForm)->fillList(p);
    if (QDomLiteElement* plist=xml->elementByTag("plist"))
    {
        QDomLiteDocument d;
        d.fromString(AUPLUGINCLASS->serializeString());
        if (*plist != *d.documentElement)
        {
            *d.documentElement = *plist;
            QString s=d.toString();
            AUPLUGINCLASS->unserializeString(s.replace("<data/>","<data></data>"));
        }
    }
}

void CAudioUnitHost::process()
{
    AUPLUGINCLASS->parseMIDI(FetchP(jnMIDIIn));
    //PLUGINCLASS->inBuffer.writeBuffer(FetchA(jnIn),true);
    if (AUPLUGINCLASS->isMono()) {
        AUPLUGINCLASS->InBuffers.fill(m_MonoBuffer.fromStereo(FetchAStereo(jnIn)->data()),m_BufferSize);
    }
    else {
        AUPLUGINCLASS->InBuffers.fill(FetchA(jnIn)->data(),m_BufferSize*2);
    }
    if (AUPLUGINCLASS->process())
    {
        if (AUPLUGINCLASS->isMono()) {
            m_AudioBuffers[jnOut]->writeBuffer(m_StereoBuffer.fromMono(AUPLUGINCLASS->OutBuffers.data()),float(VolFactor*0.5));
        }
        else {
            m_AudioBuffers[jnOut]->writeBuffer(AUPLUGINCLASS->OutBuffers.data(),VolFactor);
        }
    }
    else
    {
        m_AudioBuffers[jnOut]->zeroBuffer();
    }
}

const QString CAudioUnitHost::currentBankPresetName(const short /*channel*/) const
{
    return AUPLUGINCLASS->bankPresetName();
}

const QStringList CAudioUnitHost::presetNames(const int /*bank*/) const
{
    return AUPLUGINCLASS->bankPresetNames();
}

void CAudioUnitHost::setCurrentBankPreset(const int index)
{
    //if (index != currentBankPreset()) updateHostBankPreset(index);
    FORMFUNC(CVSTForm)->setBankPreset(index);
}

long CAudioUnitHost::currentBankPreset(const short channel) const
{
    return AUPLUGINCLASS->currentBankPreset(channel);
}

const QPixmap* CAudioUnitHost::picture() const
{
    return AUPLUGINCLASS->picture();
}

void CAudioUnitHost::parseEvent(const CMIDIEvent* Event)
{
    AUPLUGINCLASS->parseEvent(Event);
}
