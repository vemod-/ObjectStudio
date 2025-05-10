#ifndef CPLUGINBOX_H
#define CPLUGINBOX_H

#include "idevice.h"

class CPlugInBox : public IDevice
{
public:
    CPlugInBox();
    ~CPlugInBox();
    void init(const int Index, QWidget* MainWindow);
    float getNext(const int ProcIndex);
    CMIDIBuffer* getNextP(const int ProcIndex);
    CAudioBuffer* getNextA(const int ProcIndex);
    //void hideForm();
private:
    enum JackNames
    {jnMIDIIn,jnOut};
    enum ParameterNames
    {pnMIDIChannel,pnVolume};
    QList<CInJack*> InsideJacks;
    QList<IJack*> JacksCreated;
    //QRecursiveMutex mutex;
};

#endif // CPLUGINBOX_H
