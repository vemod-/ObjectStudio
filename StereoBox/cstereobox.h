#ifndef CSTEREOBOX_H
#define CSTEREOBOX_H

#include "idevice.h"

class CStereoBox : public IDevice
{
public:
    CStereoBox();
    ~CStereoBox();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
private:
    enum JackNames
    {jnOut,jnOutLeft,jnOutRight,jnIn,jnInLeft,jnInRight,jnInsideInLeft,jnInsideInRight};
    enum ParameterNames
    {};
    void process();
    QList<IJack*> JacksCreated;
    CInJack* WaveOutL;
    CInJack* WaveOutR;
    CMonoBuffer* InL;
    CMonoBuffer* InR;
    CStereoBuffer InBuffer;
};


#endif // CSTEREOBOX_H
