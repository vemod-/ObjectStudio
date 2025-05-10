#include "cvocoder.h"

CVocoder::CVocoder() : PD(presets.SampleRate), PS(presets.SampleRate)
{
    PS.setPolyphony(8);
    PD.setMaxDetectFrequency(3000);
    PD.setPitchRecordsPerSecond(40);
}

void CVocoder::updateDeviceParameter(const CParameter* /*p*/)
{
    CVDevice.Tune=m_Parameters[pnTune]->PercentValue;
    CVDevice.setTranspose(m_Parameters[pnTranspose]->Value);
    CVDevice.setChannelMode(m_Parameters[pnMIDIChannel]->Value);
    PS.setOverSampling(1 << m_Parameters[pnOversampling]->Value);
    glider.setGlide(m_Parameters[pnGlide]->Value);
}

void CVocoder::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveOut(jnOut);
    addJackWaveIn();
    addJackMIDIIn();
    startParameterGroup("MIDI", Qt::yellow);
    addParameterMIDIChannel();
    addParameterTranspose();
    endParameterGroup();
    addParameterTune();
    addParameterOffOn("AutoTune");
    addParameterPercent("Glide");
    addParameterSelect("Oversampling","1§2§4§8§16§32",3);
    addParameterPercent("Effect",50);
    updateDeviceParameter();
}

CAudioBuffer* CVocoder::getNextA(const int /*ProcIndex*/)
{
    CVDevice.parseMIDI(FetchP(jnMIDIIn));
    inBuffer=FetchAMono(jnIn);
    if (!inBuffer->isValid()) return nullptr;
    if (m_Parameters[pnEffect]->Value == 0)
    {
        m_AudioBuffers[jnOut]->writeBuffer(inBuffer);
    }
    else
    {
        PD.ProcessBuffer(inBuffer->data(),int(presets.ModulationRate));
        const CPitchDetect::PitchRecord r = PD.CurrentPitchRecord();
        int c = r.MidiCents*m_Parameters[pnAutotune]->Value;
        if (r.MidiKey > 0)
        {
            const int mc = (r.MidiKey*100) + (r.MidiCents*m_Parameters[pnAutotune]->Value);
            if (m_Parameters[pnGlide]->Value)
            {
                if (mc != m_lastMIDICent) glider.setTargetCent(c);
                c = glider.currentCent();
            }
            m_lastKey = r.MidiKey;
            m_lastMIDICent = mc;
        }
        for (int i = 0; i < 8; i++)
        {
            if (m_lastKey > 0)
            {
                if (CVDevice.note(i).MIDIKey > 0)
                {
                    m_shiftFactor[i]=cent2Factor(((CVDevice.note(i).MIDIKey-m_lastKey)*100) + c + tune2Cent(m_Parameters[pnTune]->PercentValue));
                }
                else
                {
                    m_shiftFactor[i]=double(m_Parameters[pnTune]->PercentValue)/440.0;
                }
                m_scale[i]=CVDevice.note(i).Velocity;
            }
            else
            {
                m_shiftFactor[i]=0;
                m_scale[i]=0;
            }
        }
        if (m_Parameters[pnEffect]->Value == 100)
        {
            m_AudioBuffers[jnOut]->writeBuffer(PS.process(m_shiftFactor,m_scale,presets.ModulationRate, inBuffer->data()));
        }
        else
        {
            m_AudioBuffers[jnOut]->writeBuffer(PS.process(m_shiftFactor,m_scale,presets.ModulationRate, inBuffer->data()),m_Parameters[pnEffect]->PercentValue);
            m_AudioBuffers[jnOut]->addBuffer(inBuffer,m_Parameters[pnEffect]->DryValue);
        }
    }
    return m_AudioBuffers[jnOut];
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
