#include "cmacrobox.h"
#include "cmacroboxform.h"

CMacroBox::CMacroBox()
{
}

CMacroBox::~CMacroBox()
{
    if (m_Initialized)
    {
        ((CMacroBoxForm*)m_Form)->DesktopComponent->Clear();
        qDeleteAll(JacksCreated);
    }
}

void CMacroBox::Init(const int Index, void *MainWindow)
{
    m_Name=devicename;
    IDevice::Init(Index,MainWindow);
    m_Form=new CMacroBoxForm(this,(QWidget*)MainWindow);
    CDesktopComponent* d=((CMacroBoxForm*)m_Form)->DesktopComponent;
    AddJackDualMonoOut(0);
    AddJackMIDIOut(2);
    AddJack("Amplitude Modulation Out",IJack::Amplitude,IJack::Out,3);
    AddJack("Pitch Modulation Out",IJack::Pitch,IJack::Out,4);
    AddJack("Frequency Out",IJack::Frequency,IJack::Out,5);
    AddJack("Trigger Out",IJack::Trigger,IJack::Out,6);
    AddJackDualMonoIn();
    AddJackMIDIIn();
    AddJack("Amplitude Modulation In",IJack::Amplitude,IJack::In);
    AddJack("Pitch Modulation In",IJack::Pitch,IJack::In);
    AddJack("Frequency In",IJack::Frequency,IJack::In);
    AddJack("Trigger In",IJack::Trigger,IJack::In);
    for (unsigned int i=0;i<m_Jacks.size();i++)
    {
        IJack* J=(IJack*)m_Jacks[i];
        IJack* J1=d->AddJack(d->CreateInsideJack(i,J,this),0);
        JacksCreated.push_back(J1);
        if (J->Direction==IJack::Out)
        {
            InsideJacks.push_back(J1);
        }
        else
        {
            InsideJacks.push_back(J);
        }
    }
}

void CMacroBox::Tick()
{
    ((CMacroBoxForm*)m_Form)->DesktopComponent->Tick();
}

void CMacroBox::HideForm()
{
    ((CMacroBoxForm*)m_Form)->DesktopComponent->HideForms();
    m_Form->setVisible(false);
}

const float CMacroBox::GetNext(const int ProcIndex)
{
    return ((CInJack*)InsideJacks[ProcIndex])->GetNext();
}

void* CMacroBox::GetNextP(const int ProcIndex)
{
    return ((CInJack*)InsideJacks[ProcIndex])->GetNextP();
}

float* CMacroBox::GetNextA(const int ProcIndex)
{
    return ((CInJack*)InsideJacks[ProcIndex])->GetNextA();
}

void CMacroBox::Play(const bool FromStart)
{
    ((CMacroBoxForm*)m_Form)->DesktopComponent->Play(FromStart);
}

void CMacroBox::Pause()
{
    ((CMacroBoxForm*)m_Form)->DesktopComponent->Pause();
}

