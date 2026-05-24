#ifndef CSTEREOBOX_H
#define CSTEREOBOX_H

#include "idevice.h"

#define devicejacks stereoout,leftmonoout,rightmonoout,stereoin,leftmonoin,rightmonoin
#define devicecategory Container | Effect | Generator

class CStereoBox : public IDevice
{
public:
    CStereoBox();
    ~CStereoBox();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
private:
    enum JackNames
    {devicejacks,jnInsideInLeft,jnInsideInRight};
    enum ParameterNames
    {};
    void process();
    //QList<IJack*> JacksCreated;
    CInJack* WaveOutL;
    CInJack* WaveOutR;
    CMonoBuffer* InL;
    CMonoBuffer* InR;
    CStereoBuffer InBuffer;
};


#endif // CSTEREOBOX_H
