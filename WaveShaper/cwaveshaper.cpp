#include "cwaveshaper.h"

CWaveShaper::CWaveShaper()
{
}

void CWaveShaper::updateDeviceParameter(const CParameter* /*p*/) {
    float Amount=m_Parameters[pnAmount]->scaleValue(0.00999999f);
    k = 2*Amount/(1-Amount);
    k1=-1+Amount;
    k2=0.9999f+(Amount*100);
    a=m_Parameters[pnAmount]->Value;
    a1=m_Parameters[pnAmount]->scaleValue(0.02f);
    m_Gain=m_Parameters[pnGain]->PercentValue;
}

int CWaveShaper::sign(float x) {
    if (!isZero(x)) return x/fabsf(x);
    return 0;
}

float CWaveShaper::max(float x, float a) {
    x -= a;
    x += fabsf(x);
    x *= 0.5;
    x += a;
    return (x);
}

float CWaveShaper::min(float x, float b) {
    x = b - x;
    x += fabsf(x);
    x *= 0.5;
    x = b - x;
    return (x);
}

float CWaveShaper::clip(float x, float a, float b) {
    float x1 = fabsf(x-a);
    float x2 = fabsf(x-b);
    x = x1 + (a+b);
    x -= x2;
    x *= 0.5;
    return (x);
}

void CWaveShaper::init(const int Index, QWidget* MainWindow) {
    m_Name="WaveShaper";
    IDevice::init(Index,MainWindow);
    addJackWaveIn();
    addJackWaveOut(jnOut);
    addParameterVolume("Gain");
    addParameterPercent("Amount",1);
    addParameterSelect("Type","Hard§Softer§Sinus§Gloubi-boulga§Clipping");
    updateDeviceParameter();
}

CAudioBuffer *CWaveShaper::getNextA(const int ProcIndex) {
    const CMonoBuffer* InBuffer = FetchAMono(jnIn);
    if (!InBuffer->isValid()) return nullptr;
    CMonoBuffer* OutBuffer=MonoBuffer(ProcIndex);
    switch (m_Parameters[pnType]->Value)
    {
    case 0:
        for (uint i=0;i<m_BufferSize;i++)
        {
            float x=InBuffer->at(i)*m_Gain;
            OutBuffer->setAt(i,(1+k)*x/(1+k*fabsf(x)));
        }
        break;
    case 1:
        for (uint i=0;i<m_BufferSize;i++)
        {
            float x=InBuffer->at(i)*m_Gain;
            OutBuffer->setAt(i,x*(fabsf(x) + a)/(powf(x,2) + (a-1)*fabsf(x) + 1));
        }
        break;
    case 2:
    {
        float y;
        float z = PI_F * a1;
        float s = 1/sinf(z);
        float b = 1/a1;

        for (uint i=0;i<m_BufferSize;i++)
        {
            float x=InBuffer->at(i)*m_Gain;
            if (x > b)
            {
                y = 1;
            }
            else
            {
                y = sinf(z*x)*s;
            }
            OutBuffer->setAt(i,y);
        }
    }
        break;
    case 3:
        for (uint i=0;i<m_BufferSize;i++)
        {
            float y=InBuffer->at(i)*m_Gain;

            float x = y * 0.686306f;
            float z = 1 + expf(sqrtf(fabsf(x)) * k1);//-0.75);
            OutBuffer->setAt(i,(expf(x) - expf(-x * z)) / (expf(x) + expf(-x)));
        }
        break;
    case 4:
        for (uint i=0;i<m_BufferSize;i++)
        {
            float x=InBuffer->at(i)*m_Gain;
            //float y=pow(fabs(x), 1 / k2);
            OutBuffer->setAt(i,clip(x*k2,1,-1));
        }
        break;
    }
    return OutBuffer;
}
