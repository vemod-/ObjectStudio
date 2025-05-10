#ifndef CPARAMETER_H
#define CPARAMETER_H

#include "ihost.h"
#include "QDomLite"
#include "cpresets.h"
#include "idevicebase.h"
#include <QColor>
;
#pragma pack(push,1)

#define ParameterTag "Parameter"
#define ParameterNameAttribute "Name"
#define ParameterValueAttribute "Value"
#define ParameterTimeAttribute "Time"
#define ParameterIDAttribute "ID"

class CParameterEvent
{
public:
    CParameterEvent(){}
    CParameterEvent(ulong64 t, int v, const QString& ID) : time(t), value(v), id(ID) {}
    CParameterEvent(const QDomLiteElement* e) {
        unserialize(e);
    }
    void unserialize(const QDomLiteElement* XMLParameterEvent) {
        time=XMLParameterEvent->attributeValueULongLong(ParameterTimeAttribute);
        value=XMLParameterEvent->attributeValueInt(ParameterValueAttribute);
        id=XMLParameterEvent->attribute(ParameterIDAttribute);
    }
    void serialize(QDomLiteElement* xml) const {
        xml->setAttribute(ParameterTimeAttribute,time);
        xml->setAttribute(ParameterValueAttribute,value,0);
        xml->setAttribute(ParameterIDAttribute,id);
    }
    ulong64 time=0;
    int value=0;
    QString id;
};

typedef std::vector<CParameterEvent> CParameterEventList;

#define ParameterListSeparator "§"

class CParameter : protected IPresetRef
{
public:
    enum ParameterTypes {Numeric,SelectBox,dB,Percent};
    CParameter(ParameterTypes type, const QString& name, const QString& unit, const int min, const int max, const int decimalFactor, const QString& listString, const int value, IParameterHost* owner, const int index){
        //m_Wrapper = nullptr;
        m_OwnerDevice=owner;
        Type=type;
        Name=name;
        Unit=unit;
        Min=min;
        Max=max;
        DecimalFactor=decimalFactor;
        if (DecimalFactor==0) DecimalFactor=1;
        List=listString;
        Value=value;
        PercentValue = percentValue();
        DryValue = dryValue();
        Index = index;
    }
    void setControl(IParameterHost* control)
    {
        m_Wrapper = control;
    }
    void unserialize(const QDomLiteElement* XMLParameter)
    {
        if (XMLParameter)
        {
            setValue(XMLParameter->attributeValueInt(ParameterValueAttribute));
            for (const QDomLiteElement* e : (const QDomLiteElementList)XMLParameter->elementsByTag("AutomationEvent")) {
                events.push_back(CParameterEvent(e));
            }
        }
    }
    void serialize(QDomLiteElement* xml) const
    {
        xml->setAttribute(ParameterNameAttribute,Name);
        xml->setAttribute(ParameterValueAttribute,Value,0);
        xml->setAttribute("Index",Index);
        for (const CParameterEvent& e : events) {
            e.serialize(xml->appendChild("AutomationEvent"));
        }
    }
    int Index;
    int Min;
    int Max;
    ParameterTypes Type;
    QString Name;
    QString List;
    QString Unit;
    int DecimalFactor;
    int Value;
    float PercentValue;
    float DryValue;
    inline const QStringList stringList() const { return List.split(ParameterListSeparator); }
    inline float percentValue() const { return Value*0.01f; }
    inline float dryValue() const { return 1.f - percentValue(); }
    inline double decimalValue() const { return double(Value)/DecimalFactor; }
    inline float scaleValue(const float s) const { return Value*s; }
    inline double dBValue() const { return lin2dB(Value*0.01); }
    inline ulong64 mSec2samplesValue() const { return presets.mSecsToSamples(Value); }

    inline void setValue(const int v) {
        if (v != Value)
        {
            //QMutexLocker locker(&mutex);
            Value = v;
            PercentValue = percentValue();
            DryValue = dryValue();
            if (m_Wrapper) m_Wrapper->updateParameter(this);
            if (m_OwnerDevice) m_OwnerDevice->updateParameter(this);
        }
    }
    inline void setdBValue(double v) {
        setValue(dB2linf(v)*100);
    }
    inline void setPercentValue(float v) {
        setValue(v*100);
    }
    void appendEvent(ulong64 t, int v, const QString& id) {
        events.push_back(CParameterEvent(t,v,id));
        sortEvents(events);
    }
    void changeEvent(uint i ,ulong64 t, int v)
    {
        CParameterEvent& e = events[i];
        e.time = t;
        e.value = v;
    }
    void removeEvent(ulong64 t, int v)
    {
        for (uint i = events.size()-1; i >= 0; i--)
        {
            const CParameterEvent& e = events[i];
            if (e.time == t) {
                if (e.value == v) {
                    removeEvent(i);
                    break;
                }
            }
        }
    }
    void removeEvent(uint i)
    {
        events.erase(events.begin()+i);
    }
    void sortEvents(CParameterEventList& l) {
        std::sort(l.begin(),l.end(), [] (const CParameterEvent& a, const CParameterEvent& b){ return (a.time < b.time); });
    }
    CParameterEventList events;
    QString valueText() { return valueText(Value); }
    QString valueText(int val) {
        QString v;
        if (Type==CParameter::dB)
        {
            v=QString::number(lin2dB(val*0.01),'f',2)+" "+Unit;
        }
        else if (Type==CParameter::SelectBox)
        {
            const QStringList l=stringList();
            v=l[val];
        }
        else
        {
            const int decimals = QString::number(DecimalFactor).length()-1;
            v=QString::number(double(val)/DecimalFactor,'f',decimals);
            v += QChar(QChar::Space);
            v += Unit;
        }
        return v;
    }
private:
    IParameterHost* m_OwnerDevice = nullptr;
    IParameterHost* m_Wrapper = nullptr;
    QRecursiveMutex mutex;
};

typedef std::vector<CParameter*> CParameterList;

class CParameterGroup
{
public:
    CParameterGroup(const QString& name, const int start, const int index, const QColor& col = Qt::black, const int end = -1)
    {
        startIndex = start;
        endIndex = end;
        ID = index;
        color = col;
        Name = name;
    }
    int startIndex=-1;
    int endIndex=-1;
    int ID=-1;
    QString Name;
    QColor color;
};

#pragma pack(pop)

#endif // CPARAMETER_H
