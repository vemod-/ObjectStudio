#include "ctunerform.h"
#include "ui_ctunerform.h"

CTunerForm::CTunerForm(IDevice* Device, QWidget *parent) :
    CSoftSynthsForm(Device,true,parent),
    PD(presets.SampleRate),
    ui(new Ui::CTunerForm)
{
    ui->setupUi(this);
}

CTunerForm::~CTunerForm()
{
    delete ui;
}

void CTunerForm::setPitchRecord() {
    ui->TunerWidget->setPitchRecord(PD.CurrentPitchRecord());
}

void CTunerForm::setRate(int r) {
    PD.setPitchRecordsPerSecond(1000/r);
}

void CTunerForm::setCalib(double c){
    ui->TunerWidget->setCalib(c);
    PD.setTune(c);
}


