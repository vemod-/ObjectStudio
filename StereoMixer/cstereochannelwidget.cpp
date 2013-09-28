#include "cstereochannelwidget.h"
#include "ui_cstereochannelwidget.h"

CStereoChannelWidget::CStereoChannelWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CStereoChannelWidget)
{
    ui->setupUi(this);
    mapper=new QSignalMapper(this);
    m_Ch=NULL;
    ui->VolLabel->setText("0.00 dB");
    ui->label_3->setEffect(EffectLabel::Raised);
    ui->label_3->setShadowColor(Qt::white);
    ui->Name->setEffect(EffectLabel::Raised);
    ui->Name->setShadowColor(Qt::white);
    //ui->VolLabel->setEffect(EffectLabel::Raised);
    //ui->VolLabel->setShadowColor(Qt::darkGray);
    ui->Pan->setKnobStyle(QSynthKnob::SimpleStyle);
    connect(ui->verticalSlider,SIGNAL(valueChanged(int)),this,SLOT(setVolume(int)));
    connect(ui->Pan,SIGNAL(valueChanged(int)),this,SLOT(setPan(int)));
    connect(ui->Bypass,SIGNAL(toggled(bool)),this,SLOT(setBypass(bool)));
    connect(ui->Mute,SIGNAL(toggled(bool)),this,SLOT(setMute(bool)));
    connect(ui->Solo,SIGNAL(toggled(bool)),this,SIGNAL(solo()));
    connect(mapper,SIGNAL(mapped(int)),this,SLOT(setEffect(int)));
}

CStereoChannelWidget::~CStereoChannelWidget()
{
    delete ui;
}

void CStereoChannelWidget::Init(CStereoMixerChannel *ch, const QString &Name)
{
    ui->Name->setText(Name);
    m_Ch=ch;
    QVBoxLayout* lo=(QVBoxLayout*)ui->EffectLayout->layout();
    for (int i=Effect.count();i<ch->sendCount;i++)
    {
        QSynthKnob* d=new QSynthKnob(this);
        Effect.append(d);
        d->setMaximumSize(28,28);
        d->setKnobStyle(QSynthKnob::SimpleStyle);
        d->setMaximum(100);
        d->setValue(100);
        d->setNotchesVisible(true);
        EffectLabel* l=new EffectLabel(this);
        l->setEffect(EffectLabel::Raised);
        l->setShadowColor(Qt::white);
        l->setMaximumHeight(13);
        QFont f=l->font();
        f.setPointSize(8);
        l->setFont(f);
        l->setText("AUX "+QString::number(i+1));
        l->setStyleSheet("background:transparent;");
        QHBoxLayout* hlo=new QHBoxLayout();
        hlo->setMargin(0);
        hlo->setSpacing(0);
        lo->addLayout(hlo);
        if (i & 1)
        {
            hlo->addWidget(d,0,Qt::AlignHCenter);
            hlo->addWidget(l,0,Qt::AlignHCenter);
        }
        else
        {
            hlo->addWidget(l,0,Qt::AlignHCenter);
            hlo->addWidget(d,0,Qt::AlignHCenter);
        }
        connect(d,SIGNAL(valueChanged(int)),mapper,SLOT(map()));
        mapper->setMapping(d,i);
    }
    this->setMinimumHeight(424+(42*ch->sendCount));
    ui->verticalSlider->setValue(100);
    ui->Pan->setValue(100);
    ui->Mute->setChecked(false);
    ui->Solo->setChecked(false);
    ui->Bypass->setChecked(false);
    foreach (QDial* d,Effect) d->setValue(100);
}

void CStereoChannelWidget::checkPeak()
{
    ui->PeakLeft->SetValue(m_Ch->PeakL);
    ui->PeakRight->SetValue(m_Ch->PeakR);
    m_Ch->PeakL=0;
    m_Ch->PeakR=0;
}

void CStereoChannelWidget::resetPeak()
{
    ui->PeakLeft->Reset();
    ui->PeakRight->Reset();
    m_Ch->PeakL=0;
    m_Ch->PeakR=0;
}

void CStereoChannelWidget::setVolume(int Vol)
{
    m_Ch->Level=(float)Vol*0.01;
    ui->VolLabel->setText(QString::number(lin2db((float)Vol*0.01),'f',2)+" dB");
}

void CStereoChannelWidget::setPan(int Pan)
{
    if (Pan<=100)
    {
        m_Ch->PanL=1;
        m_Ch->PanR=Pan*0.01;
    }
    else
    {
        m_Ch->PanR=1;
        m_Ch->PanL=(100-(Pan-100))*0.01;
    }
}

void CStereoChannelWidget::setMute(bool Mute)
{
    m_Ch->Mute=Mute;
}

void CStereoChannelWidget::setBypass(bool Bypass)
{
    m_Ch->EffectMute=Bypass;
}

void CStereoChannelWidget::setEffect(int effNumber)
{
    m_Ch->Effect[effNumber]=Effect[effNumber]->value()*0.01;
}

void CStereoChannelWidget::soloButton(bool pressed)
{
    ui->Solo->blockSignals(true);
    ui->Solo->setChecked(pressed);
    ui->Solo->blockSignals(false);
}

const QString CStereoChannelWidget::Save()
{
    QDomLiteElement Channel("Channel");
    Channel.setAttribute("Volume",ui->verticalSlider->value());
    Channel.setAttribute("Pan",ui->Pan->value());
    Channel.setAttribute("Mute",ui->Mute->isChecked());
    Channel.setAttribute("Bypass",ui->Bypass->isChecked());
    Channel.setAttribute("Solo",ui->Solo->isChecked());
    for (int i=0;i<Effect.count();i++) Channel.setAttribute("Effect"+QString::number(i+1),Effect[i]->value());
    //Channel.appendChildFromString(m_SF2->Save());
    return Channel.toString();
}

void CStereoChannelWidget::Load(const QString& XML)
{
    QDomLiteElement Channel("Channel");
    Channel.fromString(XML);
    ui->verticalSlider->setValue(Channel.attributeValue("Volume"));
    ui->Pan->setValue(Channel.attributeValue("Pan"));
    ui->Mute->setChecked(Channel.attributeValue("Mute"));
    ui->Bypass->setChecked(Channel.attributeValue("Bypass"));
    ui->Solo->setChecked(Channel.attributeValue("Solo"));
    //m_SF2->Load(Channel.firstChild()->toString());
    //ui->SF2->setText(QFileInfo(m_SF2->FileName()).completeBaseName());
    for (int i=0;i<Effect.count();i++) Effect[i]->setValue(Channel.attributeValue("Effect"+QString::number(i+1),100));
    checkPeak();
}

void CStereoChannelWidget::showEvent(QShowEvent *)
{
    ui->frame->setMargin(ui->verticalSlider->grooveMargin());
    ui->PeakLeft->setMargin(ui->verticalSlider->grooveMargin());
    ui->PeakRight->setMargin(ui->verticalSlider->grooveMargin());
}
