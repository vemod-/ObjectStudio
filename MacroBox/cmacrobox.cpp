#include "cmacrobox.h"
#include "cmacroboxform.h"

#undef devicename
#define devicename "MacroBox"

CMacroBox::CMacroBox()
{
    m_Form=nullptr;
}

CMacroBox::~CMacroBox()
{
    qDebug() << "~CMacroBox";
    if (m_Initialized)
    {
        FORMFUNC(CMacroBoxForm)->DesktopComponent->clear();
        qDeleteAll(JacksCreated);
    }
}

void CMacroBox::init(const int Index, QWidget* MainWindow)
{
    QMutexLocker locker(&mutex);
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackStereoOut(0);
    addJackDualMonoOut(1);
    addJackMIDIOut(3);
    addJackModulationOut(4,"Modulation Out");
    addJackModulationOut(5,"Frequency Out");
    addJackModulationOut(6,"Trigger Out");
    addJackStereoIn();
    addJackDualMonoIn();
    addJackMIDIIn();
    addJackModulationIn("Modulation In");
    addJackModulationIn("Frequency In");
    addJackModulationIn("Trigger In");
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
void CMacroBox::hideForm()
{
    FORMFUNC(CMacroBoxForm)->DesktopComponent->hideForms();
    m_Form->setVisible(false);
}
*/
float CMacroBox::getNext(const int ProcIndex)
{
    return InsideJacks[ProcIndex]->getNext();
}

CMIDIBuffer* CMacroBox::getNextP(const int ProcIndex)
{
    return InsideJacks[ProcIndex]->getNextP();
}

CAudioBuffer* CMacroBox::getNextA(const int ProcIndex)
{
    return InsideJacks[ProcIndex]->getNextA();
}

