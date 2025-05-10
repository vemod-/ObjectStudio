#include "crange.h"
//#include <QFileInfo>
#include "cpitchdetect.h"

int FindZeroXing(float* Buffer,int Samples,int Position,bool BackwardsOnly,int NearestValuePointer)
{
    int BSteps=0;
    int FSteps=0;
    int BPos=0;
    int FPos=0;
    float Prev=0;
    bool FActive=false;
    bool BActive=false;
    float NearestValue=0;
    if (NearestValuePointer>-1) NearestValue=Buffer[NearestValuePointer];
    for (int i = Position; i > 0; i--)
    {
        BActive=true;
        if (Prev>0 && Buffer[i]<=0)
        {
            if (fabsf(Prev-NearestValue)>fabsf(Buffer[i]-NearestValue))
            {
                BPos=i;
                if (BackwardsOnly) return BPos;
            }
            else
            {
                BPos=i+1;
            }
            break;
        }
        BSteps++;
        Prev=Buffer[i];
    }
    Prev=1;
    for (int i=Position;i<Samples;i++)
    {
        FActive=true;
        if (Prev<=0 && Buffer[i]>0)
        {
            FPos = (fabsf(Prev-NearestValue)>fabsf(Buffer[i]-NearestValue)) ? i : i-1;
            break;
        }
        FSteps++;
        Prev=Buffer[i];
    }
    if (!FActive)
    {
        if (BActive) return BPos;
    }
    if (!BActive)
    {
        if (FActive) return FPos;
    }
    if (FActive & BActive)
    {
        if (FSteps>BSteps)
        {
            return BPos;
        }
        return FPos;
    }
    return Position;
}

void CSampleKeyRange::pitchDetect(int Tune)
{
    /*
    const CChannelBuffer* Buffer=generator.buffer();
    if (!Buffer) return;
    int HalfLength=16384*2;
    int StartPos=generator.size()-(HalfLength*6);
    while (StartPos<0)
    {
        HalfLength=HalfLength/2;
        StartPos=generator.size()-(HalfLength*6);
    }
    qDebug() << "Halflength" << HalfLength << generator.size() << generator.LP.End << StartPos+(HalfLength*4);
    //CPitchTrackerClass* PT=new CPitchTrackerClass(HalfLength,presets.SampleRate);
*/
    CPitchDetect PD(presets.SampleRate);
    PD.setTune(Tune);
    PD.ProcessBuffer(generator.buffer()->data(),generator.size());
    CPitchDetect::PitchRecord r=PD.CurrentPitchRecord();
    if (r.MidiKey > 0) {
        generator.LP.MIDIKey=r.MidiKey;
        generator.LP.MIDICents=r.MidiCents;
    }
    qDebug() << r.MidiKey << r.MidiCents;
    //PT->InTune=Tune;
    //for (int i=0;i<HalfLength;i++)
    //{
    //    PT->coeffs[i]=Buffer->at(StartPos+(i*4),0);
    //}
    //PT->process();
    //generator.LP.MIDINote=PT->CurrentNote-24;
    //generator.LP.Tune=factor2Centf(diff2Factorf(PT->CurrentDiff));//PT->CurrentDiff*1000.0;

    //delete PT;
}

void CSampleKeyRange::autoLoop(int Cycles)
{
    float Freq=MIDIkey2Freqf(generator.LP.MIDIKey,440.f,generator.LP.MIDICents);
    float CycleLength=float(presets.SampleRate)/Freq;
    float Length=CycleLength*Cycles;
    generator.LP.LoopType=CWaveGenerator::ltForward;
    generator.LP.LoopEnd=FindZeroXing(generator.channelPointer(0),generator.size(),generator.LP.End-CycleLength,true,-1);
    generator.LP.LoopStart=FindZeroXing(generator.channelPointer(0),generator.size(),generator.LP.LoopEnd-Length,false,generator.LP.LoopEnd);
}

void CSampleKeyRange::autoTune()
{
    float Length=generator.LP.LoopEnd-generator.LP.LoopStart;
    if (Length<=0) return;
    float Freq=MIDIkey2Freqf(generator.LP.MIDIKey,440.f);
    float CycleLength=float(presets.SampleRate)/Freq;
    float Cycles=qMax(1,qRound(Length/CycleLength));
    float ActualFreq=(float(presets.SampleRate)/Length)*Cycles;
    float FreqFactor=Freq/ActualFreq;
    generator.LP.MIDICents=factor2Centf(FreqFactor);
    qDebug() << "Freq" << Freq << "CycleLength" << CycleLength << "Cycles" << Cycles << "Looplength" << Length << "ActualFreq" << ActualFreq << "FreqFactor" << FreqFactor << "Cents" << generator.LP.MIDICents;
}

void CSampleKeyRange::autoFix(int Cycles, int Tune)
{
    pitchDetect(Tune);
    autoLoop(Cycles);
    autoTune();
}

CSampleKeyRange::CSampleKeyRange(const QString& WavePath,int Upper,int Lower)
{
    QMutexLocker locker(&mutex);
    qDebug() << "Create TCSampleKeyRange";
    parameters.reset(Upper,Lower);
    PlayVol=1;
    changePath(WavePath);
}

CSampleKeyRange::CSampleKeyRange(const QString& WavePath,CSampleKeyRange::RangeParams RangeParams)
{
    QMutexLocker locker(&mutex);
    qDebug() << "Create TCSampleKeyRange";
    parameters=RangeParams;
    PlayVol=1;
    changePath(WavePath);
}

CSampleKeyRange::~CSampleKeyRange()
{
    qDebug() << "Delete TCSampleKeyRange";
}

void CSampleKeyRange::changePath(const QString& WavePath)
{
    if (WavePath==fileName) return;
    QMutexLocker locker(&mutex);
    if (QFileInfo::exists(WavePath))
    {
        generator.load(WavePath);
        generator.LP.reset(generator.size());
        fileName=WavePath;
    }
}

