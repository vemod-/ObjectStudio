#include "cautotune.h"

CAutoTune::CAutoTune() : PD(presets.SampleRate),PS(presets.SampleRate,presets.ModulationRate)
{
}

void CAutoTune::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackMonoOut(monoout);
    addJackMonoIn();
    addParameterTune("Calibration");
    addParameterPercent("Glide");
    addParameter(CParameterVars::Numeric,"Slack","Cents",0,100,0,"",2);
    startParameterGroup();
    addParameterPercent("Threshold",10);
    addParameter(CParameterVars::Numeric,"Max Frequency","Hz",5000,presets.HalfRate,0,"",5000);
    addParameter(CParameterVars::Numeric,"Rate","mSec",10,1000,0,"",10);
    addParameterSelect("Oversampling","1§2§4§8§16§32",3);
    endParameterGroup();
    updateDeviceParameter();
}

CAudioBuffer* CAutoTune::getNextA(const int /*ProcIndex*/)
{
    CMonoBuffer* InBuffer = FetchAMono(monoin);
    if (!InBuffer->isValid()) return nullptr;
    PD.ProcessBuffer(InBuffer->data(), int(presets.ModulationRate));
    const double target = PD.correctionFactor();
    PS.process(target, InBuffer->data(), m_AudioBuffers[monoout]->data());
    return m_AudioBuffers[monoout];
}

void inline CAutoTune::updateDeviceParameter(const CParameter* /*p*/)
{
    PD.setTune(double(m_Parameters[pnTune]->PercentValue));
    PD.setGlide(m_Parameters[pnGlide]->Value);
    PD.setDetectSlack(m_Parameters[pnSlack]->Value);
    PD.setDetectLevelThreshold(m_Parameters[pnThreshold]->PercentValue);
    PD.setMaxDetectFrequency(m_Parameters[pnMaxFreq]->Value);
    PD.setPitchRecordsPerSecond(1000/m_Parameters[pnRate]->Value);
    PS.setOverSampling(1 << m_Parameters[pnOversampling]->Value);
}
