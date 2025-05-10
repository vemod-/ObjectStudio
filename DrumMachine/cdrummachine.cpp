#include "cdrummachine.h"
#include "cdrummachineform.h"

#define dmTicksPQ 100

void CDrumMachine::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveOut(0);
    addJackMIDIOut(1);
    addParameter(CParameter::Numeric,"Tempo","BPM",20,300,0,"",100);
    addParameterVolume();
    addParameterPercent("Humanize");
    PatternLength=0;
    //Playing=false;
    PatternType* DefaultPattern=new PatternType("Default",16,DrumMachine::SoundCount,100,0,0);
    Patterns.append(DefaultPattern);
    PatternsInList.append(new PatternListType(DefaultPattern));
    AddSound("kick02.wav","Kick",WG[0]);
    AddSound("snr01.wav","Snare",WG[1]);
    AddSound("hat01.wav","Hi-Hat",WG[2]);
    AddSound("hat19.wav","Open Hi-Hat",WG[3]);
    AddSound("cym01.wav","Cymbal",WG[4]);
    AddSound("tom01.wav","Tom 1",WG[5]);
    AddSound("tom02.wav","Tom 2",WG[6]);
    Reset();
    updateDeviceParameter();
    CalcDuration();
    m_Form=new CDrumMachineForm(this,MainWindow);
}

CDrumMachine::~CDrumMachine()
{
    if (m_Initialized)
    {
        qDeleteAll(PatternsInList);
        qDeleteAll(Patterns);
    }
}

void CDrumMachine::tick()
{
    MIDIBuffer.clear();
    if (m_Playing)
    {
        mSecCount.addBuffer();
        while (mSecCount.moreTicks())
        {
            mSecCount.eatTick();
            if (Counter==dmTicksPQ * BeatCount)
            {
                if (PatternIndex<PatternsInList.size())
                {
                    for (int i = 0;i<DrumMachine::SoundCount;i++)
                    {
                        int vol = PatternsInList[PatternIndex]->Pattern->beat(BeatCount)->Volume[i];
                        if (vol) {
                            vol = qMin<int>(100, vol * humanizeFactor(m_Parameters[pnHumanize]->Value));
                            WG[i].trigger(vol);
                            MIDIBuffer.append(0x8A,MIDINumbers[i],0x00);
                            MIDIBuffer.append(0x9A,MIDINumbers[i],(byte)(vol * 1.27));
                        }
                    }
                    DRUMMACHINEFORM->Flash(PatternIndex,BeatCount);
                    if (++BeatCount >= Patterns[PatternIndex]->numOfBeats())
                    {
                        BeatCount=0;
                        if (PatternsInList[PatternIndex]->Repeats>0)
                        {
                            if (++PatternRepeatCount == PatternsInList[PatternIndex]->Repeats)
                            {
                                PatternLength = (++PatternIndex == PatternsInList.size()) ?
                                    0 : PatternLength=PatternsInList[PatternIndex]->Pattern->numOfBeats()*dmTicksPQ;
                            }
                        }
                    }
                }
            }
            if (++Counter >= PatternLength)
            {
                Counter=0;
                updateDeviceParameter();
            }
        }
    }
    IDevice::tick();
}

CAudioBuffer* CDrumMachine::getNextA(const int ProcIndex)
{
    if (m_Playing)
    {
        m_AudioBuffers[ProcIndex]->zeroBuffer();
        for (CWaveGeneratorX& w : WG)
            m_AudioBuffers[ProcIndex]->addBuffer(w.getNext(), w.Volume*VolumeFactor);
        return m_AudioBuffers[ProcIndex];
    }
    return nullptr;//&m_NullBufferMono;
}

CMIDIBuffer* CDrumMachine::getNextP(const int /*ProcIndex*/)
{
    return &MIDIBuffer;
}

void inline CDrumMachine::updateDeviceParameter(const CParameter* /*p*/)
{
    PatternLength = 0;
    if (PatternIndex < PatternsInList.size())
    {
        const PatternListType* PL=PatternsInList[PatternIndex];
        mSecCount.setTempo((60000000.0/4.0) / (m_Parameters[pnTempo]->PercentValue*PL->Pattern->Tempo),dmTicksPQ);
        PatternLength=PL->Pattern->numOfBeats() * dmTicksPQ;
    }
    VolumeFactor =  m_Parameters[pnVolume]->PercentValue*mixFactorf(DrumMachine::SoundCount);
}

void CDrumMachine::CalcDuration()
{
    CTickCounter c;
    c.reset(dmTicksPQ);
    for (const PatternListType* pl : std::as_const(PatternsInList))
    {
        if (pl->Repeats==0) break;
        c.setTempo((60000000.0/4.0) / (m_Parameters[pnTempo]->PercentValue*pl->Pattern->Tempo),dmTicksPQ);
        c.skipTicks(pl->Pattern->numOfBeats() * dmTicksPQ * pl->Repeats);
    }
    m_MilliSeconds=c.currentmSec();
    m_Ticks=c.currentTick();
}

void CDrumMachine::Reset()
{
    PatternIndex = 0;
    PatternRepeatCount = 0;
    BeatCount = 0;
    Counter = 0;
    mSecCount.reset(dmTicksPQ);
    updateDeviceParameter();
}

void CDrumMachine::play(const bool FromStart)
{
    CalcDuration();
    if (FromStart) Reset();
    //Playing=true;
    IDevice::play(FromStart);
}

void CDrumMachine::pause()
{
    CalcDuration();
    //Playing=false;
    for (int i=0;i<7;i++) MIDIBuffer.append(0x8A,MIDINumbers[i],0x00);
    IDevice::pause();
}

ulong CDrumMachine::milliSeconds() const
{
    return qMax<ulong>(m_MilliSeconds, IDevice::milliSeconds());
}

ulong64 CDrumMachine::samples() const
{
    return qMax<ulong64>(presets.mSecsToSamples(m_MilliSeconds), IDevice::samples());
}

ulong CDrumMachine::ticks() const
{
    return qMax<ulong>(m_Ticks, IDevice::ticks());
}

void CDrumMachine::skip(const ulong64 samples)
{
    Reset();
    while (mSecCount.currentSample() < samples)
    {
        mSecCount.addBuffer();
        while (mSecCount.moreTicks())
        {
            mSecCount.eatTick();
            if (Counter==dmTicksPQ*BeatCount)
            {
                //Debug("Se " + AnsiString(SampleCount) + " " + AnsiString(SamplesPerTick) + " " + AnsiString(Counter) );
                if (PatternIndex<PatternsInList.size())
                {
                    const PatternListType* PLI=PatternsInList[PatternIndex];
                    const PatternType* Pattern=PLI->Pattern;
                    if (++BeatCount>=Pattern->numOfBeats())
                    {
                        BeatCount=0;
                        if (PLI->Repeats>0)
                        {
                            if (++PatternRepeatCount>=PLI->Repeats)
                            {
                                PatternRepeatCount=0;
                                PatternIndex++;
                            }
                        }
                    }
                }
            }
            if (++Counter>=PatternLength)
            {
                Counter=0;
                updateDeviceParameter();
            }
        }
    }
    //Playing = true;
    IDevice::skip(samples);
}
