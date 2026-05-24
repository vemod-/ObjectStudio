#ifndef CPLUGINBOX_H
#define CPLUGINBOX_H

#include "idevice.h"

#define devicejacks stereoout,stereoin,midiin
#define devicecategory Effect | Generator | Container

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
    {devicejacks};
    enum ParameterNames
    {pnMIDIChannel,pnVolume};
};

#endif // CPLUGINBOX_H
