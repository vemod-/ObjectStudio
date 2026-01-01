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
private:
    enum JackNames
    {jnMIDIIn,jnOut};
    enum ParameterNames
    {pnMIDIChannel,pnVolume};
};

#endif // CPLUGINBOX_H
