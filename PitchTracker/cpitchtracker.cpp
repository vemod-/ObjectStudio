#include "cpitchtracker.h"

CPitchTracker::CPitchTracker() : PD(presets.SampleRate)//, m_FFTTracker(presets.SampleRate)
{
}

void CPitchTracker::init(const int Index, QWidget* MainWindow) {
    m_Name=devicename;
    LastNote=0;
    IDevice::init(Index,MainWindow);
    addJackMonoIn();
    addJackModulationOut(frequencyout,"Frequency Out");
    addJackModulationOut(midifrequencyout,"MIDI Frequency Out");
    addJackMIDIOut(midiout);
    addJackModulationOut(diffmodulationout,"Difference Out");
    addJackModulationOut(CPitchTracker::corrmodulationout,"Correction Out");
    addParameterPercent("Threshold");
    addParameterTune();
    addParameter(CParameter::Numeric,"Max Frequency","Hz",5000,presets.HalfRate,0,"",presets.HalfRate * 0.5);
    addParameter(CParameter::Numeric,"Rate","mSec",10,1000,0,"",10);
    startParameterGroup("Correction");
    addParameterPercent("Glide");
    addParameter(CParameter::Numeric,"Slack","Cents",0,100,0,"",2);
    endParameterGroup();
    //addParameter(CParameter::Numeric,"Overlap","Samples",0,240,0,"",0);
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
    CYIN::PitchRecord r = PD.CurrentPitchRecord();
    if (ProcIndex==frequencyout)
    {
        //Retval=PT.CurrentFreq/BufferDivide;
        Retval=freq2voltagef(r.Pitch);
    }
    if (ProcIndex==midifrequencyout)
    {
        //Retval=PT.CurrentMIDIFreq/BufferDivide;
        Retval=MIDIkey2voltagef(r.MidiKey);//freq2voltagef(MIDIkey2Freqf(r.MidiNote));
    }
    if (ProcIndex==diffmodulationout)
    {
        //Retval=PT.CurrentDiff;
        Retval=r.MidiCents/1200.f;
    }
    if (ProcIndex==corrmodulationout)
    {
        //Retval=PT.CurrentDiff;
        Retval=PD.correctionCents()/1200.f;
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
    const CMonoBuffer* Input = FetchAMono(monoin);
    if (!Input->isValid()) return;
    //QMutexLocker locker(&mutex);
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
    CYIN::PitchRecord r = PD.CurrentPitchRecord();
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
                //qDebug() << r.Pitch << r.MidiKey << r.MidiCents << MIDIkey2Freqf(r.MidiKey) << r.MidiCents/1200.f;
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
    PD.setGlide(m_Parameters[pnGlide]->Value);
    PD.setDetectSlack(m_Parameters[pnSlack]->Value);
    //PD.setOverlap(m_Parameters[pnOverlap]->Value);
}
