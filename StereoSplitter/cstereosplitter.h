#ifndef CSTEREOSPLITTER_H
#define CSTEREOSPLITTER_H

#include "idevice.h"

class CStereoSplitter : public IDevice
{
public:
    CStereoSplitter();
    void init(const int Index, QWidget* MainWindow);
    void process();
    CAudioBuffer* getNextA(const int ProcIndex);
private:
    enum JackNames
    {jnIn,jnOutLeft,jnOutRight};
    CStereoBuffer* m_Input;
    //CMonoBuffer OutL;
    //CMonoBuffer OutR;
};

#endif // CSTEREOSPLITTER_H
