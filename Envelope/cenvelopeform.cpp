#include "cenvelopeform.h"
#include "cenvelope.h"
#include "ui_cenvelopeform.h"

CEnvelopeForm::CEnvelopeForm(IDevice* Device, QWidget *parent) :
    CSoftSynthsForm(Device,false,parent),
    ui(new Ui::CEnvelopeForm)
{
    ui->setupUi(this);
    ADSRWidget=ui->ADSRWidget;
    parameters[CEnvelope::pnDelayTime]->connectToWidget(ui->ADSRWidget,&CADSRWidget::DelayChanged,&CADSRWidget::setDelay);
    parameters[CEnvelope::pnAttackTime]->connectToWidget(ui->ADSRWidget,&CADSRWidget::AttackChanged,&CADSRWidget::setAttack);
    parameters[CEnvelope::pnHoldTime]->connectToWidget(ui->ADSRWidget,&CADSRWidget::HoldChanged,&CADSRWidget::setHold);
    parameters[CEnvelope::pnDecayTime]->connectToWidget(ui->ADSRWidget,&CADSRWidget::DecayChanged,&CADSRWidget::setDecay);
    parameters[CEnvelope::pnSustainLevel]->connectToWidget(ui->ADSRWidget,&CADSRWidget::SustainChanged,&CADSRWidget::setSustain);
    parameters[CEnvelope::pnReleaseTime]->connectToWidget(ui->ADSRWidget,&CADSRWidget::ReleaseChanged,&CADSRWidget::setRelease);
}

CEnvelopeForm::~CEnvelopeForm()
{
    delete ui;
}
/*
void CEnvelopeForm::UpdateDevice(CADSR::ADSRParams ADSRParams)
{
    setParameter(CEnvelope::pnAttackTime,ADSRParams.Attack);
    setParameter(CEnvelope::pnDecayTime,ADSRParams.Decay);
    setParameter(CEnvelope::pnSustainLevel,ADSRParams.Sustain);
    setParameter(CEnvelope::pnReleaseTime,ADSRParams.Release);
}
*/
