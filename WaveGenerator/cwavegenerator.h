#ifndef CWAVEGENERATOR_H
#define CWAVEGENERATOR_H

#include "cwavefile.h"

class SingleWaveMap : public QMap<QString, CWaveFile*>
{
    public:
        static SingleWaveMap* getInstance()
        {
            static SingleWaveMap    instance; // Guaranteed to be destroyed.
                                  // Instantiated on first use.
            return &instance;
        }
    private:
        SingleWaveMap() {}                   // Constructor? (the {} brackets) are needed here.
        SingleWaveMap(SingleWaveMap const&);              // Don't Implement
        void operator=(SingleWaveMap const&); // Don't implement
};

class CWaveGenerator
{
public:
    enum LoopTypeEnum
    {ltForward,ltAlternate,ltXFade};
    enum SampleStates
    {ssNone,ssSilent,ssStarting,ssLooping,ssEnding};
    struct LoopParameters
    {
        size_t Start;
        size_t End;
        size_t LoopStart;
        size_t LoopEnd;
        int MIDINote;
        int Tune;
        size_t FadeIn;
        size_t FadeOut;
        int Volume;
        int XFade;
        LoopTypeEnum LoopType;
    };
    CWaveGenerator();
    ~CWaveGenerator();
    bool open(const QString& path, unsigned int SampleRate, unsigned int BufferSize);
    float* GetNext(void);
    float* GetNext(int RateOverride);
    float* GetNext(const float& Frequency);
    LoopParameters LP;
    size_t GetLength(void);
    void Reset(void);
    void Release(void);
    float* BufferPointer(const int& Channel);
    void SetPointer(const size_t& Ptr);
    int Channels;
private:
    unsigned int m_BufferSize;
    unsigned int m_SampleRate;
    double Pointer;
    CWaveFile* WF;
    QString m_Path;
    bool Finished;
    float* Buffer;
    size_t Length;
    int AlternateDirection;
    float XFadeFactor;
    bool XFadeStarted;
    size_t XFadeStart;
    size_t XFadeEnd;
    double XFadePosition;
    double Position;
    float OrigFreq;
    SampleStates SampleState;
    void inline Init(void);
    float* Audio;
    SingleWaveMap* WaveFiles;
};

#endif // CWAVEGENERATOR_H
