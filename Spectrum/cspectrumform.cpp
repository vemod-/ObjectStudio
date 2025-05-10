#include "cspectrumform.h"
#include "ui_cspectrumform.h"

CSpectrumForm::CSpectrumForm(IDevice* Device, QWidget *parent) :
    CSoftSynthsForm(Device,false,parent),
    ui(new Ui::CSpectrumForm)
{
    ui->setupUi(this);
    Spectrum = ui->SpectrumControl;
    m_TimerID=startTimer(50);
}

CSpectrumForm::~CSpectrumForm()
{
    killTimer(m_TimerID);
    m_TimerID=0;
    delete ui;
}

void CSpectrumForm::timerEvent(QTimerEvent* /*event*/)
{
    if (!m_TimerID) return;
    QMutexLocker locker(&mutex);
    if (isVisible()) Spectrum->update();
}
