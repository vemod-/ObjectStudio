#include "cequalizer.h"
#include "cequalizerform.h"

void inline CalcPeak(float Val,float* Peak)
{
    *Peak=qMax<float>(qAbs<float>(Val),*Peak);
}



void inline CEqualizer::updateDeviceParameter(const CParameter* /*p*/)
{
    for (CBiquad& f : filters) f.init();
    filters[0].lsSetParams(Freq[0], Level[0], 1, presets.SampleRate);
    for (int i=1; i<7; i++)
    {
        filters[i].eqSetParams(Freq[i], Level[i], BWIDTH, presets.SampleRate);
    }
    filters[7].hsSetParams(Freq[7], Level[7], 1, presets.SampleRate);
    EQUALIZERFORM->DrawGraph();
}

CEqualizer::CEqualizer(){}

void CEqualizer::SetLevel(const int Index, const float Level)
{
    this->Level[Index]=Level;
    updateDeviceParameter();
}

void CEqualizer::SetFreq(const int Index, const int Freq)
{
    this->Freq[Index]=Freq;
    updateDeviceParameter();
}

void CEqualizer::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveOut(jnOut);
    addJackWaveIn();
    m_Form=new CEqualizerForm(this,MainWindow);
    EQUALIZERFORM->Init();
    EQUALIZERFORM->Reset();
    updateDeviceParameter();
}

CAudioBuffer* CEqualizer::getNextA(const int ProcIndex)
{
    const CMonoBuffer* InBuffer = FetchAMono(jnIn);
    if (!InBuffer->isValid())
    {
        for (int i=0;i<8;i++) CalcPeak(0,EQUALIZERFORM->PeakVal+i);
        return nullptr;
    }
    CMonoBuffer* OutBuffer=MonoBuffer(ProcIndex);
    for (uint i=0;i<m_BufferSize;i++)
    {
        float samp = InBuffer->at(i);
        for (int i1=0;i1<8;i1++)
        {
            if (!isZero(Level[i1])) samp = filters[i1].run(samp);
            CalcPeak(samp,EQUALIZERFORM->PeakVal+i1);
        }
        OutBuffer->setAt(i,samp);
    }
    return OutBuffer;
}

void CEqualizer::play(const bool FromStart)
{
    if (FromStart) EQUALIZERFORM->Reset();
    IDevice::play(FromStart);
}
