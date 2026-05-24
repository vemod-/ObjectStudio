#include "cstereobox.h"
#include "cmacroboxform.h"

CStereoBox::CStereoBox()
{
}

CStereoBox::~CStereoBox()
{
    qDebug() << "~CStereoBox";
    if (m_Initialized)
    {
        FORMFUNC(CMacroBoxForm)->DesktopComponent->clearJacksCreated();
        //qDeleteAll(JacksCreated);
    }
}

void CStereoBox::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackStereoOut(stereoout);
    addJackDualMonoOut(leftmonoout);
    addJackStereoIn();
    addJackDualMonoIn();

    m_Form=new CMacroBoxForm(this,MainWindow);
    CDesktopComponent* d = FORMFUNC(CMacroBoxForm)->DesktopComponent;
    addTickerDevice(d->deviceList());
    setDeviceParent(d->deviceList());
    d->deviceList()->setPolyphony(2);

    WaveOutL = new CInJack("Out","This",IJack::Mono,IJack::In,this);
    d->JacksCreated.append(d->addJack(WaveOutL,0));
    WaveOutR = new CInJack("Out","This",IJack::Mono,IJack::In,this);
    d->JacksCreated.append(d->addJack(WaveOutR,1));
    d->JacksCreated.append(d->addJack(new COutJack("In","This",IJack::Mono,IJack::Out,this,jnInsideInLeft),0));
    d->JacksCreated.append(d->addJack(new COutJack("In","This",IJack::Mono,IJack::Out,this,jnInsideInRight),1));
}

void CStereoBox::process()
{
    const CStereoBuffer* B = FetchAStereo(stereoin);
    CMonoBuffer* BL = FetchAMono(leftmonoin);
    CMonoBuffer* BR = FetchAMono(rightmonoin);
    if (!B->isValid())
    {
        InL = BL;
        InR = BR;
    }
    else if ((!BL->isValid()) && (!BR->isValid()))
    {
        InL = B->leftBuffer;
        InR = B->rightBuffer;
    }
    else
    {
        InBuffer.writeBuffer(B);
        InBuffer.addDualMono(BL,BR);
        InBuffer *= M_SQRT1_2_F;
        InL = InBuffer.leftBuffer;
        InR = InBuffer.rightBuffer;
    }
    StereoBuffer(stereoout)->fromDualMono(WaveOutL->getNextA()->data(),WaveOutR->getNextA()->data());
}

CAudioBuffer* CStereoBox::getNextA(const int ProcIndex)
{
    if (m_Process)
    {
        m_Process=false;
        process();
    }
    if (ProcIndex == jnInsideInLeft) return InL;
    if (ProcIndex == jnInsideInRight) return InR;
    CStereoBuffer* OutBuffer=StereoBuffer(stereoout);
    if (ProcIndex == rightmonoout) return OutBuffer->rightBuffer;
    if (ProcIndex == leftmonoout) return OutBuffer->leftBuffer;
    return OutBuffer;
}

