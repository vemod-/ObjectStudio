#include "csampler.h"
#include "csamplerform.h"

CSampler::CSampler()
{
}

void CSampler::Init(const int Index,void* MainWindow)
{
    m_Name=devicename;
    IDevice::Init(Index,MainWindow);
    AddJackMIDIIn();
    AddJack("Modulation",IJack::Pitch,IJack::In,jnModulation);
    AddJackStereoOut(jnOut);

    AddParameterMIDIChannel();
    AddParameterTranspose();
    AddParameterTune();
    AddParameterPercent();
    VolumeFactor=1.0*(1.0/sqrtf(Sampler::samplervoices));
    LastMod=0;
    CurrentMod=1;
    SamplerDevice.ChangePath(0,0,":/test.wav");
    SamplerDevice.reset();
    m_Form=new CSamplerForm(this,(QWidget*)MainWindow);
    ((CSamplerForm*)m_Form)->Init(&SamplerDevice);
    CalcParams();
}

void CSampler::Process()
{
    CStereoBuffer* OutBuffer=(CStereoBuffer*)AudioBuffers[jnOut];
    if (SamplerDevice.TestMode==CSamplerDevice::st_NoTest)
    {
        float ModIn=0;
        if (m_ParameterValues[pnModulation]) ModIn=Fetch(jnModulation);
        if (ModIn != LastMod)
        {
            LastMod=ModIn;
            CurrentMod=pow(2.0,(float)ModIn * (float)m_ParameterValues[pnModulation] * 0.01);
        }
        SamplerDevice.parseMIDI((CMIDIBuffer*)FetchP(jnMIDIIn));
        bool First=true;
        for (int i1=0;i1<SamplerDevice.voiceCount();i1++)
        {
            float* BufferL=SamplerDevice.getNext(i1,CurrentMod);
            if (BufferL)
            {
                float volL=VolumeFactor*SamplerDevice.volL(SamplerDevice.voiceChannel(i1));
                float volR=VolumeFactor*SamplerDevice.volR(SamplerDevice.voiceChannel(i1));
                if (First)
                {
                    First=false;
                    OutBuffer->WriteBuffer(BufferL,volL,volR);
                }
                else
                {
                    OutBuffer->AddBuffer(BufferL,volL,volR);
                }
            }
        }
        if (First) OutBuffer->ZeroBuffer();
    }
    else if (SamplerDevice.TestMode==CSamplerDevice::st_LoopTest)
    {
        AudioBuffers[jnOut]->ZeroBuffer();
        SamplerDevice.LoopTest(OutBuffer->Buffer,OutBuffer->BufferR,m_BufferSize);
    }
    else if (SamplerDevice.TestMode==CSamplerDevice::st_TuneTest)
    {
        AudioBuffers[jnOut]->ZeroBuffer();
        SamplerDevice.TuneTest(OutBuffer->Buffer,OutBuffer->BufferR,m_BufferSize);
    }
}

void inline CSampler::CalcParams()
{
    VolumeFactor=1.0*(1.0/sqrtf(Sampler::samplervoices));
    SamplerDevice.setTune(m_ParameterValues[pnTune]*0.01);
    SamplerDevice.setTranspose(m_ParameterValues[pnTranspose]);
    SamplerDevice.setChannel(m_ParameterValues[pnMIDIChannel]);
}

void CSampler::Play(const bool FromStart)
{
    if (FromStart)
    {
        SamplerDevice.reset();
        ((CSamplerForm*)m_Form)->ReleaseLoop();
        CalcParams();
    }
}

void CSampler::Pause()
{
    SamplerDevice.allNotesOff();
    CalcParams();
}
