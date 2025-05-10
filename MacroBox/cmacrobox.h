#ifndef CMACROBOX_H
#define CMACROBOX_H

#include "idevice.h"

class CMacroBox : public IDevice
{
public:
    CMacroBox();
    ~CMacroBox();
    void init(const int Index, QWidget* MainWindow);
    float getNext(const int ProcIndex);
    CMIDIBuffer* getNextP(const int ProcIndex);
    CAudioBuffer* getNextA(const int ProcIndex);
    //void hideForm();
private:
    /*
    enum JackNames
    {jnMIDIIn,jnOut};
    enum ParameterNames
    {pnMIDIChannel,pnVolume};
    */
    QList<CInJack*> InsideJacks;
    QList<IJack*> JacksCreated;
    //QRecursiveMutex mutex;
};

#endif // CMACROBOX_H
