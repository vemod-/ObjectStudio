#include "cvsthost.h"
#include "cvstform.h"
#include "cvsthostclass.h"
#include <QFileDialog>

#undef devicename
#define devicename "VSTHost"
#define BufferCount 8

CVSTHost::CVSTHost()
{
}

CVSTHost::~CVSTHost()
{
    qDebug() << "Exit CVSTHost";
}

void CVSTHost::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackStereoIn();
    addJackMIDIIn();
    for (int i=0;i<BufferCount/2;i++) addJackStereoOut(jnOut+i,"Out " + QString::number(i));
    addParameterVolume();
    startParameterGroup("MIDI", Qt::yellow);
    addParameterMIDIChannel();
    addParameterTranspose();
    addParameterPatchChange();
    endParameterGroup();
    auto VSTHostClass = new CVSTHostClass();
    addFileParameter(VSTHostClass,presets.VSTPath);
    VSTHostClass->fileParameter = m_FileParameter;
    m_Form=new CVSTForm(VSTHostClass, this,MainWindow);
    VolFactor=1.0;
    OldBuffers=0;
    updateDeviceParameter();
}

void inline CVSTHost::updateDeviceParameter(const CParameter* /*p*/)
{
    VolFactor=m_Parameters[pnVolume]->PercentValue;
    VSTPLUGINCLASS->setChannelMode(m_Parameters[pnMIDIChannel]->Value);
    VSTPLUGINCLASS->setTranspose(m_Parameters[pnTranspose]->Value);
    VSTPLUGINCLASS->setPatchResponse(m_Parameters[pnPatchChange]->Value);
}

void CVSTHost::pause()
{
    VSTPLUGINCLASS->allNotesOff();
    IDevice::pause();
}

void CVSTHost::execute(const bool Show)
{
    if (Show)
    {
        if (filename().isEmpty())
        {
            VSTPLUGINCLASS->popup(QCursor::pos());
        }
        else
        {
            m_Form->show();
            //VSTPLUGINCLASS->setFixedSize(VSTPLUGINCLASS->UISize());
            //m_Form->setFixedSize(m_Form->sizeHint());;
        }
    }
    else
    {
        m_Form->hide();
    }
}

void CVSTHost::serializeCustom(QDomLiteElement* xml) const
{
    if (filename().isEmpty()) return;
    VSTPLUGINCLASS->serialize(xml->appendChild("Settings"));
}

void CVSTHost::unserializeCustom(const QDomLiteElement* xml)
{
    if (!xml) return;
    QMutexLocker locker(&mutex);
    if (!filename().isEmpty())
    {
        VSTPLUGINCLASS->unserialize(xml->elementByTag("Settings"));
        FORMFUNC(CVSTForm)->fillList(VSTPLUGINCLASS->currentBankPreset());
    }
}

void CVSTHost::process()
{
    //qDebug() << "CVSTHost Process";
    VSTPLUGINCLASS->parseMIDI(FetchP(jnMIDIIn));
    VSTPLUGINCLASS->InBuffers.fill(FetchA(jnIn)->data(),m_BufferSize*2);
    int j;
    for (int i=0;i<4;i++)
    {
        CStereoBuffer* b=StereoBuffer(jnOut+i);
        if ((j=i*2)>=OldBuffers) break;
        if (j>=VSTPLUGINCLASS->outputCount()) b->zeroLeftBuffer();
        if (++j>=OldBuffers) break;
        if (j>=VSTPLUGINCLASS->outputCount()) b->zeroRightBuffer();
    }
    OldBuffers=VSTPLUGINCLASS->outputCount();
    if (VSTPLUGINCLASS->process())
    {
        for (int i=0;i<4;i++)
        {
            CStereoBuffer* b=StereoBuffer(jnOut+i);
            if ((j=i*2)<OldBuffers) b->writeLeftBuffer(VSTPLUGINCLASS->OutBuffers.channelPointer(j),VolFactor);
            if (++j<OldBuffers) b->writeRightBuffer(VSTPLUGINCLASS->OutBuffers.channelPointer(j),VolFactor);
        }
    }
    else
    {
        for (int i=0;i<4;i++)
        {
            CStereoBuffer* b=StereoBuffer(jnOut+i);
            if ((j=i*2)<OldBuffers) b->zeroLeftBuffer();
            if (++j<OldBuffers) b->zeroRightBuffer();
        }
    }
}

const QString CVSTHost::currentBankPresetName(const short /*channel*/) const
{
    qDebug() << "bankpreset name";
    return VSTPLUGINCLASS->bankPresetName();
}

const QStringList CVSTHost::presetNames(const int /*bank*/) const
{
    qDebug() << "bankpreset names";
    return VSTPLUGINCLASS->bankPresetNames();
}

void CVSTHost::setCurrentBankPreset(const int index)
{
    qDebug() << "set bankpreset";
    FORMFUNC(CVSTForm)->setBankPreset(index);
}

long CVSTHost::currentBankPreset(const short channel) const
{
    qDebug() << "current bankpreset";
    return VSTPLUGINCLASS->currentBankPreset(channel);
}

void CVSTHost::parseEvent(CMIDIEvent *Event)
{
    //qDebug() << "parseEvent";
    VSTPLUGINCLASS->parseEvent(Event);
}

const QPixmap* CVSTHost::picture() const
{
    qDebug() << "picture";
    return VSTPLUGINCLASS->picture();
}

