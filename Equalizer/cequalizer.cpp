#include "cequalizer.h"
#include "cequalizerform.h"

void inline CalcPeak(float Val,float* Peak)
{
    if (Val < 0)
    {
        Val = -Val;
    }
    if (Val > *Peak)
    {
        *Peak=Val;
    }
}

CEqualizer::CEqualizer()
{
}

void inline CEqualizer::CalcParams()
{
    for (int i=0; i<8; i++)
    {
        eq_set_params(&filters[i], Freq[i], Level[i], BWIDTH, m_Presets.SampleRate);
    }
    ((CEqualizerForm*)m_Form)->DrawGraph();
}

void CEqualizer::SetLevel(int Index, int Level)
{
    this->Level[Index]=Level;
    eq_set_params(&filters[Index], Freq[Index], this->Level[Index], BWIDTH, m_Presets.SampleRate);
    ((CEqualizerForm*)m_Form)->DrawGraph();
}

void CEqualizer::SetFreq(int Index, int Freq)
{
    this->Freq[Index]=Freq;
    eq_set_params(&filters[Index], this->Freq[Index], Level[Index], BWIDTH, m_Presets.SampleRate);
    ((CEqualizerForm*)m_Form)->DrawGraph();
}

void CEqualizer::Init(const int Index,void* MainWindow)
{
    m_Name=devicename;
    IDevice::Init(Index,MainWindow);
    AddJackWaveOut(jnOut);
    AddJackWaveIn();
    Freq[0]=100;
    Freq[1]=200;
    Freq[2]=400;
    Freq[3]=1000;
    Freq[4]=3000;
    Freq[5]=6000;
    Freq[6]=12000;
    Freq[7]=15000;
    for (int i=0;i<8;i++)
    {
        Level[i]=0;
        biquad_init(&filters[i]);
    }
    m_Form=new CEqualizerForm(this,(QWidget*)MainWindow);
    ((CEqualizerForm*)m_Form)->Init(this);
    ((CEqualizerForm*)m_Form)->Reset();
    CalcParams();
}

float* CEqualizer::GetNextA(const int ProcIndex)
{
    CEqualizerForm* f=((CEqualizerForm*)m_Form);
    float* InSignal=FetchA(jnIn);
    if (!InSignal)
    {
        for (int i1=0;i1<8;i1++)
        {
            CalcPeak(0,f->PeakVal+i1);
        }
        return NULL;
    }
    float* Buffer=AudioBuffers[ProcIndex]->Buffer;
    for (int i=0;i<m_BufferSize;i++)
    {
        float samp = InSignal[i];
        for (int i1=0;i1<8;i1++)
        {
            if (Level[i1] != 0)
            {
                samp = biquad_run(&filters[i1], samp);
            }
            CalcPeak(samp,f->PeakVal+i1);
        }
        Buffer[i] = samp;
    }
    return Buffer;
}

void CEqualizer::Play(const bool FromStart)
{
    if (FromStart) ((CEqualizerForm*)m_Form)->Reset();
}
