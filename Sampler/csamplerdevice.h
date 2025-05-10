#ifndef CSAMPLERDEVICE_H
#define CSAMPLERDEVICE_H

#include "csamplergenerator.h"
#include "csounddevice.h"
#include "cwavebank.h"

namespace Sampler
{
const int samplervoices=16;
}

class CSamplerDevice : public ISoundDevice
{
public:
    enum SamplerTestModes
    {
        st_NoTest,
        st_LoopTest,
        st_TuneTest
    };
    CSamplerDevice();
    void noteOn(const short channelMode, const short pitch, const short velocity);
    void noteOff(const short channelMode, const short pitch);
    void aftertouch(const short channelMode, const short pitch, const short value);
    float* getNext(const int voice);
    void tuneTest(CStereoBuffer* b);
    void loopTest(CStereoBuffer* b);
    short voiceChannel(const int voice) const;
    int voiceCount() const;
    void reset();
    void allNotesOff();
    void setTune(const float tune=440);
    void setModulation(const float modulation);
    void addRange(int layer,const QString& WavePath=QString(),int Upper=127,int Lower=0);
    void addRange(const QString& WavePath=QString(),int Upper=127,int Lower=0);
    void changePath(int layer,int range,const QString& WavePath);
    void changePath(const QString& WavePath);
    void removeRange(int layer,int Index);
    void removeRange(int Index);
    void removeRange();
    void addLayer(int Upper=127, int Lower=0);
    void removeLayer(int index);
    void removeLayer();
    int layerCount() const;
    int rangeCount(const int layer) const;
    int rangeCount() const;
    const CSampleKeyRange::RangeParams RangeParams(const int layer, const int range) const;
    const CSampleKeyRange::RangeParams RangeParams() const;
    void setRangeParams(const CSampleKeyRange::RangeParams& RangeParams,const int layer, const int range);
    void setRangeParams(const CSampleKeyRange::RangeParams& RangeParams);
    const CWaveGenerator::LoopParameters LoopParams(const int layer, const int range) const;
    const CWaveGenerator::LoopParameters LoopParams() const;
    void setLoopParams(const CWaveGenerator::LoopParameters& LoopParams, const int layer, const int range);
    void setLoopParams(const CWaveGenerator::LoopParameters& LoopParams);
    const CLayer::LayerParams LayerParams(const int layer) const;
    const CLayer::LayerParams LayerParams() const;
    void setLayerParams(const CLayer::LayerParams& LayerParams, const int layer);
    void setLayerParams(const CLayer::LayerParams& LayerParams);
    const CADSR::ADSRParams ADSRParams() const;
    void setADSRParams(const CADSR::ADSRParams& ADSRParams);
    void serialize(QDomLiteElement* xml) const;
    void unserialize(const QDomLiteElement* xml);
    CLayer* layer(int Index);
    CSampleKeyRange* range(int layer, int Index);
    CLayer* currentLayer();
    CSampleKeyRange* currentRange();
    int currentLayerIndex;
    int currentRangeIndex;
    SamplerTestModes testMode;
    bool looping;
    CWaveBank waveBank;
private:
    CSamplerGenerator SamplerGenerator[Sampler::samplervoices];
    float m_Modulation;
    QRecursiveMutex mutex;
};

#endif // CSAMPLERDEVICE_H
