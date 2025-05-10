#ifndef CRANGE_H
#define CRANGE_H

#include "cwavegenerator.h"
#include "qdomlite.h"

class CSampleKeyRange : protected IPresetRef
{
public:
    class RangeParams
    {
    public:
        int UpperZero;
        int UpperTop;
        int LowerZero;
        int LowerTop;
        int Volume;
        RangeParams() { reset(); }
        void reset(int Upper=0, int Lower=0)
        {
            if (Lower>Upper) qSwap(Upper,Lower);
            UpperTop=Upper;
            UpperZero=Upper;
            LowerTop=Lower;
            LowerZero=Lower;
            Volume=100;
        }
        void unserialize(const QDomLiteElement* xml)
        {
            if (!xml) return;
            UpperZero=xml->attributeValueInt("UpperZero",127);
            UpperTop=xml->attributeValueInt("UpperTop",127);
            LowerZero=xml->attributeValueInt("LowerZero",0);
            LowerTop=xml->attributeValueInt("LowerTop",0);
            Volume=xml->attributeValueInt("Volume",100);
        }
        void serialize(QDomLiteElement* xml) const
        {
            xml->setAttribute("UpperZero",UpperZero);
            xml->setAttribute("UpperTop",UpperTop);
            xml->setAttribute("LowerZero",LowerZero);
            xml->setAttribute("LowerTop",LowerTop);
            xml->setAttribute("Volume",Volume);
        }
        float keyVolume(int MIDIKey)
        {
            if ((MIDIKey<LowerZero) || (MIDIKey>UpperZero)) return 0;
            float Vol=Volume*0.01f;
            if (MIDIKey<LowerTop)
            {
                float diff=LowerTop-LowerZero;
                float Val=MIDIKey-(LowerZero);
                Vol=(Vol*Val)/diff;
            }
            else if (MIDIKey>UpperTop)
            {
                float diff=-(UpperTop-UpperZero);
                float Val=-(MIDIKey-(UpperZero-1));
                Vol=(Vol*Val)/diff;
            }
            return lin2expf(Vol);
        }
    };
    CWaveGenerator generator;
    QString fileName;
    float PlayVol;
    RangeParams parameters;
    CSampleKeyRange(const QString& WavePath=QString(),int Upper=127,int Lower=0);
    CSampleKeyRange(const QString& WavePath,CSampleKeyRange::RangeParams RangeParams);
    ~CSampleKeyRange();
    void changePath(const QString& WavePath);
    void pitchDetect(int Tune);
    void autoLoop(int Cycles);
    void autoTune();
    void autoFix(int Cycles, int Tune);
private:
    QRecursiveMutex mutex;
};

#endif // CRANGE_H
