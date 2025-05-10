#include "cstereosplitbox.h"
#include "cmacroboxform.h"

CStereoSplitBox::CStereoSplitBox()
{
}

CStereoSplitBox::~CStereoSplitBox()
{
    qDebug() << "~CStereoSplitBox";
    if (m_Initialized)
    {
        FORMFUNC(CMacroBoxForm)->DesktopComponent->clear();
        qDeleteAll(JacksCreated);
    }
}

void CStereoSplitBox::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackStereoOut(jnOut);
    addJackStereoIn();

    m_Form=new CMacroBoxForm(this,MainWindow);
    CDesktopComponent* d=FORMFUNC(CMacroBoxForm)->DesktopComponent;
    addTickerDevice(d->deviceList());
    setDeviceParent(d->deviceList());

    WaveOutL=new CInJack("Out Left","This",IJack::Wave,IJack::In,this);
    JacksCreated.append(d->addJack(WaveOutL,0));
    WaveOutR=new CInJack("Out Right","This",IJack::Wave,IJack::In,this);
    JacksCreated.append(d->addJack(WaveOutR,0));
    JacksCreated.append(d->addJack(new COutJack("In Left","This",IJack::Wave,IJack::Out,this,jnInLeft),0));
    JacksCreated.append(d->addJack(new COutJack("In Right","This",IJack::Wave,IJack::Out,this,jnInRight),0));
}
/*
void CStereoSplitBox::hideForm()
{
    FORMFUNC(CMacroBoxForm)->DesktopComponent->hideForms();
    m_Form->setVisible(false);
}
*/
void CStereoSplitBox::process()
{
    InBuffer = FetchAStereo(jnIn);
}

CAudioBuffer* CStereoSplitBox::getNextA(const int ProcIndex)
{
    if (ProcIndex==jnOut) StereoBuffer(jnOut)->fromDualMono(WaveOutL->getNextA()->data(),WaveOutR->getNextA()->data());
    if (ProcIndex==jnInLeft)
    {
        if (m_Process)
        {
            m_Process=false;
            process();
        }
        if (InBuffer->leftBuffer->isValid()) return InBuffer->leftBuffer;
        return nullptr;
    }
    if (ProcIndex==jnInRight)
    {
        if (m_Process)
        {
            m_Process=false;
            process();
        }
        if (InBuffer->rightBuffer->isValid()) return InBuffer->rightBuffer;
        return nullptr;
    }
    return m_AudioBuffers[ProcIndex];
}

