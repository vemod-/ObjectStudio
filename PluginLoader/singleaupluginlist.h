#ifndef SINGLEAUPLUGINLIST_H
#define SINGLEAUPLUGINLIST_H

#include <QtCore>
#include "CAComponent.h"
#include "CAComponentDescription.h"
#include "CAStreamBasicDescription.h"

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
        last = &(ioCompBuffer);
    }
    return l;
}

class SingleAUPlugInList : public QMap<OSType,QList<CAComponent> >
{
public:
    static SingleAUPlugInList* getInstance()
    {
        static SingleAUPlugInList    instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return &instance;
    }
private:
    SingleAUPlugInList()
    {
        if (isEmpty())
        {
            QList<CAComponent> l=getComponentListForAUType(kAudioUnitType_MusicDevice);
            if (l.count()) insert(kAudioUnitType_MusicDevice,l);
            l=getComponentListForAUType(kAudioUnitType_Effect);
            if (l.count()) insert(kAudioUnitType_Effect,l);
            l=getComponentListForAUType(kAudioUnitType_MusicEffect);
            if (l.count()) insert(kAudioUnitType_MusicEffect,l);
            l=getComponentListForAUType(kAudioUnitType_Generator);
            if (l.count()) insert(kAudioUnitType_Generator,l);
        }
    }                   // Constructor? (the {} brackets) are needed here.
    SingleAUPlugInList(SingleAUPlugInList const&);              // Don't Implement
    void operator=(SingleAUPlugInList const&); // Don't implement
};

#endif // SINGLEAUPLUGINLIST_H
