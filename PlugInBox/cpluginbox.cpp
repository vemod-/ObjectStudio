#include "cpluginbox.h"
#include "cmacroboxform.h"

#undef devicename
#define devicename "PlugInBox"

CPlugInBox::CPlugInBox()
{
    m_Form=nullptr;
}

CPlugInBox::~CPlugInBox()
{
    qDebug() << "~CPlugInBox";
    if (m_Initialized)
    {
        FORMFUNC(CMacroBoxForm)->DesktopComponent->clear();
        qDeleteAll(JacksCreated);
    }
}

void CPlugInBox::init(const int Index, QWidget* MainWindow)
{
    QMutexLocker locker(&mutex);
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackStereoOut(0);
    addJackStereoIn();
    addJackMIDIIn();
    m_Form=new CMacroBoxForm(this,MainWindow);
    CDesktopComponent* d=FORMFUNC(CMacroBoxForm)->DesktopComponent;
    addTickerDevice(d->deviceList());
    setDeviceParent(d->deviceList());
    for (uint i=0;i<m_Jacks.size();i++)
    {
        IJack* J=m_Jacks[i];
        IJack* J1=d->addJack(J->createInsideJack(i,this),0);
        JacksCreated.append(J1);
        (J->isOutJack()) ? InsideJacks.append(dynamic_cast<CInJack*>(J1)) : InsideJacks.append(dynamic_cast<CInJack*>(J));
    }
}
/*
void CPlugInBox::hideForm()
{
    FORMFUNC(CMacroBoxForm)->DesktopComponent->hideForms();
    m_Form->setVisible(false);
}
*/
float CPlugInBox::getNext(const int ProcIndex)
{
    return InsideJacks[ProcIndex]->getNext();
}

CMIDIBuffer* CPlugInBox::getNextP(const int ProcIndex)
{
    return InsideJacks[ProcIndex]->getNextP();
}

CAudioBuffer* CPlugInBox::getNextA(const int ProcIndex)
{
    return InsideJacks[ProcIndex]->getNextA();
}

