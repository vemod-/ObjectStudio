#include "cstereosplitter.h"

CStereoSplitter::CStereoSplitter()
{
}

void CStereoSplitter::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackStereoIn();
    addJackDualMonoOut(leftmonoout);
}

void CStereoSplitter::process()
{
    m_Input=FetchAStereo(stereoin);
}

CAudioBuffer* CStereoSplitter::getNextA(const int ProcIndex)
{
    if (m_Process)
    {
        process();
        m_Process=false;
    }
    if (ProcIndex==rightmonoout) return m_Input->rightBuffer;
    /*
    {
        OutR.fromRawData(m_Input->dataR());
        return &OutR;
    }
    */
    if (ProcIndex==leftmonoout) return m_Input->leftBuffer;
    /*
    {
        OutL.fromRawData(m_Input->data());
        return &OutL;
    }
    */
    return m_Input;
}
