#include "caudiounitform.h"
#include "ui_caudiounitform.h"
#include <QVBoxLayout>

CAudioUnitForm::CAudioUnitForm(IDevice *Device, QWidget *parent) :
    CSoftSynthsForm(Device,true,parent),
    ui(new Ui::CAudioUnitForm)
{
    ui->setupUi(this);
    AU=NULL;

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

    li=new QComboBox(this);
    li->setFocusPolicy(Qt::ClickFocus);
    li->setAttribute(Qt::WA_MacShowFocusRect,false);

    bl->addWidget(li);

    listHolder->hide();

    connect(li,SIGNAL(currentIndexChanged(int)),this,SLOT(ChangeProgram(int)));

    AU=new CAudioUnitClass(CPresets::Presets.SampleRate,CPresets::Presets.ModulationRate,this);
    connect(AU,SIGNAL(UnitIndexChanged()),this,SLOT(UnitIndexChanged()));
    l->addWidget(AU);
    startTimer(0);
}

CAudioUnitForm::~CAudioUnitForm()
{
    delete ui;
}

void CAudioUnitForm::FillList(int CurrentProgram)
{
    li->blockSignals(true);
    li->clear();
    li->addItems(AU->ProgramNames());
    listHolder->setVisible(li->count() > 1);
    if (CurrentProgram > -1)
    {
        li->setCurrentIndex(CurrentProgram);
    }
    li->blockSignals(false);
}

void CAudioUnitForm::UnitIndexChanged()
{
    FillList();
    setVisible(true);
    m_Device->UpdateHost();
}

void CAudioUnitForm::ChangeProgram(int programIndex)
{
    AU->setCurrentProgram(programIndex);
}

void CAudioUnitForm::SetProgram(const int index)
{
    ChangeProgram(index);
    if (li->count() > index)
    {
        li->blockSignals(true);
        li->setCurrentIndex(index);
        li->blockSignals(false);
    }
}

bool CAudioUnitForm::event(QEvent *event)
{
    if (event->type()==QEvent::NonClientAreaMouseButtonPress)
    {
        if (((QMouseEvent*)event)->button()==Qt::RightButton)
        {
            AU->Popup(mapToGlobal(((QMouseEvent*)event)->pos()));
        }
    }
    return QDialog::event(event);
}

void CAudioUnitForm::timerEvent(QTimerEvent */*event*/)
{
    if (li->count())
    {
        int p=AU->getCurrentProgram();
        if (p != li->currentIndex())
        {
            li->blockSignals(true);
            li->setCurrentIndex(p);
            li->blockSignals(false);
        }
    }
}
