#ifndef CLAYERCLASS_H
#define CLAYERCLASS_H

#include "crange.h"
#include "caudiobuffer.h"

class CLayer
{
public:
    class LayerParams
    {
    public:
        int UpperTop;
        int LowerTop;
        int UpperZero;
        int LowerZero;
        int Volume;
        int Transpose;
        int Tune;
        LayerParams() { reset(); }
        void reset(int Upper=0, int Lower=0)
        {
            if (Lower>Upper) qSwap(Upper,Lower);
            UpperTop=Upper;
            UpperZero=Upper;
            LowerTop=Lower;
            LowerZero=Lower;
            Volume=100;
            Tune=0;
            Transpose=0;
        }
        void unserialize(const QDomLiteElement* xml)
        {
            if (!xml) return;
            UpperZero=xml->attributeValueInt("UpperZero",127);
            UpperTop=xml->attributeValueInt("UpperTop",127);
            LowerZero=xml->attributeValueInt("LowerZero",0);
            LowerTop=xml->attributeValueInt("LowerTop",0);
            Volume=xml->attributeValueInt("Volume",100);
            Transpose=xml->attributeValueInt("Transpose",0);
            Tune=xml->attributeValueInt("Tune",0);
        }
        void serialize(QDomLiteElement* xml) const
        {
            xml->setAttribute("UpperZero",UpperZero);
            xml->setAttribute("UpperTop",UpperTop);
            xml->setAttribute("LowerZero",LowerZero);
            xml->setAttribute("LowerTop",LowerTop);
            xml->setAttribute("Volume",Volume);
            xml->setAttribute("Transpose",Transpose);
            xml->setAttribute("Tune",Tune);
        }
        float velVolume(int Velocity)
        {
            if ((Velocity<LowerZero) || (Velocity>UpperZero)) return 0;
            float Vol=Volume*0.01f;
            if (Velocity<LowerTop)
            {
                float diff=LowerTop-LowerZero;
                float Val=Velocity-LowerZero;
                Vol=(Vol*Val)/diff;
            }
            if (Velocity>UpperTop)
            {
                float diff=-(UpperTop-UpperZero);
                float Val=-(Velocity-UpperZero);
                Vol=(Vol*Val)/diff;
            }
            return lin2expf(Vol);
        }
    };
    LayerParams parameters;
    float PlayVol;
    CLayer(int Upper=127,int Lower=0);
    CLayer(LayerParams LayerParams);
    ~CLayer();
    CSampleKeyRange* addRange(const QString& WavePath=QString(),int Upper=127,int Lower=0);
    void changePath(int range, const QString &WavePath);
    void removeRange(CSampleKeyRange* KR);
    void modifyBuffer(CStereoBuffer& Buffer, float Frequency,float Velocity);
    void serialize(QDomLiteElement* xml) const;
    void unserialize(const QDomLiteElement* xml);
    void reset(short MidiNote);
    void release();
    CSampleKeyRange* range(int Index);
    int rangeCount();
    CSampleKeyRange::RangeParams rangeParameters(int range);
    void setRangeParameters(CSampleKeyRange::RangeParams rangeParameters,int range);
    CWaveGenerator::LoopParameters loopParameters(int range);
    void setLoopParameters(CWaveGenerator::LoopParameters loopParameters,int range);
private:	// User declarations
    QList<CSampleKeyRange*> Ranges;
    QList<CSampleKeyRange*> ActiveRanges;
    QRecursiveMutex mutex;
};

#endif // CLAYERCLASS_H
