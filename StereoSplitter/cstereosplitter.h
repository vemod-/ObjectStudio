#ifndef CSTEREOSPLITTER_H
#define CSTEREOSPLITTER_H

#include "idevice.h"
#define devicecategory SynthModule

#define devicejacks stereoin,leftmonoout,rightmonoout

class CStereoSplitter : public IDevice
{
public:
    CStereoSplitter();
    void init(const int Index, QWidget* MainWindow);
    void process();
    CAudioBuffer* getNextA(const int ProcIndex);
private:
    enum JackNames
    {devicejacks};
    CStereoBuffer* m_Input;
    //CMonoBuffer OutL;
    //CMonoBuffer OutR;
};

#endif // CSTEREOSPLITTER_H
