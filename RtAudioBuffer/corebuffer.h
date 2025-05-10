#ifndef COREBUFFER_H
#define COREBUFFER_H

//#define __MINIAUDIO__
//#define __NOMIDI__

#ifdef __MINIAUDIO__
    #define MA_NO_RUNTIME_LINKING
    #define MINIAUDIO_IMPLEMENTATION
    #include "miniaudio.h"
#else
    #include "/Library/Developer/Library/rtaudio-6.0.1/RtAudio.h"
#endif
#ifndef __NOMIDI__
    #include "/Library/Developer/Library/rtmidi-6.0.0/RtMidi.h"
#endif

#include "idevice.h"
#include "cwavefile.h"
//#include <QObject>
//#include <QStringList>
//#include <QProgressBar>
//#include <QThread>
//#include <CoreServices/CoreServices.h>
//#include <QMutexLocker>
#include "cmseccounter.h"
#include "ccaffeine.h"

class CCoreMainBuffers : public IDevice
{
public:
    CInJack* InAudio;
    CInJack* InMIDI;
    COutJack* OutAudio;
    COutJack* OutMIDI;
    CCoreMainBuffers();
    ~CCoreMainBuffers();
    void init(const int Index, QWidget* MainWindow);
    CMIDIBuffer* getNextP(const int ProcIndex);
    void MainAudioLoop(void* OutBuffer, void* InBuffer, const uint BufferSize);
    bool driverCheck();
    void scanDrivers();
    void createBuffer();
    void finish();
    void panic();
    void wait();
    void startRecording();
    void stopRecording();
    bool saveRecording(const QString& fileName);
    void render(const QString &fileName);
    void getPeak(float& L,float& R);

#ifdef __MINIAUDIO__
    static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

#else
    static int AudioCallback( void *outputBuffer, void *inputBuffer, uint nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData );
    static void AudioErrorCallback(RtAudioErrorType type, const std::string &errorText);
#endif

    float outputVol;
    const QStringList inDriverNames();
    const QStringList outDriverNames();
    void setInDriver(const QString& driverName);
    void setOutDriver(const QString& driverName);
    void setDrivers(const QString& inDriverName,const QString& outDriverName);
    const QString inDriverName();
    const QString outDriverName();
    const QList<uint>sampleRates();
    inline bool isRecording() { return m_Recording; }
    //void setTicker(ITicker* Ticker) { m_Ticker=Ticker; }
    void play(const bool FromStart);
    void pause();
    void skip(const ulong64 samples);
    inline ulong64 currentSample() const { return mSecCount.currentSample(); }
    inline ulong currentMilliSecond() const { return mSecCount.currentmSec(); }
    //inline bool isPlaying() const { return m_Playing; }
    /*
    ulong duration()
    {
        if (m_Ticker) return m_Ticker->duration();
        return 0;
    }
    ulong milliSeconds()
    {
        if (m_Ticker) return m_Ticker->milliSeconds();
        return 0;    }
        */
private:
    enum BufferStates
    {
        Ready=0,
        Working=1,
        Stopped=2,
        Starting=3
    };
    enum JackNames { jnIn,jnMIDIIn,jnOut,jnMIDIOut };
    BufferStates BufferState;

#ifdef __MINIAUDIO__
    ma_device* miniAudio;
#else
    RtAudio* m_Audio;
#endif
#ifndef __NOMIDI__
    RtMidiIn m_MidiIn;
    RtMidiOut m_MidiOut;
#endif

    uint m_Startcounter;

    CCaffeine caffeine;

    //float* ChannelBufferL;
    //float* ChannelBufferR;
    CStereoBuffer* OutChannelBuffer;

    CStereoBuffer* InChannelBuffer;

    CStereoBuffer m_NullBufferStereo;

    //ITicker* m_Ticker;
    uint TickCount;

    float PeakL;
    float PeakR;

    CMIDIBuffer MIDIBuffer;
    //float inline TruncateVal(float Buf, float& Peak);
    void inline ParseMidi(const CMIDIBuffer* MIDIBuffer);

    CWaveFile WaveFile;
    bool m_Recording;

    ulong64 ActivityCount;
    ulong64 ActivityLimit;

#ifdef __MINIAUDIO__
    #define driverIDtype ma_device_id
    ma_device_id m_InDriverID;
    ma_device_id m_OutDriverID;
#else
    #define driverIDtype uint
    int m_InDriverID=-1;
    int m_OutDriverID=-1;
#endif

    QStringList m_InDriverNames;
    QStringList m_OutDriverNames;
    QList<driverIDtype> m_InDriverIDs;
    QList<driverIDtype> m_OutDriverIDs;

    //bool m_Playing;
    ulong m_Samples;
    CSampleCounter mSecCount;

    void soundDriverNames(int Direction,QStringList& names,QList<driverIDtype>& ids);
};

#endif // COREBUFFER_H
