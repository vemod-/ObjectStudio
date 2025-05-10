#include "cpitchtracker.h"

CPitchTracker::CPitchTracker() : PD(presets.SampleRate)//, m_FFTTracker(presets.SampleRate)
{
}

void CPitchTracker::init(const int Index, QWidget* MainWindow) {
    m_Name=devicename;
    LastNote=0;
    IDevice::init(Index,MainWindow);
    addJackWaveIn();
    addJackModulationOut(jnFrequencyOut,"Frequency Out");
    addJackModulationOut(jnMIDIFreqOut,"MIDI Frequency Out");
    addJackMIDIOut(jnMIDIOut);
    addJackModulationOut(jnDiffOut,"Difference Out");
    addParameterPercent("Threshold");
    addParameterTune();
    addParameter(CParameter::Numeric,"Max Frequency","Hz",5000,presets.HalfRate,0,"",presets.HalfRate*0.5);
    addParameter(CParameter::Numeric,"Rate","mSec",10,1000,0,"",10);
    addParameter(CParameter::Numeric,"Overlap","Samples",0,240,0,"",0);
    tuneFactor=1;
    updateDeviceParameter();
}

float CPitchTracker::getNext(const int ProcIndex) {
    float Retval=0;
    if (m_Process)
    {
        m_Process=false;
        process();
    }
    CPitchDetect::PitchRecord r=PD.CurrentPitchRecord();
    if (ProcIndex==jnFrequencyOut)
    {
        //Retval=PT.CurrentFreq/BufferDivide;
        Retval=freq2voltagef(r.Pitch);
    }
    if (ProcIndex==jnMIDIFreqOut)
    {
        //Retval=PT.CurrentMIDIFreq/BufferDivide;
        Retval=MIDIkey2voltagef(r.MidiKey);//freq2voltagef(MIDIkey2Freqf(r.MidiNote));
    }
    if (ProcIndex==jnDiffOut)
    {
        //Retval=PT.CurrentDiff;
        Retval=r.MidiCents/1200.f;
    }
    /*
    if (BufferFill==0)
    {
        if (NewBufferDivide!=BufferDivide)
        {
            BufferDivide=NewBufferDivide;
        }
    }
    */
    return Retval;
}

void CPitchTracker::process() {
    const CMonoBuffer* Input = FetchAMono(jnIn);
    if (!Input->isValid()) return;
    QMutexLocker locker(&mutex);
    //m_FFTTracker.process(Input->data(),presets.ModulationRate);
    PD.ProcessBuffer(Input->data(),presets.ModulationRate);
    //m_BAC.appendBuffer(Input->data(),presets.ModulationRate);
}

CMIDIBuffer *CPitchTracker::getNextP(int) {
    if (m_Process)
    {
        m_Process=false;
        process();
    }
    MIDIBuffer.clear();
    CPitchDetect::PitchRecord r=PD.CurrentPitchRecord();
    if (r.MidiKey)
    {
        if (r.MidiKey != LastNote)
        {
            if (LastNote)
            {
                //LastNote Off
                MIDIBuffer.append(0x80,LastNote,0);
            }
            if (r.MidiKey)
            {
                qDebug() << r.Pitch << r.MidiKey << r.MidiCents << MIDIkey2Freqf(r.MidiKey) << r.MidiCents/1200.f;
                MIDIBuffer.append(0x90,r.MidiKey,127);
            }
        }

        LastNote=r.MidiKey;
    }
    else
    {
        if (LastNote)
        {
            MIDIBuffer.append(0x80,LastNote,0);

            LastNote=0;
        }
    }
    return &MIDIBuffer;
}

void CPitchTracker::updateDeviceParameter(const CParameter* /*p*/) {
    PD.setTune(m_Parameters[pnTune]->PercentValue);
    PD.setDetectLevelThreshold(m_Parameters[pnThreshold]->PercentValue);
    PD.setMaxDetectFrequency(m_Parameters[pnMaxFreq]->Value);
    PD.setPitchRecordsPerSecond(1000/m_Parameters[pnRate]->Value);
    PD.setOverlap(m_Parameters[pnOverlap]->Value);
}
