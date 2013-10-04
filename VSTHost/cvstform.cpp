#include "cvstform.h"
#include "ui_cvstform.h"
#include <QVBoxLayout>

CVSTForm::CVSTForm(IAudioPlugInHost* plug, IDevice *Device, QWidget *parent) :
    CSoftSynthsForm(Device,true,parent),
    ui(new Ui::CVSTForm)
{
    ui->setupUi(this);

    QVBoxLayout* l=new QVBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);
    l->setContentsMargins(0,0,0,0);
    l->setSizeConstraint(QVBoxLayout::SetFixedSize);

    listHolder=new QWidget(this);
    l->addWidget(listHolder);

    QVBoxLayout* bl=new QVBoxLayout(listHolder);
    bl->setMargin(0);
    bl->setSpacing(0);
    bl->setContentsMargins(0,0,0,0);

    li=new QComboBox;
    //li->setFocusPolicy(Qt::ClickFocus);
    li->setAttribute(Qt::WA_MacShowFocusRect,false);

    bl->addWidget(li);

    listHolder->hide();

    connect(li,SIGNAL(currentIndexChanged(int)),this,SLOT(ChangeProgram(int)));

    PlugIn=plug;
    connect(PlugIn,SIGNAL(PlugInChanged()),this,SLOT(PlugInIndexChanged()));
    l->addWidget(PlugIn);
    startTimer(0);
}

CVSTForm::~CVSTForm()
{
    delete ui;
}

void CVSTForm::FillList(int CurrentProgram)
{
    li->blockSignals(true);
    li->clear();
    li->addItems(PlugIn->ProgramNames());
    listHolder->setVisible(li->count() > 1);
    if (CurrentProgram > -1)
    {
        li->setCurrentIndex(CurrentProgram);
    }
    li->blockSignals(false);
}

void CVSTForm::PlugInIndexChanged()
{
    FillList();
    setVisible(true);
    m_Device->UpdateHost();
}

void CVSTForm::ChangeProgram(int programIndex)
{
    PlugIn->SetProgram(programIndex);
}

void CVSTForm::SetProgram(const int programIndex)
{
    ChangeProgram(programIndex);
    if (li->count() > programIndex)
    {
        li->blockSignals(true);
        li->setCurrentIndex(programIndex);
        li->blockSignals(false);
    }
}

bool CVSTForm::event(QEvent *event)
{
    if (event->type()==QEvent::NonClientAreaMouseButtonPress)
    {
        if (((QMouseEvent*)event)->button()==Qt::RightButton)
        {
            PlugIn->Popup(mapToGlobal(((QMouseEvent*)event)->pos()));
        }
    }
    return CSoftSynthsForm::event(event);
}

void CVSTForm::timerEvent(QTimerEvent */*event*/)
{
    if (li->count())
    {
        int p=PlugIn->CurrentProgram();
        if (p != li->currentIndex())
        {
            li->blockSignals(true);
            li->setCurrentIndex(p);
            li->blockSignals(false);
        }
    }
}
