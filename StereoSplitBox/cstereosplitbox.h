#ifndef CSTEREOSPLITBOX_H
#define CSTEREOSPLITBOX_H

#include "idevice.h"

#define devicejacks stereoout,stereoin
#define devicecategory Container | Effect

class CStereoSplitBox  : public IDevice
{
public:
    CStereoSplitBox();
    ~CStereoSplitBox();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
    //void hideForm();
private:
    enum JackNames
    {devicejacks,jnOutLeft,jnOutRight,jnInLeft,jnInRight};
    enum ParameterNames
    {};
    void process();
    //QList<IJack*> JacksCreated;
    CInJack* WaveOutL;
    CInJack* WaveOutR;
    CStereoBuffer* InBuffer = nullptr;
};

#endif // CSTEREOSPLITBOX_H
