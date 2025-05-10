#ifndef CWAVEGENERATOR_H
#define CWAVEGENERATOR_H

#include "cwavefile.h"
#include "qdomlite.h"
#include <QGraphicsScene>
#include "cpresets.h"

class CWaveGenerator : protected IPresetRef
{
public:
    enum LoopTypeEnum
    {ltForward,ltAlternate,ltXFade};
    enum SampleStates
    {ssNone,ssSilent,ssStarting,ssLooping,ssEnding};
    class LoopParameters
    {
    public:
        LoopParameters() { reset(); }
        ulong64 Start;
        ulong64 End;
        ulong64 LoopStart;
        ulong64 LoopEnd;
        int MIDIKey;
        int MIDICents;
        ulong64 FadeIn;
        ulong64 FadeOut;
        int Volume;
        int XFade;
        LoopTypeEnum LoopType;
        uint origRate;
        double Speed;
        double PitchShift;
        void reset(ulong64 len=0)
        {
            Start=0;
            End=len;
            LoopStart=0;
            LoopEnd=0;
            MIDIKey=69;
            MIDICents=0;
            LoopType=CWaveGenerator::ltForward;
            FadeIn=0;
            FadeOut=0;
            XFade=0;
            Volume=100;
            Speed = 1;
            PitchShift = 0;
            origRate=CPresets::presets().SampleRate;
        }
        void convertRate(double oldSampleRate=0)
        {
            if (isZero(oldSampleRate)) oldSampleRate=origRate;
            if (origRate == CPresets::presets().SampleRate) return;
            double newSampleRate=CPresets::presets().SampleRate;
            const ldouble rateFactor=newSampleRate/oldSampleRate;
            Start*=rateFactor;
            End*=rateFactor;
            LoopStart*=rateFactor;
            LoopEnd*=rateFactor;
            FadeIn*=rateFactor;
            FadeOut*=rateFactor;
            origRate=CPresets::presets().SampleRate;
        }
        void unserialize(const QDomLiteElement* xml,ulong64 end=0)
        {
            if (!xml) return;
            Volume=xml->attributeValueInt("Volume",100);
            Start=xml->attributeValueULongLong("Start",0);
            End=xml->attributeValueULongLong("End",end);
            LoopStart=xml->attributeValueULongLong("LoopStart",end);
            LoopEnd=xml->attributeValueULongLong("LoopEnd",end);
            FadeIn=xml->attributeValueULongLong("FadeIn",0);
            FadeOut=xml->attributeValueULongLong("FadeOut",0);
            MIDIKey=xml->attributeValueInt("MIDINote",60);
            MIDICents=xml->attributeValueInt("Tune",0);
            LoopType=CWaveGenerator::LoopTypeEnum(xml->attributeValueInt("LoopType",0));
            XFade=xml->attributeValueInt("XFade",0);
            Speed=xml->attributeValue("Speed",1);
            PitchShift=xml->attributeValue("PitchShift",0);
            origRate=xml->attributeValueInt("OrigRate",44100);
            convertRate();
        }
        void serialize(QDomLiteElement* xml) const
        {
            xml->setAttribute("Volume",Volume);
            xml->setAttribute("Start",Start);
            xml->setAttribute("End",End);
            xml->setAttribute("LoopStart",LoopStart);
            xml->setAttribute("LoopEnd",LoopEnd);
            xml->setAttribute("FadeIn",FadeIn);
            xml->setAttribute("FadeOut",FadeOut);
            xml->setAttribute("MIDINote",MIDIKey);
            xml->setAttribute("Tune",MIDICents);
            xml->setAttribute("LoopType",int(LoopType));
            xml->setAttribute("XFade",XFade);
            xml->setAttribute("Speed",Speed);
            xml->setAttribute("PitchShift",PitchShift);
            xml->setAttribute("OrigRate",int(CPresets::presets().SampleRate));
        }
        inline ulong64 playLength() const { return ldouble(End-Start)/Speed; }
        inline float fadeVolume(ldouble Counter) const
        {
            float Vol=Volume*0.01f;
            if (Counter < FadeIn/Speed)
            {
                Vol*=Counter/(FadeIn/Speed);
            }
            if (Counter>playLength()-(FadeOut/Speed))
            {
                Vol*=(playLength()-Counter)/(FadeOut/Speed);
            }
            return lin2expf(Vol);
        }
    };
    CWaveGenerator();
    ~CWaveGenerator();
    bool load(const QString& path, uint SampleRate=CPresets::presets().SampleRate, uint BufferSize=CPresets::presets().ModulationRate);
    float* getNext();
    float* getNextSpeed(const double Speed);
    float* getNextRate(const double RateOverride);
    float* getNextFreq(const double Frequency=0);
    LoopParameters LP;
    inline ulong64 size() { return m_Size; }
    void reset();
    void release();
    inline float* channelPointer(const uint Channel) const {
        return (!WF) ? nullptr : WF->data.channelPointer(Channel);
    }
    inline CChannelBuffer* buffer() const {
        return (!WF) ? nullptr : &WF->data;
    }
    void skipTo(const ulong64 Ptr);
    inline uint channels() const { return m_Audio.channels(); }
    inline uint origRate() const { return WF->frequency; }
    void paint(QGraphicsScene& Scene, QRect waveRect, QRect visibleRect, double zoom, LoopParameters* LP) {
        paint(Scene,this,waveRect,visibleRect,zoom,LP);
    }
    static void paint(QGraphicsScene& Scene, CWaveGenerator* WG, QRect waveRect, QRect visibleRect, double zoom, CWaveGenerator::LoopParameters*LP) {
        if (!waveRect.intersects(visibleRect)) return;
        QRect r = waveRect.intersected(visibleRect);
        QPen p(Qt::black);
        if (WG->size())
        {
            ulong64 Start = 0;
            ulong64 End = WG->size();
            ldouble ZoomValue = 1.0 / zoom;
            if (LP) {
                Start = LP->Start;
                End = LP->End;
                ZoomValue *= LP->Speed;
            }
            const CChannelBuffer* Buffer = WG->buffer();
            if (Buffer)
            {
                for (uint channel = 0; channel < WG->channels(); channel++)
                {
                    float YFactor = waveRect.height() / (2*WG->channels());
                    int HalfHeight = waveRect.top() + (YFactor + (YFactor * channel * 2));
                    ldouble sample = Start;
                    if (r.left()  > waveRect.left()) sample += ZoomValue * (r.left() - waveRect.left());
                    long zeroCount = 0;
                    long64 lastSample = sample;
                    int x = r.left();
                    do {
                        if (sample>=End) break;
                        float max = 0, min = 0;
                        for (long64 s = lastSample; s < sample; s++) {
                            float v = Buffer->at(s,channel);
                            if (v > 0) {
                                if (v > max) max = v;
                            }
                            else {
                                if (v < min) min = v;
                            }
                        }
                        lastSample = sample;
                        int iMax = max * YFactor;
                        int iMin = min * YFactor;
                        if ((iMax == 0) && (iMin == 0))
                        {
                            zeroCount++;
                        }
                        else
                        {
                            if (zeroCount) Scene.addLine(x - zeroCount, HalfHeight, x, HalfHeight, p);
                            zeroCount = 0;
                            Scene.addLine(x, HalfHeight - iMax, x, HalfHeight - iMin, p);
                        }
                        sample += ZoomValue;
                        x++;
                    } while (x < r.right());
                    if (zeroCount) Scene.addLine(x - zeroCount, HalfHeight, x - 1, HalfHeight, p);
                }

            }
        }
    }
private:
    void finishBuffer(const uint fromPtr);
    uint m_BufferSize;
    ldouble m_Pointer;
    CWaveFile* WF;
    QString m_Path;
    bool m_Finished;
    ulong64 m_Size;
    int AlternateDirection;
    float XFadeFactor;
    bool XFadeStarted;
    ulong64 XFadeStart;
    ulong64 XFadeEnd;
    double XFadePosition;
    ldouble m_Position;
    double m_OrigFreq;
    SampleStates m_SampleState;
    void inline Init();
    CChannelBuffer m_Audio;
    void Unref();
    QRecursiveMutex mutex;
};

#endif // CWAVEGENERATOR_H
