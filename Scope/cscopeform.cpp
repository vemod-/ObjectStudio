#include "cscopeform.h"
#include "ui_cscopeform.h"

CScopeForm::CScopeForm(IDevice* Device, QWidget *parent) :
    CSoftSynthsForm(Device,false,parent),
    ui(new Ui::CScopeForm)
{
    ui->setupUi(this);
    Scope=ui->ScopeControl;
    m_TimerID=startTimer(50);
}

CScopeForm::~CScopeForm()
{
    killTimer(m_TimerID);
    m_TimerID=0;
    delete ui;
}

void CScopeForm::timerEvent(QTimerEvent* /*event*/)
{
    if (!m_TimerID) return;
    QMutexLocker locker(&mutex);
    if (isVisible()) Scope->update();
}

