#ifndef CSTEREOSPLITBOX_H
#define CSTEREOSPLITBOX_H

#include "idevice.h"

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
    {jnOut,jnIn,jnOutLeft,jnOutRight,jnInLeft,jnInRight};
    enum ParameterNames
    {};
    void process();
    QList<IJack*> JacksCreated;
    CInJack* WaveOutL;
    CInJack* WaveOutR;
    CStereoBuffer* InBuffer;
};

#endif // CSTEREOSPLITBOX_H
