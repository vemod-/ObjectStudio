#include "csequenser.h"
#include "csequenserform.h"

#define seqTicksPQ 100

CSequenser::CSequenser()
{
}

void CSequenser::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackMIDIOut(jnMIDIOut);
    addParameter(CParameter::Numeric,"Tempo","BPM",20,300,0,"",100);
    addParameter(CParameter::Numeric,"MIDI Channel","",1,16,0,"",1);
    addParameterPercent("Humanize");
    MIDIBuffer=new CMIDIBuffer();
    PatternLength=0;
    //Playing=false;
    PatternType* DefaultPattern=new PatternType("Default",16);
    Patterns.append(DefaultPattern);
    PatternsInList.append(new PatternListType(DefaultPattern));
    Reset();
    CalcDuration();
    updateDeviceParameter();
    m_Form=new CSequenserForm(this,MainWindow);
}

CSequenser::~CSequenser()
{
    if (m_Initialized)
    {
        qDeleteAll(PatternsInList);
        qDeleteAll(Patterns);
    }
}

void CSequenser::tick()
{
    if (!m_Playing) {
        IDevice::tick();
        return;
    }
    MIDIBuffer->clear();
    mSecCount.addBuffer();
    while (mSecCount.moreTicks())
    {
        mSecCount.eatTick();
        if (Counter==NextBeat)
        {
            if (PatternIndex<PatternsInList.size())
            {
                const PatternListType* PLI=PatternsInList[PatternIndex];
                const PatternType* Pattern=PLI->Pattern;
                const BeatType* Beat=Pattern->beat(BeatCount);
                if (Beat->Length[0]>0)
                {
                    CurrentLength=Beat->Length[0];
                    CurrentPitch=Beat->Pitch[0];
                    if (CurrentPitch>0)
                    {
                        int vol = Beat->Volume[0];
                        vol = qMin<int>(100, vol * humanizeFactor(m_Parameters[pnHumanize]->Value));
                        MIDIBuffer->append(m_Parameters[pnMIDIChannel]->Value + 0x90 - 1,CurrentPitch,(byte)(vol * 1.27));
                    }
                }
                else
                {
                    CurrentLength=0;
                }
                FORMFUNC(CSequenserForm)->Flash(PatternIndex,BeatCount);
                NextBeat=seqTicksPQ * (BeatCount+1);
                NextStop=(seqTicksPQ * BeatCount) + CurrentLength;
                if (++BeatCount>=Pattern->numOfBeats())
                {
                    BeatCount=0;
                    NextBeat=0;
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
        if (Counter==NextStop)
        {
            MIDIBuffer->append(m_Parameters[pnMIDIChannel]->Value + 0x90 - 1,CurrentPitch,0);
        }
        if (++Counter>=PatternLength)
        {
            Counter=0;
            updateDeviceParameter();
        }
    }
    IDevice::tick();
}

CMIDIBuffer* CSequenser::getNextP(int /*ProcIndex*/)
{
    if (!m_Playing)
    {
        return nullptr;
    }
    return MIDIBuffer;
}

void inline CSequenser::updateDeviceParameter(const CParameter* /*p*/)
{
    PatternLength=0;
    if (PatternIndex < PatternsInList.size())
    {
        const PatternListType* PL=PatternsInList[PatternIndex];
        const PatternType* P=PL->Pattern;
        mSecCount.setTempo((60000000.0/4.0) / (m_Parameters[pnTempo]->PercentValue*P->Tempo),seqTicksPQ);
        PatternLength=P->numOfBeats() * seqTicksPQ;
    }
}

void CSequenser::CalcDuration()
{
    CTickCounter c;
    c.reset(seqTicksPQ);
    for (const PatternListType* pl : std::as_const(PatternsInList))
    {
        if (pl->Repeats==0) break;
        c.setTempo((60000000.0/4.0) / (m_Parameters[pnTempo]->PercentValue*pl->Pattern->Tempo),seqTicksPQ);
        c.skipTicks(pl->Pattern->numOfBeats() * seqTicksPQ * pl->Repeats);
    }
    m_MilliSeconds=c.currentmSec();
    m_Ticks=c.currentTick();
}

void CSequenser::Reset()
{
    CurrentPitch = 0;
    CurrentLength = 0;
    PatternIndex = 0;
    PatternRepeatCount = 0;
    BeatCount = 0;
    NextBeat = 0;
    NextStop = 0;
    Counter = 0;
    //SampleCount = 0;
    mSecCount.reset(seqTicksPQ);
    updateDeviceParameter();
    MIDIBuffer->clear();
}

void CSequenser::play(const bool FromStart)
{
    CalcDuration();
    if (FromStart)
    {
        Reset();
    }
    //Playing=true;
    IDevice::play(FromStart);
}

void CSequenser::pause()
{
    CalcDuration();
    //Playing=false;
    IDevice::pause();
}

ulong CSequenser::milliSeconds() const
{
    return qMax<ulong>(m_MilliSeconds, IDevice::milliSeconds());
}

ulong CSequenser::ticks() const
{
    return qMax<ulong>(m_Ticks, IDevice::ticks());
}

ulong64 CSequenser::samples() const
{
    return qMax<ulong64>(presets.mSecsToSamples(m_MilliSeconds), IDevice::samples());
}

void CSequenser::skip(const ulong64 samples)
{
    Reset();
    while (mSecCount.currentSample() < samples)
    {
        mSecCount.addBuffer();
        while (mSecCount.moreTicks())
        {
            mSecCount.eatTick();
            if (Counter==NextBeat)
            {
                if (PatternIndex<PatternsInList.size())
                {
                    const PatternListType* PLI=PatternsInList[PatternIndex];
                    const PatternType* Pattern=PLI->Pattern;
                    NextBeat=seqTicksPQ * (BeatCount+1);
                    if (++BeatCount>=Pattern->numOfBeats())
                    {
                        BeatCount=0;
                        NextBeat=0;
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
    IDevice::skip(samples);
}
