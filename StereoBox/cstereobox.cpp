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
        FORMFUNC(CMacroBoxForm)->DesktopComponent->clear();
        qDeleteAll(JacksCreated);
    }
}

void CStereoBox::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackStereoOut(jnOut);
    addJackDualMonoOut(jnOutLeft);
    addJackStereoIn();
    addJackDualMonoIn();

    m_Form=new CMacroBoxForm(this,MainWindow);
    CDesktopComponent* d=FORMFUNC(CMacroBoxForm)->DesktopComponent;
    addTickerDevice(d->deviceList());
    setDeviceParent(d->deviceList());
    d->deviceList()->setPolyphony(2);

    WaveOutL=new CInJack("Out","This",IJack::Wave,IJack::In,this);
    JacksCreated.append(d->addJack(WaveOutL,0));
    WaveOutR=new CInJack("Out","This",IJack::Wave,IJack::In,this);
    JacksCreated.append(d->addJack(WaveOutR,1));
    JacksCreated.append(d->addJack(new COutJack("In","This",IJack::Wave,IJack::Out,this,jnInsideInLeft),0));
    JacksCreated.append(d->addJack(new COutJack("In","This",IJack::Wave,IJack::Out,this,jnInsideInRight),1));
}

void CStereoBox::process()
{
    const CStereoBuffer* B = FetchAStereo(jnIn);
    CMonoBuffer* BL = FetchAMono(jnInLeft);
    CMonoBuffer* BR = FetchAMono(jnInRight);
    if (!B->isValid())
    {
        InL=BL;
        InR=BR;
    }
    else if ((!BL->isValid()) && (!BR->isValid()))
    {
        InL=B->leftBuffer;
        InR=B->rightBuffer;
    }
    else
    {
        InBuffer.writeBuffer(B);
        InBuffer.addDualMono(BL,BR);
        InBuffer *= M_SQRT1_2_F;
        InL=InBuffer.leftBuffer;
        InR=InBuffer.rightBuffer;
    }
    StereoBuffer(jnOut)->fromDualMono(WaveOutL->getNextA()->data(),WaveOutR->getNextA()->data());
}

CAudioBuffer* CStereoBox::getNextA(const int ProcIndex)
{
    if (m_Process)
    {
        m_Process=false;
        process();
    }
    if (ProcIndex==jnInsideInLeft) return InL;
    if (ProcIndex==jnInsideInRight) return InR;
    CStereoBuffer* OutBuffer=StereoBuffer(jnOut);
    if (ProcIndex==jnOutRight) return OutBuffer->rightBuffer;
    if (ProcIndex==jnOutLeft) return OutBuffer->leftBuffer;
    return OutBuffer;
}

