#include "csamplergenerator.h"

CSamplerGenerator::CSamplerGenerator()
{
    CurrentMIDINote=0;
    CurrentVelocity=0;
    m_Loading=false;
}

CSamplerGenerator::~CSamplerGenerator()
{
    m_Loading=true;
    qDeleteAll(Layers);
}

float* CSamplerGenerator::getNext()
{
    if (m_Loading)
    {
        finished=true;
        return nullptr;
    }
    float VolFact=ADSR.GetVol();
    if (ADSR.State==CADSR::esSilent)
    {
        finished=true;
        return nullptr;
    }
    if (VolFact==0) return nullptr;
    if (ActiveLayers.isEmpty()) return nullptr;
    Buffer.zeroBuffer();
    for(CLayer* L : std::as_const(ActiveLayers))
    {
        const float Freq=MIDIkey2Freqf(CurrentMIDINote+m_PortamentoStep+L->parameters.Transpose,m_Tune,m_PitchWheel+L->parameters.Tune)*m_Modulation;
        const float Vol=((float)CurrentVelocity/127.0)*VolFact*L->PlayVol;
        L->modifyBuffer(Buffer,Freq,Vol);
    }
    return Buffer.data();
}

void CSamplerGenerator::setAftertouch(const int value)
{
    float val=(value*0.001)+1;
    Q_UNUSED(val);
}

void CSamplerGenerator::startNote(const short MidiNote, const short MidiVelo)
{
    CurrentMIDINote=MidiNote;
    CurrentVelocity=MidiVelo;
    ActiveLayers.clear();
    ADSR.Start();
    for(CLayer* L : std::as_const(Layers))
    {
        float Vol=L->parameters.velVolume(MidiVelo);
        if (Vol > 0)
        {
            ActiveLayers.append(L);
            L->PlayVol=Vol;
            L->reset(MidiNote);
        }
    }
    finished=false;
}

void CSamplerGenerator::endNote()
{
    ID=0;
    ADSR.Finish();
    for(CLayer* L : std::as_const(ActiveLayers)) L->release();
}

void CSamplerGenerator::serialize(QDomLiteElement* xml) const
{
    for(const CLayer* L : Layers)
    {
        QDomLiteElement* Custom = xml->appendChild("Layer")->appendChild("Custom");
        L->serialize(Custom);
        L->parameters.serialize(Custom);
    }
    ADSR.serialize(xml->appendChild("ADSR")->appendChild("Custom"));
}

void CSamplerGenerator::unserialize(const QDomLiteElement* xml)
{
    QMutexLocker locker(&mutex);
    if (!xml) return;
    m_Loading=true;
    const QDomLiteElementList XMLLayers = xml->elementsByTag("Layer");
    while (XMLLayers.size()<Layers.size())
    {
        CLayer* L=Layers.last();
        Layers.removeOne(L);
        ActiveLayers.removeOne(L);
        delete L;
    }
    while (XMLLayers.size()>Layers.size()) Layers.append(new CLayer());
    int i=0;
    for (const QDomLiteElement* Layer : XMLLayers)
    {
        CLayer* L=Layers[i];
        L->parameters.unserialize(Layer->elementByTag("Custom"));
        L->unserialize(Layer->elementByTag("Custom"));
        i++;
    }
    ADSR.clear();
    if (const QDomLiteElement* ADSRelement = xml->elementByTag("ADSR")) ADSR.unserialize(ADSRelement->elementByTag("Custom"));
    m_Loading=false;
}

const CSampleKeyRange::RangeParams CSamplerGenerator::rangeParameters(const int Layer, const int Range) const
{
    return Layers[Layer]->rangeParameters(Range);
}

void CSamplerGenerator::setRangeParameters(const CSampleKeyRange::RangeParams& RangeParams, const int Layer, const int Range)
{
    Layers[Layer]->setRangeParameters(RangeParams,Range);
}

const CWaveGenerator::LoopParameters CSamplerGenerator::loopParameters(const int Layer, const int Range) const
{
    return Layers[Layer]->loopParameters(Range);
}

void CSamplerGenerator::setLoopParameters(const CWaveGenerator::LoopParameters& LoopParams, const int Layer, const int Range)
{
    Layers[Layer]->setLoopParameters(LoopParams,Range);
}

const CLayer::LayerParams CSamplerGenerator::layerParameters(const int Layer) const
{
    return Layers[Layer]->parameters;
}

void CSamplerGenerator::setLayerParameters(const CLayer::LayerParams& LayerParams, int Layer)
{
    Layers[Layer]->parameters=LayerParams;
}

const CADSR::ADSRParams CSamplerGenerator::ADSRParameters() const
{
    return ADSR.AP;
}

void CSamplerGenerator::setADSRParams(const CADSR::ADSRParams& ADSRParams)
{
    ADSR.AP=ADSRParams;
}

void CSamplerGenerator::addRange(int Layer, const QString &WavePath, int Upper, int Lower)
{
    QMutexLocker locker(&mutex);
    Layers[Layer]->addRange(WavePath,Upper,Lower);
}

void CSamplerGenerator::changePath(int Layer, int Range, const QString &WavePath)
{
    QMutexLocker locker(&mutex);
    Layers[Layer]->changePath(Range,WavePath);
}

void CSamplerGenerator::removeRange(int Layer, int Index)
{
    QMutexLocker locker(&mutex);
    CSampleKeyRange* KR=Layers[Layer]->range(Index);
    Layers[Layer]->removeRange(KR);
}

void CSamplerGenerator::addLayer(int Upper, int Lower)
{
    QMutexLocker locker(&mutex);
    auto L=new CLayer(Upper,Lower);
    L->addRange();
    Layers.append(L);
}

void CSamplerGenerator::removeLayer(int index)
{
    QMutexLocker locker(&mutex);
    CLayer* L=Layers[index];
    Layers.removeOne(L);
    ActiveLayers.removeOne(L);
    delete L;
}

CLayer* CSamplerGenerator::layer(const int Index)
{
    return Layers[Index];
}

int CSamplerGenerator::layerCount() const
{
    return Layers.size();
}

int CSamplerGenerator::rangeCount(const int Layer) const
{
    return Layers[Layer]->rangeCount();
}
