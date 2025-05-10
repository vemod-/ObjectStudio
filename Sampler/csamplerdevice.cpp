#include "csamplerdevice.h"

CSamplerDevice::CSamplerDevice()
{
    currentLayerIndex=0;
    currentRangeIndex=0;
    m_Modulation=1;
    testMode=st_NoTest;
    looping=false;
    addLayer();
}

void CSamplerDevice::noteOn(short channel, short pitch, short velocity)
{
    if (channelSettings[channel].portNote)
    {
        for (CSamplerGenerator& g : SamplerGenerator)//(int i=0;i<Sampler::samplervoices;i++)
        {
            if (g.matches(channel,channelSettings[channel].portNote))
            {
                g.ID=pitch;
                g.addPortamento(pitch-channelSettings[channel].portNote);
                break;
            }
        }
    }
    else
    {
        int FreeIndex=-1;
        for (int i=0;i<Sampler::samplervoices;i++)
        {
            if (SamplerGenerator[i].ID==0)
            {
                FreeIndex=i;
                break;
            }
        }
        if (FreeIndex>-1)
        {
            SamplerGenerator[FreeIndex].setID(channel,pitch);
            SamplerGenerator[FreeIndex].startNote(pitch,velocity & 0x7F);
        }
    }
    channelSettings[channel].portNote=0;
}

void CSamplerDevice::noteOff(short channel, short pitch)
{
    for (CSamplerGenerator& g : SamplerGenerator)//(int i=0;i<Sampler::samplervoices;i++)
    {
        if ((channel) == g.channel)
        {
            if (channelSettings[channel].pedal)
            {
                channelSettings[channel].pedalnotes.append(pitch);
            }
            else if (pitch==g.ID)
            {
                g.endNote();
                break;
            }
        }
    }
}

void CSamplerDevice::aftertouch(const short channel, const short pitch, const short value)
{
    for (CSamplerGenerator& g : SamplerGenerator)//(int i=0;i<Sampler::samplervoices;i++)
    {
        if (g.matches(channel,pitch))
        {
            g.setAftertouch(value);
        }
    }
}

void CSamplerDevice::allNotesOff()
{
    testMode=st_NoTest;
    looping=false;
    for (CSamplerGenerator& g : SamplerGenerator)//(int i=0;i<Sampler::samplervoices;i++)
    {
        if (!g.finished)
        {
            g.endNote();
        }
    }
}

float* CSamplerDevice::getNext(const int voice)
{
    SamplerGenerator[voice].setPitchWheel(channelSettings[voiceChannel(voice)].pitchWheel);
    SamplerGenerator[voice].setModulation(m_Modulation);
    return SamplerGenerator[voice].getNext();
}

void CSamplerDevice::loopTest(CStereoBuffer* b)
{
    CWaveGenerator* WG=&currentRange()->generator;
    if (!looping)
    {
        looping=true;
        WG->reset();
    }
    (WG->channels()==1) ? b->addMono(WG->getNextFreq(),0.5,0.5) : b->addBuffer(WG->getNextFreq(),0.5);
}

void CSamplerDevice::tuneTest(CStereoBuffer* b)
{
    CWaveGenerator* WG=&currentRange()->generator;
    if (!looping)
    {
        looping=true;
        int TempTune=WG->LP.MIDICents;
        WG->LP.MIDICents=0;
        WG->reset();
        WG->LP.MIDICents=TempTune;
    }

    float PlayFreq=cent2Factorf(WG->LP.MIDICents)*440.f;

    if (WG->channels()==1)
    {
        CMonoBuffer B(WG->getNextFreq(PlayFreq));
        if (B.isValid())
        {
            for (uint i=0;i<b->size();i++)
            {
                b->addAt(i,(B.at(i)+(waveBank.getNextFreq(440,CWaveBank::Sawtooth)*0.1))*0.5);
            }
        }
    }
    else
    {
        const CStereoBuffer B(WG->getNextFreq(PlayFreq));
        if (B.isValid())
        {
            for (uint i=0;i<b->size();i++)
            {
                float SawWave=waveBank.getNextFreq(440,CWaveBank::Sawtooth)*0.1;
                b->addAt(i,(SawWave+B.at(i))*0.5,(SawWave+B.atR(i)*0.5));
            }
        }
    }
}

short CSamplerDevice::voiceChannel(const int voice) const
{
    return SamplerGenerator[voice].channel;
}

int CSamplerDevice::voiceCount() const
{
    return Sampler::samplervoices;
}

void CSamplerDevice::reset()
{
    testMode=st_NoTest;
    looping=false;
    for (CSamplerGenerator& g : SamplerGenerator)//(int i=0;i<Sampler::samplervoices;i++)
    {
        g.resetPortamento();
        g.ID=0;
        g.channel=0;
    }
    for (ChannelData& d : channelSettings)//(int i = 0; i < 16; i++)
    {
        d.resetAll();
    }
}

const CSampleKeyRange::RangeParams CSamplerDevice::RangeParams(const int Layer, const int Range) const
{
    return SamplerGenerator[0].rangeParameters(Layer,Range);
}

const CSampleKeyRange::RangeParams CSamplerDevice::RangeParams() const
{
    return RangeParams(currentLayerIndex,currentRangeIndex);
}

void CSamplerDevice::setRangeParams(const CSampleKeyRange::RangeParams& RangeParams, const int Layer, const int Range)
{
    for (CSamplerGenerator& g : SamplerGenerator)//(int i=0;i<Sampler::samplervoices;i++)
    {
        g.setRangeParameters(RangeParams,Layer,Range);
    }
}

void CSamplerDevice::setRangeParams(const CSampleKeyRange::RangeParams& RangeParams)
{
    setRangeParams(RangeParams,currentLayerIndex,currentRangeIndex);
}

const CWaveGenerator::LoopParameters CSamplerDevice::LoopParams(const int Layer, const int Range) const
{
    return SamplerGenerator[0].loopParameters(Layer,Range);
}

const CWaveGenerator::LoopParameters CSamplerDevice::LoopParams() const
{
    return LoopParams(currentLayerIndex,currentRangeIndex);
}

void CSamplerDevice::setLoopParams(const CWaveGenerator::LoopParameters& LoopParams, const int Layer, const int Range)
{
    for (CSamplerGenerator& g : SamplerGenerator)//(int i=0;i<Sampler::samplervoices;i++)
    {
        g.setLoopParameters(LoopParams,Layer,Range);
    }
}

void CSamplerDevice::setLoopParams(const CWaveGenerator::LoopParameters& LoopParams)
{
    setLoopParams(LoopParams,currentLayerIndex,currentRangeIndex);
}

const CLayer::LayerParams CSamplerDevice::LayerParams(const int Layer) const
{
    return SamplerGenerator[0].layerParameters(Layer);
}

const CLayer::LayerParams CSamplerDevice::LayerParams() const
{
    return LayerParams(currentLayerIndex);
}

void CSamplerDevice::setLayerParams(const CLayer::LayerParams& LayerParams, const int Layer)
{
    for (CSamplerGenerator& g : SamplerGenerator)//(int i=0;i<Sampler::samplervoices;i++)
    {
        g.setLayerParameters(LayerParams,Layer);
    }
}

void CSamplerDevice::setLayerParams(const CLayer::LayerParams& LayerParams)
{
    setLayerParams(LayerParams,currentLayerIndex);
}

const CADSR::ADSRParams CSamplerDevice::ADSRParams() const
{
    return SamplerGenerator[0].ADSRParameters();
}

void CSamplerDevice::setADSRParams(const CADSR::ADSRParams& ADSRParams)
{
    for (CSamplerGenerator& g : SamplerGenerator)//(int i=0;i<Sampler::samplervoices;i++)
    {
        g.setADSRParams(ADSRParams);
    }
}

void CSamplerDevice::setTune(const float tune)
{
    for (CSamplerGenerator& g : SamplerGenerator)//(int i=0;i<Sampler::samplervoices;i++)
    {
        g.setTune(tune);
    }
}

void CSamplerDevice::setModulation(const float modulation)
{
    m_Modulation=modulation;
}

void CSamplerDevice::addRange(int Layer, const QString &WavePath, int Upper, int Lower)
{
    QMutexLocker locker(&mutex);
    for (CSamplerGenerator& g : SamplerGenerator)//(int i=0;i<Sampler::samplervoices;i++)
    {
        g.addRange(Layer,WavePath,Upper,Lower);
    }
}

void CSamplerDevice::addRange(const QString &WavePath, int Upper, int Lower)
{
    QMutexLocker locker(&mutex);
    addRange(currentLayerIndex,WavePath,Upper,Lower);
}

void CSamplerDevice::changePath(int Layer, int Range, const QString &WavePath)
{
    QMutexLocker locker(&mutex);
    for (CSamplerGenerator& g : SamplerGenerator)//(int i=0;i<Sampler::samplervoices;i++)
    {
        g.changePath(Layer,Range,WavePath);
    }
}

void CSamplerDevice::changePath(const QString &WavePath)
{
    QMutexLocker locker(&mutex);
    changePath(currentLayerIndex,currentRangeIndex,WavePath);
}

void CSamplerDevice::removeRange(int Layer, int Index)
{
    QMutexLocker locker(&mutex);
    for (CSamplerGenerator& g : SamplerGenerator)//(int i=0;i<Sampler::samplervoices;i++)
    {
        g.removeRange(Layer,Index);
    }
}

void CSamplerDevice::removeRange(int Index)
{
    QMutexLocker locker(&mutex);
    removeRange(currentLayerIndex,Index);
}

void CSamplerDevice::removeRange()
{
    QMutexLocker locker(&mutex);
    removeRange(currentLayerIndex,currentRangeIndex);
}

void CSamplerDevice::addLayer(int Upper, int Lower)
{
    QMutexLocker locker(&mutex);
    for (CSamplerGenerator& g : SamplerGenerator)//(int i=0;i<Sampler::samplervoices;i++)
    {
        g.addLayer(Upper,Lower);
    }
}

void CSamplerDevice::removeLayer(int index)
{
    QMutexLocker locker(&mutex);
    for (CSamplerGenerator& g : SamplerGenerator)//(int i=0;i<Sampler::samplervoices;i++)
    {
        g.removeLayer(index);
    }
}

void CSamplerDevice::removeLayer()
{
    QMutexLocker locker(&mutex);
    removeLayer(currentLayerIndex);
}

int CSamplerDevice::rangeCount(const int Layer) const
{
    return SamplerGenerator[0].rangeCount(Layer);
}

int CSamplerDevice::rangeCount() const
{
    return rangeCount(currentLayerIndex);
}

int CSamplerDevice::layerCount() const
{
    return SamplerGenerator[0].layerCount();
}

CLayer* CSamplerDevice::layer(int Index)
{
    return SamplerGenerator[0].layer(Index);
}

CSampleKeyRange* CSamplerDevice::range(int Layer, int Index)
{
    return this->layer(Layer)->range(Index);
}

CLayer* CSamplerDevice::currentLayer()
{
    return layer(currentLayerIndex);
}

CSampleKeyRange* CSamplerDevice::currentRange()
{
    return range(currentLayerIndex,currentRangeIndex);
}

void CSamplerDevice::serialize(QDomLiteElement* xml) const
{
    SamplerGenerator[0].serialize(xml);
}

void CSamplerDevice::unserialize(const QDomLiteElement* xml)
{
    QMutexLocker locker(&mutex);
    testMode=st_NoTest;
    looping=false;
    for (CSamplerGenerator& g : SamplerGenerator)//(int i=0;i<Sampler::samplervoices;i++)
    {
        g.unserialize(xml);
    }
}
