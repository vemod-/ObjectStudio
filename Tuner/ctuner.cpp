#include "ctuner.h"
#include "ctunerform.h"

CTuner::CTuner()
{
}

CTuner::~CTuner()
{
}

void CTuner::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveOut(jnOut);
    addJackWaveIn();
    addParameterTune("Calibration");
    addParameterOffOn("Silent");
    addParameter(CParameter::Numeric,"Max Frequency","Hz",5000,presets.HalfRate,0,"",5000);
    addParameter(CParameter::Numeric,"Rate","mSec",10,1000,0,"",100);
    QMutexLocker locker(&mutex);
    m_Form=new CTunerForm(this,MainWindow);
    updateDeviceParameter();
}

void CTuner::tick()
{
    if (!m_OutJacks[0]->connectCount())
    {
        const CMonoBuffer* InBuffer = FetchAMono(jnIn);
        if (InBuffer->isValid())
        {
            if (m_Form->isVisible())
            {
                FORMFUNC(CTunerForm)->PD.ProcessBuffer(InBuffer->data(),presets.ModulationRate);
            }
        }
    }
    IDevice::tick();
}

CAudioBuffer* CTuner::getNextA(const int /*ProcIndex*/)
{
    CMonoBuffer* InBuffer = FetchAMono(jnIn);
    if (InBuffer->isValid())
    {
        if (m_Form->isVisible())
        {
            FORMFUNC(CTunerForm)->PD.ProcessBuffer(InBuffer->data(),presets.ModulationRate);
        }
    }
    return (m_Parameters[pnSilent]->Value) ? nullptr : InBuffer;
}

void inline CTuner::updateDeviceParameter(const CParameter* /*p*/)
{
    auto f=FORMFUNC(CTunerForm);
    f->PD.setTune(m_Parameters[pnTune]->PercentValue);
    f->PD.setMaxDetectFrequency(m_Parameters[pnMaxFreq]->Value);
    f->PD.setPitchRecordsPerSecond(1000/m_Parameters[pnRate]->Value);
}
