#include "clayer.h"

CLayer::CLayer(int Upper,int Lower)
{
    qDebug() << "Create CLayer";
    parameters.reset(Upper,Lower);
    PlayVol=1;
}

CLayer::CLayer(LayerParams LayerParams)
{
    qDebug() << "Create CLayer";
    parameters=LayerParams;
    PlayVol=1;
}

CLayer::~CLayer()
{
    qDebug() << "Delete CLayer";
    while (!Ranges.empty()) removeRange(Ranges.last());
}

void CLayer::reset(short MidiNote)
{
    ActiveRanges.clear();
    for(CSampleKeyRange* KR : std::as_const(Ranges))
    {
        float Vol=KR->parameters.keyVolume(MidiNote+parameters.Transpose);
        if (Vol > 0)
        {
            ActiveRanges.append(KR);
            KR->PlayVol=Vol;
            KR->generator.reset();
        }
    }
}

void CLayer::release()
{
    for(CSampleKeyRange* KR : std::as_const(ActiveRanges)) KR->generator.release();
}

void CLayer::modifyBuffer(CStereoBuffer& Buffer, float Frequency, float Velocity)
{
    for(CSampleKeyRange* KR : std::as_const(ActiveRanges))
    {
        const float Vol=KR->PlayVol*Velocity;
        const double Freq=Frequency*cent2Factor(KR->generator.LP.MIDICents);
        (KR->generator.channels()==1) ? Buffer.addMono(KR->generator.getNextFreq(Freq),Vol,Vol) :
            Buffer.addBuffer(KR->generator.getNextFreq(Freq),Vol);
    }
}

CSampleKeyRange* CLayer::addRange(const QString& WavePath,int Upper,int Lower)
{
    QMutexLocker locker(&mutex);
    qDebug() << "Create Range" << WavePath;
    auto KR=new CSampleKeyRange(WavePath,Upper,Lower);
    Ranges.append(KR);
    return KR;
}

void CLayer::changePath(int Range, const QString &WavePath)
{
    QMutexLocker locker(&mutex);
    Ranges[Range]->changePath(WavePath);
}

void CLayer::removeRange(CSampleKeyRange* KR)
{
    QMutexLocker locker(&mutex);
    Ranges.removeOne(KR);
    ActiveRanges.removeOne(KR);
    delete KR;
}

void CLayer::serialize(QDomLiteElement* xml) const
{
    for(const CSampleKeyRange* KR : Ranges)
    {
        QDomLiteElement* Range = xml->appendChild("Range","WaveFile",KR->fileName);
        KR->generator.LP.serialize(Range);
        KR->parameters.serialize(Range);
    }
}

void CLayer::unserialize(const QDomLiteElement* xml)
{
    QMutexLocker locker(&mutex);
    if (!xml) return;
    const QDomLiteElementList XMLRanges = xml->elementsByTag("Range");
    while (XMLRanges.size()<Ranges.size())
    {
        CSampleKeyRange* KR=Ranges.last();
        Ranges.removeOne(KR);
        ActiveRanges.removeOne(KR);
        delete KR;
    }
    while (XMLRanges.size()>Ranges.size()) addRange();
    int i=0;
    for (const QDomLiteElement* Range : XMLRanges)
    {
        CSampleKeyRange* KR=Ranges[i];
        const QString FileName=CPresets::resolveFilename(Range->attribute("WaveFile"));
        KR->changePath(FileName);
        KR->generator.LP.unserialize(Range,KR->generator.size());
        KR->parameters.unserialize(Range);
        i++;
    }
}

CSampleKeyRange* CLayer::range(int Index)
{
    return Ranges[Index];
}

int CLayer::rangeCount()
{
    return Ranges.size();
}

CSampleKeyRange::RangeParams CLayer::rangeParameters(int Range)
{
    return Ranges[Range]->parameters;
}

void CLayer::setRangeParameters(CSampleKeyRange::RangeParams RangeParams, int Range)
{
    Ranges[Range]->parameters=RangeParams;
}

CWaveGenerator::LoopParameters CLayer::loopParameters(int Range)
{
    return Ranges[Range]->generator.LP;
}

void CLayer::setLoopParameters(CWaveGenerator::LoopParameters LoopParams, int Range)
{
    Ranges[Range]->generator.LP=LoopParams;
}
