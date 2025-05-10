#ifndef SINGLEAUPLUGINLIST_H
#define SINGLEAUPLUGINLIST_H

#include <QtCore>
#include "CAComponent.h"
#include "CAComponentDescription.h"
#include "CAStreamBasicDescription.h"
#include "macstrings.h"

int componentCountForAUType(OSType inAUType)
{
    CAComponentDescription desc = CAComponentDescription(inAUType);
    return desc.Count();
}

QList<CAComponent> getComponentListForAUType(OSType inAUType)
{
    CAComponentDescription desc = CAComponentDescription(inAUType);
    CAComponent *last = NULL;
    QList<CAComponent> l;

    for (int i = 0; i < componentCountForAUType(inAUType); ++i) {
        CAComponent ioCompBuffer = CAComponent(desc, last);
        l.append(ioCompBuffer);
        last = &(l.last());
    }
    return l;
}

class SingleAUPlugInList : public QMap<OSType,QList<CAComponent> >
{
public:
    static const QList<CAComponent> audioUnits(OSType type)
    {
        return getInstance()->value(type);
    }
    static const CAComponent audioUnit(int type, int subtype)
    {
        return getInstance()->value(getInstance()->m_AudioUnitTypes.at(type)).at(subtype);
    }
    static const QStringList audioUnitTypeNames()
    {
        return getInstance()->m_AudioUnitTypeNames;
    }
    static const QString audioUnitTypeName(int type)
    {
        return audioUnitTypeNames().at(type);
    }
    static const QString audioUnitTypeName(OSType type)
    {
        return audioUnitTypeName(typeIndex(type));
    }
    static const QList<OSType> audioUnitTypes()
    {
        return getInstance()->m_AudioUnitTypes;
    }
    static OSType audioUnitType(int type)
    {
        return audioUnitTypes().at(type);
    }
    static const QStringList audioUnitNames(OSType type=kAudioUnitType_Effect)
    {
        QStringList RetVal;
        QList<CAComponent> l=getInstance()->value(type);
        for (int i = 0; i < l.size(); ++i)
        {
            RetVal << qt_mac_NSStringToQString(l.at(i).GetAUName());
        }
        return RetVal;
    }
    static const QStringList audioUnitNames(int type)
    {
        return audioUnitNames(audioUnitType(type));
    }
    static int typeIndex(OSType type)
    {
        return getInstance()->m_AudioUnitTypes.indexOf(type);
    }
    static int subTypeIndex(OSType type, OSType subtype)
    {
        QList<CAComponent> l=getInstance()->value(type);
        for (int i = 0; i < l.size(); ++i)
        {
            if (l.at(i).Desc().componentSubType==subtype) return i;
        }
        return -1;
    }
    static const QString audioUnitName(OSType type, OSType subtype)
    {
        const int subIndex = subTypeIndex(type,subtype);
        if (subIndex == -1) return QString();
        return audioUnitNames(type)[subIndex];
    }
    static const QString audioUnitName(int type, int subtype)
    {
        return audioUnitNames(audioUnitTypes().at(type))[subtype];
    }
private:
    static SingleAUPlugInList* getInstance()
    {
        static SingleAUPlugInList    instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return &instance;
    }
    SingleAUPlugInList()
    {
        if (isEmpty())
        {
            QList<CAComponent> l=getComponentListForAUType(kAudioUnitType_MusicDevice);
            if (l.size()) insert(kAudioUnitType_MusicDevice,l);
            l=getComponentListForAUType(kAudioUnitType_Effect);
            if (l.size()) insert(kAudioUnitType_Effect,l);
            l=getComponentListForAUType(kAudioUnitType_MusicEffect);
            if (l.size()) insert(kAudioUnitType_MusicEffect,l);
            l=getComponentListForAUType(kAudioUnitType_Generator);
            if (l.size()) insert(kAudioUnitType_Generator,l);
            m_AudioUnitTypeNames=QStringList() << "Instruments" << "Effects" << "Music Effects" << "Generators";
            m_AudioUnitTypes=QList<OSType>() << kAudioUnitType_MusicDevice << kAudioUnitType_Effect << kAudioUnitType_MusicEffect << kAudioUnitType_Generator;
        }
    }                   // Constructor? (the {} brackets) are needed here.
    SingleAUPlugInList(SingleAUPlugInList const&);              // Don't Implement
    void operator=(SingleAUPlugInList const&); // Don't implement
    QStringList m_AudioUnitTypeNames;
    QList<OSType> m_AudioUnitTypes;
};

#endif // SINGLEAUPLUGINLIST_H
