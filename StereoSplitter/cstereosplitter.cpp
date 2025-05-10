#include "cstereosplitter.h"

CStereoSplitter::CStereoSplitter()
{
}

void CStereoSplitter::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackStereoIn();
    addJackDualMonoOut(jnOutLeft);
}

void CStereoSplitter::process()
{
    m_Input=FetchAStereo(jnIn);
}

CAudioBuffer* CStereoSplitter::getNextA(const int ProcIndex)
{
    if (m_Process)
    {
        process();
        m_Process=false;
    }
    if (ProcIndex==jnOutRight) return m_Input->rightBuffer;
    /*
    {
        OutR.fromRawData(m_Input->dataR());
        return &OutR;
    }
    */
    if (ProcIndex==jnOutLeft) return m_Input->leftBuffer;
    /*
    {
        OutL.fromRawData(m_Input->data());
        return &OutL;
    }
    */
    return m_Input;
}
