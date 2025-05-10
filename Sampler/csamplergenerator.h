#ifndef CSAMPLERGENERATOR_H
#define CSAMPLERGENERATOR_H

#include "cadsr.h"
#include "clayer.h"
#include "isoundgenerator.h"

class CSamplerGenerator : public ISoundGenerator
{
public:
    CSamplerGenerator();
    ~CSamplerGenerator();
    float* getNext();
    void setAftertouch(const int value);
    void startNote(const short MidiNote,const short MidiVelo);
    void endNote();
    void serialize(QDomLiteElement* xml) const;
    void unserialize(const QDomLiteElement* xml);
    void addRange(int layer,const QString& WavePath=QString(),int Upper=127,int Lower=0);
    void changePath(int layer,int Range,const QString& WavePath);
    void removeRange(int layer,int Index);
    void addLayer(int Upper=127, int Lower=0);
    void removeLayer(int index);
    CLayer* layer(const int Index);
    int layerCount() const;
    int rangeCount(const int layer) const;
    const CSampleKeyRange::RangeParams rangeParameters(const int layer, const int Range) const;
    void setRangeParameters(const CSampleKeyRange::RangeParams& rangeParameters,const int layer, const int Range);
    const CWaveGenerator::LoopParameters loopParameters(const int layer, const int Range) const;
    void setLoopParameters(const CWaveGenerator::LoopParameters& loopParameters, const int layer, const int Range);
    const CLayer::LayerParams layerParameters(const int layer) const;
    void setLayerParameters(const CLayer::LayerParams& layerParameters,const int layer);
    const CADSR::ADSRParams ADSRParameters() const;
    void setADSRParams(const CADSR::ADSRParams& ADSRParams);
private:
    QList<CLayer*> Layers;
    QList<CLayer*> ActiveLayers;
    CADSR ADSR;
    short CurrentMIDINote;
    short CurrentVelocity;
    CStereoBuffer Buffer;
    QRecursiveMutex mutex;
    bool m_Loading;
};

#endif // CSAMPLERGENERATOR_H
