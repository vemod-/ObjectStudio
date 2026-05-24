#include "cpanner.h"

CPanner::CPanner()
{
}

void CPanner::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackStereoIn();
    addJackStereoOut(stereoout);
    addJackDualMonoOut(leftmonoout);
    addJackModulationIn("Modulation In");
    addParameterPan();
    addParameterPercent();
    Modulator.init(m_Jacks[modulationin],m_Parameters[pnModulation]);
    LeftModFactor=1;
    RightModFactor=1;
    updateDeviceParameter();
}

void CPanner::process()
{
    InSignal=FetchAStereo(stereoin);
    if (!InSignal->isValid()) return;
    const float CurrentMod = Modulator.exec();
    if (Modulator.changed())
    {
        LeftModFactor=1;
        RightModFactor=1;
        if (CurrentMod > 0) LeftModFactor -= CurrentMod;
        else if (CurrentMod < 0) RightModFactor += CurrentMod;
    }
    StereoBuffer(stereoout)->writeStereoBuffer(InSignal->data(),LeftFactor*LeftModFactor,RightFactor*RightModFactor);
}

CAudioBuffer *CPanner::getNextA(const int ProcIndex)
{
    if (m_Process)
    {
        m_Process=false;
        process();
    }
    if (!InSignal->isValid())
    {
        if (ProcIndex == stereoout) return nullptr;//&m_NullBufferStereo;
        return nullptr;//&m_NullBufferMono;
    }
    CStereoBuffer* OutBuffer=StereoBuffer(stereoout);
    if (ProcIndex==leftmonoout) return OutBuffer->rightBuffer;
    if (ProcIndex==rightmonoout) return OutBuffer->leftBuffer;
    return OutBuffer;
}

void CPanner::updateDeviceParameter(const CParameter* /*p*/)
{
    LeftFactor=1;
    RightFactor=1;
    if (m_Parameters[pnPan]->Value > 0) LeftFactor -= m_Parameters[pnPan]->PercentValue;
    else if (m_Parameters[pnPan]->Value < 0) RightFactor += m_Parameters[pnPan]->PercentValue;
}
