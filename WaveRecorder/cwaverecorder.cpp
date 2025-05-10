#include "cwaverecorder.h"
#include "cwaverecorderform.h"

CWaveRecorder::CWaveRecorder()
{
    m_Recording=false;
    //Playing=false;
}

CWaveRecorder::~CWaveRecorder()
{
}

void CWaveRecorder::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackStereoOut(jnOut);
    addJackStereoIn();
    m_Form=new CWaveRecorderForm(this,MainWindow);
    FORMFUNC(CWaveRecorderForm)->setHost(m_Host);
    //Playing=false;
    m_MilliSeconds=0;
    m_Recording=false;
    PeakL=0;
    PeakR=0;
    m_SilentBuffer.zeroBuffer();
}

void CWaveRecorder::execute(bool show)
{
    IDevice::execute(show);
    FORMFUNC(CWaveRecorderForm)->showMixer(show);
}

void CWaveRecorder::tick()
{
    const CStereoBuffer* InBuffer = FetchAStereo(jnIn);
    if (InBuffer->isValid())
    {
        RecordBuffer.writeStereoBuffer(InBuffer,FORMFUNC(CWaveRecorderForm)->volumeL(),FORMFUNC(CWaveRecorderForm)->volumeR());
        if (m_Recording) WaveFile.pushBuffer(RecordBuffer.data(),RecordBuffer.size());
        RecordBuffer.peakStereoBuffer(&PeakL,&PeakR);
    }
    else
    {
        RecordBuffer.zeroBuffer();
    }
    //if (m_TickerDevice) m_TickerDevice->tick();
    IDevice::tick();
}

void CWaveRecorder::initWithFile(const QString& path) {
    FORMFUNC(CWaveRecorderForm)->initWithFile(path);
}
/*
ulong CWaveRecorder::milliSeconds() const
{
    return m_MilliSeconds;
}
*/
/*
void CWaveRecorder::skip(ulong mSecs)
{
    FORMFUNC(CWaveRecorderForm)->skip(samples);
}
*/
CAudioBuffer* CWaveRecorder::getNextA(const int ProcIndex)
{
    if (m_Monitor) {
        m_AudioBuffers[ProcIndex]->writeBuffer(FORMFUNC(CWaveRecorderForm)->getNextA(ProcIndex));
        m_AudioBuffers[ProcIndex]->addBuffer(&RecordBuffer,m_MonitorLevel);
        return m_AudioBuffers[ProcIndex];
    }
    return FORMFUNC(CWaveRecorderForm)->getNextA(ProcIndex);
}

void CWaveRecorder::play(const bool FromStart)
{
    FORMFUNC(CWaveRecorderForm)->setPlayIcon(true);
    IDevice::play(FromStart);
}

void CWaveRecorder::pause()
{
    FORMFUNC(CWaveRecorderForm)->setPlayIcon(false);
    IDevice::pause();
}

void CWaveRecorder::startRecording()
{
    m_Recording=true;
    WaveFile.startRecording(2,presets.SampleRate);
}

void CWaveRecorder::finishRecording()
{
    m_Recording=false;
    WaveFile.finishRecording();
}

bool CWaveRecorder::saveAs(const QString& path)
{
    if (m_Recording) finishRecording();
    return WaveFile.save(path);
}
