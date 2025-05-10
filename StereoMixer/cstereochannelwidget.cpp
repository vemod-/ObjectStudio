#include "cstereochannelwidget.h"
#include "ui_cstereochannelwidget.h"

CStereoChannelWidget::CStereoChannelWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CStereoChannelWidget)
{
    ui->setupUi(this);
    m_Ch=NULL;
    connect(ui->ChannelEffects,SIGNAL(soloTriggered(bool)),this,SIGNAL(soloTriggered(bool)));
    connect(ui->ChannelVol,SIGNAL(volChanged(int)),this,SLOT(setVolume(int)));
}

CStereoChannelWidget::~CStereoChannelWidget()
{
    delete ui;
}

void CStereoChannelWidget::init(CStereoMixerChannel *ch, const QString &Name)
{
    m_Name=Name;
    setSender(QString());
    m_Ch=ch;
    ui->ChannelEffects->init(m_Ch);
    this->setMinimumHeight(424+(42*ch->sendCount));
}

void CStereoChannelWidget::checkPeak()
{
    ui->ChannelVol->peak(m_Ch->PeakL,m_Ch->PeakR);
    m_Ch->PeakL=0;
    m_Ch->PeakR=0;
}

void CStereoChannelWidget::resetPeak()
{
    ui->ChannelVol->resetPeak();
    m_Ch->PeakL=0;
    m_Ch->PeakR=0;
}

void CStereoChannelWidget::setVolume(int Vol)
{
    m_Ch->Level=Vol*0.01f;
}

void CStereoChannelWidget::soloButton(bool pressed)
{
    ui->ChannelEffects->setSolo(pressed);
}

void CStereoChannelWidget::muteButton(bool pressed)
{
    ui->ChannelEffects->setMute(pressed);
}

void CStereoChannelWidget::setSender(const QString& s)
{
    if (s.isEmpty())
    {
        ui->Name->setFont(QFont(QString(),13));
        ui->Name->setText(m_Name);
    }
    else
    {
        ui->Name->setFont(QFont(QString(),9));
        ui->Name->setText(s);
    }
}

void CStereoChannelWidget::serialize(QDomLiteElement* xml)
{
    ui->ChannelVol->serialize(xml);
    ui->ChannelEffects->serialize(xml);
}

void CStereoChannelWidget::unserialize(QDomLiteElement* xml)
{
    if (!xml) return;
    ui->ChannelVol->unserialize(xml);
    ui->ChannelEffects->unserialize(xml);
    checkPeak();
}

