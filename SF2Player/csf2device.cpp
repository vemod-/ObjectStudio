#include "csf2device.h"

CSF2Device::CSF2Device() :
    ISoundDevice()
{
    drumBank=0;
    loaded=false;
    m_IsGM=true;
    lastChannel=0;
}

CSF2Device::~CSF2Device()
{
}

void CSF2Device::noteOn(short channel, short pitch, short velocity)
{
    if (channelSettings[channel].portNote)
    {
        for (CSF2Generator& g : SF2Generator)//for (int i=0;i<SF2Device::sf2voices;i++)
        {
            if (g.matches(channel,channelSettings[channel].portNote))
            {
                g.ID=pitch;
                g.addPortamento(pitch-channelSettings[channel].portNote);
                break;
            }
        }
    }
    else
    {
        int FreeIndex=-1;
        for (int i=0;i<SF2Device::sf2voices;i++)
        {
            if (SF2Generator[i].ID==0)
            {
                FreeIndex=i;
                break;
            }
        }
        if (FreeIndex>-1)
        {
            SF2Generator[FreeIndex].setID(channel,pitch);
            SF2Generator[FreeIndex].MidiBank=channelSettings[channel].bank;
            SF2Generator[FreeIndex].MidiPreset=channelSettings[channel].patch;
            SF2Generator[FreeIndex].startNote(pitch,velocity & 0x7F);
        }
    }
    channelSettings[channel].portNote=0;
}

void CSF2Device::noteOff(short channel, short pitch)
{
    for (CSF2Generator& g : SF2Generator)//for (int i=0;i<SF2Device::sf2voices;i++)
    {
        if ((channel) == g.channel)
        {
            if (channelSettings[channel].pedal)
            {
                channelSettings[channel].pedalnotes.append(pitch);
            }
            else if (pitch==g.ID)
            {
                g.endNote();
                break;
            }
        }
    }
}

void CSF2Device::aftertouch(const short channel, const short pitch, const short value)
{
    for (CSF2Generator& g : SF2Generator)//for (int i=0;i<SF2Device::sf2voices;i++)
    {
        if (g.matches(channel,pitch))
        {
            g.setAftertouch(value);
        }
    }
}

void CSF2Device::patch(const short channel, const short value)
{
    channelSettings[channel].patch=value;
}

void CSF2Device::controller(short channel, short controller, short value)
{
    switch (controller)
    {
    case 0:
    case 32:
    case 121:
        lastChannel=channel;
        break;
    default:
        break;
    }
    switch (controller)
    {
    case 0: // Bank select
            if ((channel != 9) | (!m_IsGM))
            {
                if (SF2Generator[0].bankNumbers().contains(value))
                {
                    channelSettings[channel].bank=value;
                }
            }
        break;
    case 121: // All controllers off
        if (m_PatchResponse | m_IsGM)
        {
            channelSettings[channel].resetAll();
            channelSettings[9].bank=drumBank;
        }
        else
        {
            channelSettings[channel].reset();
        }
        break;
    default:
        ISoundDevice::controller(channel,controller,value);
        break;
    }
}

void CSF2Device::allNotesOff()
{
    for (CSF2Generator& g : SF2Generator)//for (int i=0;i<SF2Device::sf2voices;i++)
    {
        if (!g.finished)
        {
            g.endNote();
        }
    }
}

float* CSF2Device::getNext(const int voice)
{
    SF2Generator[voice].setPitchWheel(channelSettings[voiceChannel(voice)].pitchWheel);
    return SF2Generator[voice].getNext();
}

short CSF2Device::voiceChannel(const int voice) const
{
    return SF2Generator[voice].channel;
}

int CSF2Device::voiceCount() const
{
    return SF2Device::sf2voices;
}

void CSF2Device::reset()
{
    findDrumBank();
    for (CSF2Generator& g : SF2Generator)//for (int i=0;i<SF2Device::sf2voices;i++)
    {
        g.resetPortamento();
        g.ID=0;
        g.channel=0;
    }
    for (ChannelData& d : channelSettings)//for (int i = 0; i < 16; i++)
    {
        d.resetAll();
    }
    if (m_IsGM) channelSettings[9].bank=drumBank;
    lastChannel=0;
}

const QString CSF2Device::bankPresetName(const int program) const
{
    return SF2Generator[0].bankPresetName(program);
}

int CSF2Device::bankPresetNumber(const int bank, const int preset) const
{
    return SF2Generator[0].bankPresetNumber(bank,preset);
}

int CSF2Device::banknumber(const int program) const
{
    return SF2Generator[0].banknumber(program);
}

int CSF2Device::presetnumber(const int program) const
{
    return SF2Generator[0].presetnumber(program);
}

int CSF2Device::currentBank(const short channel) const
{
    return  (channel < 0) ? channelSettings[lastChannel].bank : channelSettings[channel].bank;
}

int CSF2Device::currentPreset(const short channel) const
{
    return  (channel < 0) ? channelSettings[lastChannel].patch : channelSettings[channel].patch;
}

int CSF2Device::currentBankPreset(const short channel) const
{
    return bankPresetNumber(currentBank(channel),currentPreset(channel));
}

void CSF2Device::setBankPreset(const int bank, const int preset)
{
    for (ChannelData& d : channelSettings)//for (int i=0;i<16;i++)
    {
        d.bank=bank;
        d.patch=preset;
    }
    if (m_IsGM) channelSettings[9].bank=drumBank;
}

void CSF2Device::setBankPreset(const int program)
{
    setBankPreset(banknumber(program),presetnumber(program));
}

bool CSF2Device::loadFile(const QString& filename)
{
    QMutexLocker locker(&mutex);
    allNotesOff();
    loaded=false;
    CSF2Generator* sf=&SF2Generator[0];
    qDebug() << "SF2Device load" << filename;
    if (sf->load(filename))
    {
        for (int i=1;i<SF2Device::sf2voices;i++) SF2Generator[i].load(filename);
        loaded=true;
        findDrumBank();
        if (m_IsGM) channelSettings[9].bank=drumBank;
    }
    return loaded;
}

void CSF2Device::findDrumBank()
{
    drumBank=0;
    if (loaded)
    {
        for (const int& bn : SF2Generator[0].bankNumbers()) if (bn > drumBank) drumBank=bn;
    }
}

const QString CSF2Device::bankCaption(const int bank) const
{
    return QString(QStringLiteral("000")+QString::number(bank)).right(3);
}

const QString CSF2Device::presetCaption(const int bank, const int preset) const
{
    return bankCaption(preset)+" "+SF2Generator[0].presetName(bank,preset);
}
const QStringList CSF2Device::bankCaptions() const
{
    QStringList l;
    for (const int& i : SF2Generator[0].bankNumbers()) l.append(bankCaption(i));
    l.sort();
    return l;
}

const QStringList CSF2Device::presetCaptions(const int bank) const
{
    QStringList l;
    for (const int& i : SF2Generator[0].presetNumbers(bank)) l.append(presetCaption(bank,i));
    l.sort();
    return l;
}

void CSF2Device::setTune(const float tune)
{
    for (CSF2Generator& g : SF2Generator) g.setTune(tune);
}
