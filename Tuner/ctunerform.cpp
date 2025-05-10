#include "ctunerform.h"
#include "ui_ctunerform.h"

CTunerForm::CTunerForm(IDevice* Device, QWidget *parent) :
    CSoftSynthsForm(Device,true,parent),
    PD(presets.SampleRate),
    ui(new Ui::CTunerForm)
{
    ui->setupUi(this);
    m_TimerID = startTimer(100);
}

CTunerForm::~CTunerForm()
{
    if (m_TimerID) killTimer(m_TimerID);
    m_TimerID = 0;
    delete ui;
}

void CTunerForm::timerEvent(QTimerEvent* /*e*/)
{
    if (!m_TimerID) return;
    QMutexLocker locker(&mutex);
    if (isVisible()) ui->TunerWidget->setTune(PD.CurrentPitchRecord(),PD.tune());
}

