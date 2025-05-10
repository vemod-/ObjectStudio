#include "ckeyboardform.h"
#include "ui_ckeyboardform.h"
#include "ckeyboard.h"
#include "mouseevents.h"

CKeyboardForm::CKeyboardForm(IDevice* Device, QWidget *parent) :
    CSoftSynthsForm(Device,true,parent),
    ui(new Ui::CKeyboardForm)
{
    ui->setupUi(this);
    connect(ui->piano,&OCPiano::NoteOnTriggered,this,&CKeyboardForm::NoteOn);
    connect(ui->piano,&OCPiano::NoteOffTriggered,this,&CKeyboardForm::NoteOff);
    auto e=new MouseEvents();
    ui->pitchSlider->installEventFilter(e);
    connect(e,&MouseEvents::MouseReleased,this,&CKeyboardForm::PitchWheelRelease);
    auto e1=new MouseEvents();
    ui->modulationSlider1->installEventFilter(e1);
    connect(e1,&MouseEvents::MouseReleased,this,&CKeyboardForm::ModWheel1Release);
    auto e2=new MouseEvents();
    ui->modulationSlider2->installEventFilter(e2);
    connect(e2,&MouseEvents::MouseReleased,this,&CKeyboardForm::ModWheel2Release);
}

CKeyboardForm::~CKeyboardForm()
{
    delete ui;
}

void CKeyboardForm::NoteOn(int pitch)
{
    auto* k = DEVICEFUNC(CKeyboard);
    k->notesOn.append(pitch);
    k->notesDown.append(pitch);
}

void CKeyboardForm::NoteOff(int pitch)
{
    auto* k = DEVICEFUNC(CKeyboard);
    k->notesOff.append(pitch);
    k->notesDown.removeOne(pitch);
}

int CKeyboardForm::pitchWheel()
{
    return ui->pitchSlider->value();
}

int CKeyboardForm::modWheel1()
{
    return ui->modulationSlider1->value();
}

int CKeyboardForm::modWheel2()
{
    return ui->modulationSlider2->value();
}

void CKeyboardForm::PitchWheelRelease()
{
    ui->pitchSlider->setValue(0);
}

void CKeyboardForm::ModWheel1Release()
{
    ui->modulationSlider1->setValue(0);
}

void CKeyboardForm::ModWheel2Release()
{
    ui->modulationSlider2->setValue(0);
}
