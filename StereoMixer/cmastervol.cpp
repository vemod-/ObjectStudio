#include "cmastervol.h"
#include "ui_cmastervol.h"
#include "softsynthsdefines.h"

CMasterVol::CMasterVol(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CMasterVol)
{
    ui->setupUi(this);
    ui->VolLabelL->setText("0.00 dB");
    ui->VolLabelR->setText("0.00 dB");
    connect(ui->VolL,&QAbstractSlider::valueChanged,this,&CMasterVol::setLeftVol);
    connect(ui->VolR,&QAbstractSlider::valueChanged,this,&CMasterVol::setRightVol);
    connect(ui->VolL,&QAbstractSlider::valueChanged,this,&CMasterVol::leftVolChanged);
    connect(ui->VolR,&QAbstractSlider::valueChanged,this,&CMasterVol::rightVolChanged);
    connect(ui->Lock,&QAbstractButton::toggled,this,&CMasterVol::setLock);
    connect(ui->Lock,&QAbstractButton::toggled,this,&CMasterVol::lockChanged);
    ui->Lock->setChecked(true);
    ui->VolL->setValue(100);
}

CMasterVol::~CMasterVol()
{
    delete ui;
}

void CMasterVol::setLeftVol(int vol)
{
    ui->VolL->blockSignals(true);
    ui->VolL->setValue(vol);
    ui->VolL->blockSignals(false);
    ui->VolLabelL->setText(percent2dBText(vol));
    if (ui->Lock->isChecked())
    {
        ui->VolR->blockSignals(true);
        ui->VolR->setValue(vol);
        ui->VolLabelR->setText(percent2dBText(vol));
        ui->VolR->blockSignals(false);
    }
}

void CMasterVol::setRightVol(int vol)
{
    ui->VolR->blockSignals(true);
    ui->VolR->setValue(vol);
    ui->VolR->blockSignals(false);
    ui->VolLabelR->setText(percent2dBText(vol));
    if (ui->Lock->isChecked())
    {
        ui->VolL->blockSignals(true);
        ui->VolL->setValue(vol);
        ui->VolLabelL->setText(percent2dBText(vol));
        ui->VolL->blockSignals(false);
    }
}

void CMasterVol::setLock(bool v)
{
    ui->Lock->blockSignals(true);
    ui->Lock->setChecked(v);
    ui->Lock->blockSignals(false);
}

int CMasterVol::leftVol()
{
    return ui->VolL->value();
}

int CMasterVol::rightVol()
{
    return ui->VolR->value();
}

bool CMasterVol::lock()
{
    return ui->Lock->isChecked();
}

void CMasterVol::peak(float l, float r)
{
    ui->Peak->setValues(l,r);
}

void CMasterVol::resetPeak()
{
    ui->Peak->reset();
}

void CMasterVol::showEvent(QShowEvent *)
{
    ui->Peak->setMargin(ui->VolL->grooveMargin());
}
