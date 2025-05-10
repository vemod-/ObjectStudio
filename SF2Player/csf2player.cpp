#include "csf2player.h"
#include "csf2playerform.h"

#undef devicename
#define devicename "SF2Player"

CSF2Player::CSF2Player()
{
    //m_Loading=false;
}

void CSF2Player::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackMIDIIn();
    addJackStereoOut(jnOut);
    startParameterGroup("MIDI", Qt::yellow);
    addParameterMIDIChannel();
    addParameterTranspose();
    addParameterPatchChange();
    endParameterGroup();
    addParameterTune();
    addParameterVolume();
    addFileParameter(&SF2Device);
    m_Form=new CSF2PlayerForm(this,MainWindow);
    //FORMFUNC(CSF2PlayerForm)->fileParameter = m_FileParameter;
    LastTrigger=0;
    LastFreq=0;
    VolumeFactor=mixFactorf(SF2Device::sf2voices);
    SF2Device.reset();
    updateDeviceParameter();
}

void CSF2Player::process()
{
    SF2Device.parseMIDI(FetchP(jnIn));
    bool First = true;
    CStereoBuffer* OutBuffer = StereoBuffer(jnOut);
    for (int i = 0; i < SF2Device.voiceCount(); i++)
    {
        const CStereoBuffer DeviceBuffer(SF2Device.getNext(i));
        if (DeviceBuffer.isValid())
        {
            const float volL=VolumeFactor*SF2Device.volL(SF2Device.voiceChannel(i));
            const float volR=VolumeFactor*SF2Device.volR(SF2Device.voiceChannel(i));
            if (First)
            {
                First=false;
                OutBuffer->writeStereoBuffer(DeviceBuffer.data(),volL,volR);
            }
            else
            {
                OutBuffer->addStereoBuffer(DeviceBuffer.data(),volL,volR);
            }
        }
    }
    if (First) OutBuffer->zeroBuffer();
}

void CSF2Player::pause()
{
    SF2Device.allNotesOff();
    IDevice::pause();
}

void CSF2Player::play(const bool FromStart)
{
    if (FromStart) updateDeviceParameter();
    IDevice::play(FromStart);
}

void inline CSF2Player::updateDeviceParameter(const CParameter* /*p*/)
{
    VolumeFactor=m_Parameters[pnVolume]->PercentValue*mixFactorf(SF2Device::sf2voices);
    SF2Device.setTune(m_Parameters[pnTune]->percentValue());
    SF2Device.setTranspose(m_Parameters[pnTranspose]->Value);
    SF2Device.setChannelMode(m_Parameters[pnMIDIChannel]->Value);
    FORMFUNC(CSF2PlayerForm)->SetPatchResponse();
}

void CSF2Player::setCurrentBankPreset(const int Program)
{
    QMutexLocker locker(&mutex);
    if (m_Form) FORMFUNC(CSF2PlayerForm)->SetProgram(Program);
}

const QString CSF2Player::currentBankPresetName(const short channel) const
{
    const int p=SF2Device.currentBankPreset(channel);
    return "Bank "+QString::number(SF2Device.currentBank(channel))+"\n"+SF2Device.bankPresetName(p);
}

const QStringList CSF2Player::bankNames() const
{
    return SF2Device.bankCaptions();
}

const QStringList CSF2Player::presetNames(const int bank) const
{
    return SF2Device.presetCaptions(bank);
}

long CSF2Player::currentBankPreset(const short channel) const
{
    return SF2Device.currentBankPreset(channel);
}

int CSF2Player::bankPresetNumber(const int bank, const int preset) const
{
    return SF2Device.bankPresetNumber(bank,preset);
}

