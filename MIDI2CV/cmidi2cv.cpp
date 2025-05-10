#include "cmidi2cv.h"

CMIDI2CV::CMIDI2CV()
{
}

void CMIDI2CV::updateDeviceParameter(const CParameter* /*p*/)
{
    CVDevice.Tune=m_Parameters[pnTune]->PercentValue;
    CVDevice.setTranspose(m_Parameters[pnTranspose]->Value);
    CVDevice.setChannelMode(m_Parameters[pnMIDIChannel]->Value);
}

void CMIDI2CV::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    for (int i=0;i<CVDevice::CVVoices;i++)
    {
        addJackModulationOut(jnFrequency+i,"Frequency Out " + QString::number(i+1));
    }
    for (int i=0;i<CVDevice::CVVoices;i++)
    {
        addJackModulationOut(jnVelocity+i,"Velocity Out " + QString::number(i+1));
    }
    addJackMIDIIn();
    startParameterGroup("MIDI", Qt::yellow);
    addParameterMIDIChannel();
    addParameterTranspose();
    endParameterGroup();
    addParameterTune();
    updateDeviceParameter();
}

float CMIDI2CV::getNext(const int ProcIndex)
{
    if (m_Process)
    {
        m_Process=false;
        CVDevice.parseMIDI(FetchP(jnIn));
    }
    if ((ProcIndex>=jnFrequency) && (ProcIndex<jnFrequency+CVDevice::CVVoices))
    {
        return CVDevice.note(ProcIndex-jnFrequency).Voltage + (CVDevice.getPitchbend(ProcIndex-jnFrequency)/1200.0);
    }
    if ((ProcIndex>=jnVelocity) && (ProcIndex<jnVelocity+CVDevice::CVVoices))
    {
        return float(CVDevice.note(ProcIndex-jnVelocity).Velocity*CVDevice.Vol(ProcIndex-jnVelocity));
    }
    return 0;
}
void CMIDI2CV::play(const bool FromStart)
{
    if (FromStart)
    {
        CVDevice.reset();
        updateDeviceParameter();
    }
    IDevice::play(FromStart);
}
void CMIDI2CV::pause()
{
    CVDevice.allNotesOff();
    updateDeviceParameter();
    IDevice::pause();
}
