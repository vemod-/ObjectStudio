#include "cspectrum.h"
#include "cspectrumform.h"

CSpectrum::CSpectrum()
{
}

void CSpectrum::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveIn();
    addParameterVolume("Gain");
    addParameter(CParameter::Numeric,"Rate","mSec",10,200,0,"",100);
    addParameterSelect("Mode","Circular§Continuous§Diagram§Peak Diagram§Avg Diagram");
    addParameterSelect("Window","No Window§Hanning§Gauss§Flattop");
    addParameterSelect("Scale","Logaritmic§Linear");
    addParameterCutOff("Range",5000);
    QMutexLocker locker(&mutex);
    m_Form=new CSpectrumForm(this,MainWindow);
    Reset();
    updateDeviceParameter();
}

void CSpectrum::tick()
{
    if (m_Form->isVisible())
    {
        auto f=FORMFUNC(CSpectrumForm);
        const CMonoBuffer* InBuffer = FetchAMono(jnIn);
        f->Spectrum->process(InBuffer->data(),presets.ModulationRate);
        //(!InBuffer->isValid()) ? f->Spectrum->process(nullptr,presets.ModulationRate) :
        //                         f->Spectrum->process(InBuffer->data(),presets.ModulationRate);
    }
    IDevice::tick();
}

void inline CSpectrum::updateDeviceParameter(const CParameter* /*p*/)
{
    auto f=FORMFUNC(CSpectrumForm);
    f->Spectrum->SetUpdateRate(m_Parameters[pnSpectrumRate]->Value);
    f->Spectrum->SetVol(m_Parameters[pnVolume]->Value);
    f->Spectrum->SetMode(m_Parameters[pnMode]->Value);
    f->Spectrum->SetWindow(m_Parameters[pnWindow]->Value);
    f->Spectrum->SetScale(m_Parameters[pnScale]->Value);
    f->Spectrum->SetRange(m_Parameters[pnRange]->Value);
}

void CSpectrum::Reset()
{
}

