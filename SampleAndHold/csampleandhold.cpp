#include "csampleandhold.h"


CSampleAndHold::CSampleAndHold()
{
    ReturnValue=0;
    m_Counter=0;
}

void CSampleAndHold::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveIn();
    addJackModulationOut(jnOutPitch,"Out");
    addParameterRate("Sample Rate",400);
    updateDeviceParameter();
}

float CSampleAndHold::getNext(const int /*ProcIndex*/)
{
    const CMonoBuffer* InBuffer = FetchAMono(jnIn);
    if (InBuffer->isValid())
    {
        while (m_Counter + m_SampleRate < m_BufferSize)
        {
            m_Counter += m_SampleRate;
            if (m_Counter >= 0) ReturnValue = InBuffer->at(m_Counter);
        }
        m_Counter -= m_BufferSize;
    }
    return ReturnValue;
}

void CSampleAndHold::updateDeviceParameter(const CParameter* /*p*/)
{
    m_SampleRate = presets.SampleRate / m_Parameters[pnSampleRate]->PercentValue;
}
