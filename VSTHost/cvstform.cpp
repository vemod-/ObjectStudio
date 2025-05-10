#include "cvstform.h"
#include "ui_cvstform.h"
#include <QVBoxLayout>
#include "idevice.h"

CVSTForm::CVSTForm(IAudioPlugInHost* plug, IDevice *Device, QWidget *parent) :
    CSoftSynthsForm(Device,true,parent),
    ui(new Ui::CVSTForm)
{
    setAttribute(Qt::WA_MacShowFocusRect,false);
    ui->setupUi(this);
    auto l=new QVBoxLayout(this);
    l->setSpacing(0);
    l->setContentsMargins(0,0,0,0);
    l->setSizeConstraint(QVBoxLayout::SetFixedSize);

    listHolder=new QWidget(this);
    l->addWidget(listHolder);

    auto bl=new QVBoxLayout(listHolder);
    bl->setSpacing(0);
    bl->setContentsMargins(0,0,0,0);

    li=new QComboBox;
    li->setAttribute(Qt::WA_MacShowFocusRect,false);

    bl->addWidget(li);

    listHolder->hide();

    connect(li,SIGNAL(currentIndexChanged(int)),this,SLOT(ChangeBankPreset(int)));

    plugIn=plug;
    connect(plugIn,SIGNAL(PlugInChanged()),this,SLOT(PlugInIndexChanged()));
    l->addWidget(plugIn);
    m_TimerID=startTimer(200);
    updateTimer.setSingleShot(true);
    updateTimer.setInterval(1000);
    connect(&updateTimer,&QTimer::timeout,this,&CVSTForm::updateHost);
    updateHost();
    updateTimer.start();
}

CVSTForm::~CVSTForm()
{
    qDebug() << "~VSTForm";
    killTimer(m_TimerID);
    m_TimerID=0;
    delete plugIn;
    delete ui;
    qDebug() << "Exit VSTForm";
}

void CVSTForm::fillList(int CurrentProgram)
{
    qDebug() << "vstform filllist";
    li->blockSignals(true);
    li->clear();
    li->addItems(plugIn->bankPresetNames());
    if (CurrentProgram > -1)
    {
        li->setCurrentIndex(CurrentProgram);
    }
    li->blockSignals(false);
    listHolder->setVisible(li->count() > 1);
    updateHost();
}

void CVSTForm::PlugInIndexChanged()
{
    qDebug() << "vstform plugin index changed";
    fillList();
    setVisible(true);
    updateHost();
    updateTimer.start();
}

void CVSTForm::ChangeBankPreset(int programIndex)
{
    qDebug() << "vstform change bankpreset";
    plugIn->setBankPreset(programIndex);
    updateHost();
}

void CVSTForm::setBankPreset(const int programIndex)
{
    qDebug() << "vstform set bankpreset";
    ChangeBankPreset(programIndex);
    if (li->count() > programIndex)
    {
        li->blockSignals(true);
        li->setCurrentIndex(programIndex);
        li->blockSignals(false);
        updateHost();
    }
}

bool CVSTForm::event(QEvent *event)
{
    //qDebug() << "CVSTform event" << event->type();
    bool ret = CSoftSynthsForm::event(event);
    if (event->type()==QEvent::NonClientAreaMouseButtonPress)
    {
        qDebug() << "CVSTform event" << event->type() << dynamic_cast<QMouseEvent*>(event)->button();
        if (dynamic_cast<QMouseEvent*>(event)->button()==Qt::RightButton)
        {
            plugIn->popup(mapToGlobal(dynamic_cast<QMouseEvent*>(event)->pos()));
        }
    }
    /*
    else if (event->type()==QEvent::Resize) {
        qDebug() << "CVSTform event 1";
        //if (isVisible()) plugIn->repaint();
    }
    else if (event->type()==QEvent::Show) {
        qDebug() << "CVSTform event 2";
        //plugIn->repaint();
    }
*/
    return ret;
}

void CVSTForm::timerEvent(QTimerEvent */*event*/)
{
    if (!m_TimerID) return;
    //qDebug() << "vstform timer";
    if (li->count())
    {
        const int p=plugIn->currentBankPreset();
        if (p != li->currentIndex())
        {
            li->blockSignals(true);
            li->setCurrentIndex(p);
            li->blockSignals(false);
            updateHost();
        }
    }
}
/*
void CVSTForm::updateHost()
{
    //qDebug() << "CVSTform updateHost 1";
    //adjustSize();
    //updateGeometry();
    //qDebug() << "CVSTform updateHost 2";
    //plugIn->repaint();
    qDebug() << "CVSTform updateHost 3";
    CSoftSynthsForm::updateHost();
    //qDebug() << "CVSTform updateHost 4";
}
*/
