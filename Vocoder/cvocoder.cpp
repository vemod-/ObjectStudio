#include "cvocoder.h"

CVocoder::CVocoder() : PD(presets.SampleRate), PS(presets.SampleRate,presets.ModulationRate,8)
{
    PD.setMaxDetectFrequency(3000);
    PD.setPitchRecordsPerSecond(10);
}

void CVocoder::updateDeviceParameter(const CParameter* /*p*/)
{
    CVDevice.Tune=m_Parameters[pnTune]->PercentValue;
    CVDevice.setTranspose(m_Parameters[pnTranspose]->Value);
    CVDevice.setChannelMode(m_Parameters[pnMIDIChannel]->Value);
    PS.setOverSampling(1 << m_Parameters[pnOversampling]->Value);
    PD.setGlide(m_Parameters[pnGlide]->Value);
    PD.setDetectSlack(m_Parameters[pnSlack]->Value);
    PD.setDetectLevelThreshold(m_Parameters[pnThreshold]->PercentValue);
}

void CVocoder::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackMonoOut(monoout);
    addJackMonoIn();
    addJackMIDIIn();
    startParameterGroup("MIDI", Qt::yellow);
    addParameterMIDIChannel();
    addParameterTranspose();
    endParameterGroup();
    addParameterTune();
    startParameterGroup();
    addParameterOffOn("AutoTune");
    addParameterPercent("Glide");
    addParameter(CParameter::Numeric,"Slack","Cents",0,100,0,"",2);
    addParameterPercent("Threshold",10);
    endParameterGroup();
    addParameterSelect("Oversampling","1§2§4§8§16§32",3);
    addParameterPercent("Effect",50);
    updateDeviceParameter();
}

CAudioBuffer* CVocoder::getNextA(const int /*ProcIndex*/)
{
    CVDevice.parseMIDI(FetchP(midiin));
    inBuffer=FetchAMono(monoin);
    if (!inBuffer->isValid()) return nullptr;
    if (m_Parameters[pnEffect]->Value == 0)
    {
        m_AudioBuffers[monoout]->writeBuffer(inBuffer);
    }
    else
    {
        PD.ProcessBuffer(inBuffer->data(),int(presets.ModulationRate));
        const CYIN::PitchRecord r = PD.CurrentPitchRecord();
        int target = (m_Parameters[pnAutotune]->Value) ? PD.correctionCents() : 0;
        if (r.MidiKey > 0) m_lastKey = r.MidiKey;
        for (int i = 0; i < 8; i++)
        {
            if (m_lastKey > 0)
            {
                if (CVDevice.note(i).MIDIKey > 0)
                {
                    m_shiftFactor[i]=cent2Factor(((CVDevice.note(i).MIDIKey-m_lastKey)*100) + target + tune2Cent(m_Parameters[pnTune]->PercentValue));
                }
                else
                {
                    m_shiftFactor[i]=double(m_Parameters[pnTune]->PercentValue)/440.0;
                }
                m_scale[i] = CVDevice.note(i).Velocity * m_Parameters[pnEffect]->PercentValue;
            }
            else
            {
                m_shiftFactor[i]=0;
                m_scale[i]=0;
            }
        }
        if (m_Parameters[pnEffect]->Value == 100)
        {
            PS.process(m_shiftFactor,m_scale,inBuffer->data(),m_AudioBuffers[monoout]->data());
        }
        else
        {
            PS.process(m_shiftFactor,m_scale,inBuffer->data(),m_AudioBuffers[monoout]->data());
            m_AudioBuffers[monoout]->addBuffer(inBuffer,m_Parameters[pnEffect]->DryValue);
        }
    }
    return m_AudioBuffers[monoout];
}

void CVocoder::play(const bool FromStart)
{
    if (FromStart)
    {
        m_lastKey = 0;
        CVDevice.reset();
        updateDeviceParameter();
    }
    IDevice::play(FromStart);
}

void CVocoder::pause()
{
    CVDevice.allNotesOff();
    updateDeviceParameter();
    IDevice::pause();
}
