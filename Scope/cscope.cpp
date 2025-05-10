#include "cscope.h"
#include "cscopeform.h"
#include "csimplebuffer.h"

CScope::CScope()
{
}

CScope::~CScope()
{
}

void CScope::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveIn();
    addJackModulationIn("Voltage In");
    addParameterVolume("Gain");
    addParameter(CParameter::Numeric,"Rate","mSec",1,500,0,"",20);
    startParameterGroup();
    addParameterFrequency("Frequency");
    addParameterSelect("Detect Pitch","Off§On");
    endParameterGroup();
    addParameterSelect("Mode","Audio§Voltage");
    QMutexLocker locker(&mutex);
    m_Form=new CScopeForm(this,MainWindow);
    Reset();
    updateDeviceParameter();
}

void CScope::tick()
{
    if (m_Form->isVisible())
    {
        auto f=FORMFUNC(CScopeForm);
        const CMonoBuffer* InBuffer = FetchAMono(jnIn);
        const float Modulation = Fetch(jnModulationIn);
        if (m_Parameters[pnScopeMode]->Value==0)
        {
            (!InBuffer->isValid()) ?
                        f->Scope->process(nullptr,presets.ModulationRate) :
                        f->Scope->process(InBuffer->data(),presets.ModulationRate);
        }
        else
        {
            f->Scope->processVoltage(Modulation,presets.ModulationRate);
        }
    }
    IDevice::tick();
}

void inline CScope::updateDeviceParameter(const CParameter* /*p*/)
{
    auto f=FORMFUNC(CScopeForm);
    f->Scope->SetRate(m_Parameters[pnScopeRate]->Value);
    f->Scope->SetFreq(m_Parameters[pnFrequency]->PercentValue);
    f->Scope->SetVol(m_Parameters[pnVolume]->Value);
    f->Scope->SetDetectPitch(m_Parameters[pnDetectPitch]->Value);
}

void CScope::Reset()
{
}

