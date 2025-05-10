#include "cstereopeak.h"
#include "ui_cstereopeak.h"

CStereoPeak::CStereoPeak(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CStereoPeak)
{
    ui->setupUi(this);
}

CStereoPeak::~CStereoPeak()
{
    delete ui;
}

void CStereoPeak::setValues(const float L, const float R)
{
    ui->PeakL->setValue(L);
    ui->PeakR->setValue(R);
}

void CStereoPeak::reset()
{
    ui->PeakL->reset();
    ui->PeakR->reset();
}

void CStereoPeak::setMargin(int margin)
{
    ui->PeakL->setMargin(margin);
    ui->PeakR->setMargin(margin);
    ui->Scale->setMargin(margin);
}

void CStereoPeak::setMax(int max)
{
    ui->PeakL->setMax(max);
    ui->PeakR->setMax(max);
    ui->Scale->setMax(max);
}
