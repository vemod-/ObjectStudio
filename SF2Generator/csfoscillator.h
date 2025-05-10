#ifndef CSFOSCILLATOR_H
#define CSFOSCILLATOR_H

#include "softsynthsdefines.h"
#include "enabler/enab.h"
#include "csf2file.h"
#include "csfenvelope.h"
#include "csffilter.h"
#include "csflfo.h"

class CSFOscillator : protected IPresetRef
{
private:
    double EnvMod;
    float EnvVol;
    double PosRate;
    float VolumeFactor;
    int pitchWheel;
    int transpose;
    float Aftertouch;
    float m_Tune;
    CSFEnvelope VolEnv;
    CSFEnvelope ModEnv;
    CSFLFO ModLFO;
    CSFLFO VibLFO;
    CSFFilter Filter;
    sfData OscData;
public:
    enum StereoType
    {
        Mono,StereoL,StereoR
    };
    double LeftPan;
    double RightPan;
    ulong RelEndLoop;
    ulong LoopSize;
    ulong RealStart;
    ulong SampleSize;
    short SampleMode;
    bool Silent;
    double Position;
    StereoType Stereo;
    CSFOscillator() : m_Tune(440) {}
    void Init(const sfData* Data, const short MidiNote, const short MidiVelo);
    void setPitchWheel(const int cent) { pitchWheel=cent; }
    void setTranspose(const int steps) { transpose=steps*100; }
    void setAftertouch(const float value) { Aftertouch=value; }
    void setTune(const float tune=440) { m_Tune=tune; }
    void Modulate();
    inline float UpdatePos(CSF2File* SFFile)
    {
        const float Data = SFFile->readSample(RealPos());
        Position += PosRate * EnvMod;
        if (OscData.shInitialFilterFc >= 13500) return Data * VolumeFactor * EnvVol;
        return Filter.GetNext(Data * VolumeFactor * EnvVol);
    }
    inline ulong64 RealPos() const { return ulong64(RealStart+Position); }
    inline void Loop()
    {
        while (Position >= RelEndLoop) Position-=LoopSize;
    }
    inline bool NoLoop()
    {
        if (Position >= SampleSize)
        {
            Silent=true;
            return true;
        }
        return false;
    }
    void Start();
    void Finish();
};

#endif // CSFOSCILLATOR_H
