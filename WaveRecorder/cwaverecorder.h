#ifndef CWAVERECORDER_H
#define CWAVERECORDER_H

#include "idevice.h"
#include "cwavefile.h"

class CWaveRecorder : public IDevice
{
private:
    enum JackNames
    {jnOut,jnIn};
    enum ParameterNames
    {};
    //bool Playing;
    ulong m_MilliSeconds;
    bool m_Recording;
    CStereoBuffer m_SilentBuffer;
public:
    CWaveRecorder();
    ~CWaveRecorder();
    void initWithFile(const QString& path);
    void play(const bool FromStart);
    void pause();
    void init(const int Index, QWidget* MainWindow);
    void execute(bool show);
    CAudioBuffer* getNextA(const int ProcIndex);
    void tick();
    //ulong milliSeconds() const;
    //void skip(ulong mSecs);
    CStereoBuffer RecordBuffer;
    CWaveFile WaveFile;
    void startRecording();
    void finishRecording();
    bool saveAs(const QString& path);
    float PeakL;
    float PeakR;
    bool m_Monitor = false;
    float m_MonitorLevel = 1;
};

#endif // CWAVERECORDER_H
