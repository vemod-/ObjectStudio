#include "csampler.h"
#include "csamplerform.h"

CSampler::CSampler()
{
}

void CSampler::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackStereoOut(jnOut);
    addJackMIDIIn();
    addJackModulationIn();
    startParameterGroup("MIDI", Qt::yellow);
    addParameterMIDIChannel();
    addParameterTranspose();
    endParameterGroup();
    makeParameterGroup(2,"Tune",Qt::green);
    addParameterTune();
    addParameterPercent();
    Modulator.init(m_Jacks[jnModulation],m_Parameters[pnModulation]);
    VolumeFactor=mixFactorf(Sampler::samplervoices);
    //LastMod=0;
    //CurrentMod=1;
    SamplerDevice.changePath(0,0,":/test.wav");
    SamplerDevice.reset();
    m_Form=new CSamplerForm(this,MainWindow);
    FORMFUNC(CSamplerForm)->Init(&SamplerDevice);
    updateDeviceParameter();
}

void CSampler::process()
{
    CStereoBuffer* OutBuffer=StereoBuffer(jnOut);
    if (SamplerDevice.testMode==CSamplerDevice::st_NoTest)
    {
        const long ModCent = Modulator.execCent();
        SamplerDevice.parseMIDI(FetchP(jnMIDIIn));
        bool First=true;
        SamplerDevice.setModulation(cent2Factorf(ModCent));
        for (int i1=0;i1<SamplerDevice.voiceCount();i1++)
        {
            const CStereoBuffer DeviceBuffer(SamplerDevice.getNext(i1));
            if (DeviceBuffer.isValid())
            {
                float volL=VolumeFactor*SamplerDevice.volL(SamplerDevice.voiceChannel(i1));
                float volR=VolumeFactor*SamplerDevice.volR(SamplerDevice.voiceChannel(i1));
                if (First)
                {
                    First=false;
                    OutBuffer->writeStereoBuffer(&DeviceBuffer,volL,volR);
                }
                else
                {
                    OutBuffer->addStereoBuffer(&DeviceBuffer,volL,volR);
                }
            }
        }
        if (First) OutBuffer->zeroBuffer();
    }
    else if (SamplerDevice.testMode==CSamplerDevice::st_LoopTest)
    {
        m_AudioBuffers[jnOut]->zeroBuffer();
        SamplerDevice.loopTest(OutBuffer);
    }
    else if (SamplerDevice.testMode==CSamplerDevice::st_TuneTest)
    {
        m_AudioBuffers[jnOut]->zeroBuffer();
        SamplerDevice.tuneTest(OutBuffer);
    }
}

void inline CSampler::updateDeviceParameter(const CParameter* /*p*/)
{
    VolumeFactor=mixFactorf(Sampler::samplervoices);
    SamplerDevice.setTune(m_Parameters[pnTune]->PercentValue);
    SamplerDevice.setTranspose(m_Parameters[pnTranspose]->Value);
    SamplerDevice.setChannelMode(m_Parameters[pnMIDIChannel]->Value);
}

void CSampler::play(const bool FromStart)
{
    if (FromStart)
    {
        SamplerDevice.reset();
        FORMFUNC(CSamplerForm)->ReleaseLoop();
        updateDeviceParameter();
    }
    IDevice::play(FromStart);
}

void CSampler::pause()
{
    SamplerDevice.allNotesOff();
    updateDeviceParameter();
    IDevice::pause();
}
