#include "cadsrwidget.h"
#include "ui_cadsrwidget.h"

CADSRWidget::CADSRWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CADSRWidget)
{
    ui->setupUi(this);
    ui->Delay->setMaximum(ADSRControl::ADSR_MaxTime);
    ui->Attack->setMaximum(ADSRControl::ADSR_MaxTime);
    ui->Hold->setMaximum(ADSRControl::ADSR_MaxTime);
    ui->Decay->setMaximum(ADSRControl::ADSR_MaxTime);
    ui->Release->setMaximum(ADSRControl::ADSR_MaxTime);

    connect(ui->ADSRControl,&CADSRControl::Changed,this,&CADSRWidget::Changed);
    connect(ui->ADSRControl,&CADSRControl::DelayChanged,ui->Delay,&QSpinBox::setValue);
    connect(ui->ADSRControl,&CADSRControl::DelayChanged,this,&CADSRWidget::DelayChanged);
    connect(ui->Delay,qOverload<int>(&QSpinBox::valueChanged),ui->ADSRControl,&CADSRControl::setDelay);
    connect(ui->Delay,qOverload<int>(&QSpinBox::valueChanged),this,&CADSRWidget::DelayChanged);
    connect(ui->Delay,qOverload<int>(&QSpinBox::valueChanged),this,&CADSRWidget::emitChanged);

    connect(ui->ADSRControl,&CADSRControl::AttackChanged,ui->Attack,&QSpinBox::setValue);
    connect(ui->ADSRControl,&CADSRControl::AttackChanged,this,&CADSRWidget::AttackChanged);
    connect(ui->Attack,qOverload<int>(&QSpinBox::valueChanged),ui->ADSRControl,&CADSRControl::setAttack);
    connect(ui->Attack,qOverload<int>(&QSpinBox::valueChanged),this,&CADSRWidget::AttackChanged);
    connect(ui->Attack,qOverload<int>(&QSpinBox::valueChanged),this,&CADSRWidget::emitChanged);

    connect(ui->ADSRControl,&CADSRControl::HoldChanged,ui->Hold,&QSpinBox::setValue);
    connect(ui->ADSRControl,&CADSRControl::HoldChanged,this,&CADSRWidget::HoldChanged);
    connect(ui->Hold,qOverload<int>(&QSpinBox::valueChanged),ui->ADSRControl,&CADSRControl::setHold);
    connect(ui->Hold,qOverload<int>(&QSpinBox::valueChanged),this,&CADSRWidget::HoldChanged);
    connect(ui->Hold,qOverload<int>(&QSpinBox::valueChanged),this,&CADSRWidget::emitChanged);

    connect(ui->ADSRControl,&CADSRControl::DecayChanged,ui->Decay,&QSpinBox::setValue);
    connect(ui->ADSRControl,&CADSRControl::DecayChanged,this,&CADSRWidget::DecayChanged);
    connect(ui->Decay,qOverload<int>(&QSpinBox::valueChanged),ui->ADSRControl,&CADSRControl::setDecay);
    connect(ui->Decay,qOverload<int>(&QSpinBox::valueChanged),this,&CADSRWidget::DecayChanged);
    connect(ui->Decay,qOverload<int>(&QSpinBox::valueChanged),this,&CADSRWidget::emitChanged);

    connect(ui->ADSRControl,&CADSRControl::SustainChanged,ui->Sustain,&QSpinBox::setValue);
    connect(ui->ADSRControl,&CADSRControl::SustainChanged,this,&CADSRWidget::SustainChanged);
    connect(ui->Sustain,qOverload<int>(&QSpinBox::valueChanged),ui->ADSRControl,&CADSRControl::setSustain);
    connect(ui->Sustain,qOverload<int>(&QSpinBox::valueChanged),this,&CADSRWidget::SustainChanged);
    connect(ui->Sustain,qOverload<int>(&QSpinBox::valueChanged),this,&CADSRWidget::emitChanged);

    connect(ui->ADSRControl,&CADSRControl::ReleaseChanged,ui->Release,&QSpinBox::setValue);
    connect(ui->ADSRControl,&CADSRControl::ReleaseChanged,this,&CADSRWidget::ReleaseChanged);
    connect(ui->Release,qOverload<int>(&QSpinBox::valueChanged),ui->ADSRControl,&CADSRControl::setRelease);
    connect(ui->Release,qOverload<int>(&QSpinBox::valueChanged),this,&CADSRWidget::ReleaseChanged);
    connect(ui->Release,qOverload<int>(&QSpinBox::valueChanged),this,&CADSRWidget::emitChanged);

    UpdateGraph();
}

CADSRWidget::~CADSRWidget()
{
    delete ui;
}

void CADSRWidget::Update(CADSR::ADSRParams ADSRParams)
{
    UpdateControls(ADSRParams);
    ui->ADSRControl->Draw(ADSRParams);
}

void CADSRWidget::UpdateGraph()
{
    AP.Delay=ui->Delay->value();
    AP.Attack=ui->Attack->value();
    AP.Hold=ui->Hold->value();
    AP.Decay=ui->Decay->value();
    AP.Sustain=ui->Sustain->value();
    AP.Release=ui->Release->value();
    ui->ADSRControl->Draw(AP);
    emit Changed(AP);
}

void CADSRWidget::emitChanged()
{
    AP.Delay=ui->Delay->value();
    AP.Attack=ui->Attack->value();
    AP.Hold=ui->Hold->value();
    AP.Decay=ui->Decay->value();
    AP.Sustain=ui->Sustain->value();
    AP.Release=ui->Release->value();
    emit Changed(AP);
}

void CADSRWidget::UpdateControls(CADSR::ADSRParams ADSRParams)
{
    AP=ADSRParams;
    ui->Delay->blockSignals(true);
    ui->Attack->blockSignals(true);
    ui->Hold->blockSignals(true);
    ui->Decay->blockSignals(true);
    ui->Sustain->blockSignals(true);
    ui->Release->blockSignals(true);
    ui->Delay->setValue(AP.Delay);
    ui->Attack->setValue(AP.Attack);
    ui->Hold->setValue(AP.Hold);
    ui->Decay->setValue(AP.Decay);
    ui->Sustain->setValue(AP.Sustain);
    ui->Release->setValue(AP.Release);

    ui->Delay->blockSignals(false);
    ui->Attack->blockSignals(false);
    ui->Hold->blockSignals(false);
    ui->Decay->blockSignals(false);
    ui->Sustain->blockSignals(false);
    ui->Release->blockSignals(false);
}

void CADSRWidget::setDelay(int v)
{
    ui->Delay->blockSignals(true);
    ui->Delay->setValue(v);
    ui->Delay->blockSignals(false);
    ui->ADSRControl->setDelay(v);
}

void CADSRWidget::setAttack(int v)
{
    ui->Attack->blockSignals(true);
    ui->Attack->setValue(v);
    ui->Attack->blockSignals(false);
    ui->ADSRControl->setAttack(v);
}

void CADSRWidget::setHold(int v)
{
    ui->Hold->blockSignals(true);
    ui->Hold->setValue(v);
    ui->Hold->blockSignals(false);
    ui->ADSRControl->setHold(v);
}

void CADSRWidget::setDecay(int v)
{
    ui->Decay->blockSignals(true);
    ui->Decay->setValue(v);
    ui->Decay->blockSignals(false);
    ui->ADSRControl->setDecay(v);
}

void CADSRWidget::setSustain(int v)
{
    ui->Sustain->blockSignals(true);
    ui->Sustain->setValue(v);
    ui->Sustain->blockSignals(false);
    ui->ADSRControl->setSustain(v);
}

void CADSRWidget::setRelease(int v)
{
    ui->Release->blockSignals(true);
    ui->Release->setValue(v);
    ui->Release->blockSignals(false);
    ui->ADSRControl->setRelease(v);
}
