#include "cautotune.h"

CAutoTune::CAutoTune() : PD(presets.SampleRate),PS(presets.SampleRate)
{
}

void CAutoTune::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveOut(jnOut);
    addJackWaveIn();
    addParameterTune("Calibration");
    addParameterPercent("Glide");
    startParameterGroup();
    addParameter(CParameter::Numeric,"Max Frequency","Hz",5000,presets.HalfRate,0,"",5000);
    addParameter(CParameter::Numeric,"Rate","mSec",10,1000,0,"",10);
    addParameterSelect("Oversampling","1§2§4§8§16§32",3);
    endParameterGroup();
    QMutexLocker locker(&mutex);
    updateDeviceParameter();
}

CAudioBuffer* CAutoTune::getNextA(const int /*ProcIndex*/)
{
    const CMonoBuffer* InBuffer = FetchAMono(jnIn);
    if (!InBuffer->isValid()) return nullptr;
    PD.ProcessBuffer(InBuffer->data(),int(presets.ModulationRate));
    CPitchDetect::PitchRecord r=PD.CurrentPitchRecord();
    int c = r.MidiCents;
    if (r.MidiKey > 0)
    {
        const int mc = (r.MidiKey*100) + r.MidiCents;
        if (m_Parameters[pnGlide]->Value)
        {
            if (mc != m_lastMIDICent) glider.setTargetCent(c);
            c = glider.currentCent();
        }
        m_lastMIDICent = mc;
    }
    mutex.lock();
    PS.process(cent2Factor(c),presets.ModulationRate,InBuffer->data(),m_AudioBuffers[jnOut]->data());
    mutex.unlock();
    return m_AudioBuffers[jnOut];
}

void inline CAutoTune::updateDeviceParameter(const CParameter* /*p*/)
{
    PD.setTune(double(m_Parameters[pnTune]->PercentValue));
    glider.setGlide(m_Parameters[pnGlide]->Value);
    PD.setMaxDetectFrequency(m_Parameters[pnMaxFreq]->Value);
    PD.setPitchRecordsPerSecond(1000/m_Parameters[pnRate]->Value);
    PS.setOverSampling(1 << m_Parameters[pnOversampling]->Value);
}
