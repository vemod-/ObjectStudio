#include "csf2channelwidget.h"
#include "ui_csf2channelwidget.h"

CSF2ChannelWidget::CSF2ChannelWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CSF2ChannelWidget)
{
    ui->setupUi(this);
    mapper=new QSignalMapper(this);
    preset=-1;
    bank=-1;
    m_Ch=NULL;
    ui->VolLabel->setText("0.00 dB");
    ui->Name->setEffect(EffectLabel::Raised);
    ui->Name->setShadowColor(Qt::white);
    ui->label_3->setEffect(EffectLabel::Raised);
    ui->label_3->setShadowColor(Qt::white);
    //ui->VolLabel->setEffect(EffectLabel::Raised);
    //ui->VolLabel->setShadowColor(Qt::darkGray);
    ui->Pan->setKnobStyle(QSynthKnob::SimpleStyle);
    connect(ui->SF2,SIGNAL(rightClick()),this,SLOT(loadDialog()));
    connect(ui->verticalSlider,SIGNAL(valueChanged(int)),this,SLOT(setVolume(int)));
    connect(ui->Pan,SIGNAL(valueChanged(int)),this,SLOT(setPan(int)));
    connect(ui->Bypass,SIGNAL(toggled(bool)),this,SLOT(setBypass(bool)));
    connect(ui->Mute,SIGNAL(toggled(bool)),this,SLOT(setMute(bool)));
    connect(ui->Solo,SIGNAL(toggled(bool)),this,SIGNAL(solo()));
    connect(mapper,SIGNAL(mapped(int)),this,SLOT(setEffect(int)));
    instrumentMenu=new QSignalMenu(this);
    showUIAction = instrumentMenu->addAction("Show UI","Show UI");
    connect(ui->SF2,SIGNAL(leftClick()),showUIAction,SIGNAL(triggered()));
    instrumentMenu->addAction("VST","VSTHost");
    instrumentMenu->addAction("AU","AudioUnitHost");
    instrumentMenu->addAction("SF2","SF2Player");
    connect(instrumentMenu,SIGNAL(menuClicked(QString)),this,SLOT(selectInstrument(QString)));
    bankMenu=new QSignalMenu(this);
    connect(ui->Patch,SIGNAL(leftClick()),this,SLOT(showBankMenu()));
    togglePatchChangeAction=bankMenu->addAction("Change Patch");
    togglePatchChangeAction->setCheckable(true);
    connect(togglePatchChangeAction,SIGNAL(triggered()),this,SLOT(togglePatchChange()));
}

CSF2ChannelWidget::~CSF2ChannelWidget()
{
    delete ui;
}

void CSF2ChannelWidget::Init(CStereoMixerChannel *ch, CDeviceContainer *SF2, short MIDIChannel, QString Name)
{
    ui->Name->setText(Name);
    m_Ch=ch;
    m_Instrument=SF2;
    m_MIDIChannel=MIDIChannel;
    for (int i=Effect.count();i<ch->sendCount;i++)
    {
        QSynthKnob* d=new QSynthKnob(this);
        Effect.append(d);
        d->setMaximumSize(28,28);
        d->setMaximum(100);
        d->setValue(100);
        d->setNotchesVisible(true);
        d->setKnobStyle(QSynthKnob::SimpleStyle);
        EffectLabel* l=new EffectLabel(this);
        l->setEffect(EffectLabel::Raised);
        l->setShadowColor(Qt::white);
        l->setMaximumHeight(13);
        QFont f=l->font();
        f.setPointSize(8);
        l->setFont(f);
        l->setText("AUX "+QString::number(i+1));
        //l->setStyleSheet("background:transparent;");
        QHBoxLayout* hlo=new QHBoxLayout();
        hlo->setMargin(0);
        hlo->setSpacing(0);
        ui->EffectLayout->addLayout(hlo);
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


void CSF2ChannelWidget::checkPeak()
{
    ui->PeakLeft->SetValue(m_Ch->PeakL);
    ui->PeakRight->SetValue(m_Ch->PeakR);
    m_Ch->PeakL=0;
    m_Ch->PeakR=0;
}

void CSF2ChannelWidget::resetPeak()
{
    ui->PeakLeft->Reset();
    ui->PeakRight->Reset();
    m_Ch->PeakL=0;
    m_Ch->PeakR=0;
}

void CSF2ChannelWidget::checkPreset()
{
    QString s=m_Instrument->CurrentPresetName(m_MIDIChannel);
    if (s != ui->Patch->text())
    {
        ui->Patch->setText(s);
    }
    s=QFileInfo(m_Instrument->FileName()).completeBaseName();
    if (s != ui->SF2->text())
    {
        ui->SF2->setText(s);
        buildPresetMenu();
    }
}

void CSF2ChannelWidget::setVolume(int Vol)
{
    m_Ch->Level=(float)Vol*0.01;
    ui->VolLabel->setText(QString::number(lin2db((float)Vol*0.01),'f',2)+" dB");
}

void CSF2ChannelWidget::setPan(int Pan)
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

void CSF2ChannelWidget::setMute(bool Mute)
{
    m_Ch->Mute=Mute;
}

void CSF2ChannelWidget::setBypass(bool Bypass)
{
    m_Ch->EffectMute=Bypass;
}

void CSF2ChannelWidget::setEffect(int effNumber)
{
    m_Ch->Effect[effNumber]=Effect[effNumber]->value()*0.01;
}

void CSF2ChannelWidget::loadSF(QString filename)
{
    if (m_Instrument->FileName().isEmpty())
    {
        m_Instrument->Load("<Custom File=\""+filename+"\" DeviceType=\"SF2Player\"/>");
        m_Instrument->SetParameterValue("Patch Change",1);
    }
    if (m_MIDIChannel>-1) m_Instrument->SetParameterValue("MIDI Channel",m_MIDIChannel+1);
    ui->SF2->setText(QFileInfo(m_Instrument->FileName()).completeBaseName());
    buildPresetMenu();
    checkPreset();
}

void CSF2ChannelWidget::loadDialog()
{
    showUIAction->setVisible(!m_Instrument->FileName().isEmpty());
    instrumentMenu->popup(ui->frame_2->mapToGlobal(ui->SF2->geometry().bottomLeft()));
}

void CSF2ChannelWidget::selectInstrument(QString instrument)
{
    if (instrument=="Show UI")
    {
        m_Instrument->Execute(true);
        m_Instrument->RaiseForm();
        return;
    }
    if (m_Instrument->OpenFile(instrument)==instrument)
    {
        m_Instrument->OpenFile(QString());
        m_Instrument->OpenFile(instrument);
    }
    m_Instrument->Execute(true);
}

void CSF2ChannelWidget::soloButton(bool pressed)
{
    ui->Solo->blockSignals(true);
    ui->Solo->setChecked(pressed);
    ui->Solo->blockSignals(false);
}

const QString CSF2ChannelWidget::Save()
{
    QDomLiteElement Channel("Channel");
    Channel.setAttribute("Volume",ui->verticalSlider->value());
    Channel.setAttribute("Pan",ui->Pan->value());
    Channel.setAttribute("Mute",ui->Mute->isChecked());
    Channel.setAttribute("Bypass",ui->Bypass->isChecked());
    Channel.setAttribute("Solo",ui->Solo->isChecked());
    for (int i=0;i<Effect.count();i++) Channel.setAttribute("Effect"+QString::number(i+1),Effect[i]->value());
    Channel.appendChildFromString(m_Instrument->Save());
    return Channel.toString();
}

void CSF2ChannelWidget::Load(const QString& XML)
{
    QDomLiteElement Channel("Channel");
    Channel.fromString(XML);
    ui->verticalSlider->setValue(Channel.attributeValue("Volume"));
    ui->Pan->setValue(Channel.attributeValue("Pan"));
    ui->Mute->setChecked(Channel.attributeValue("Mute"));
    ui->Bypass->setChecked(Channel.attributeValue("Bypass"));
    ui->Solo->setChecked(Channel.attributeValue("Solo"));
    QDomLiteElement* c=Channel.firstChild();
    if (c)
    {
        QString s=c->attribute("DeviceType");
        if (s.isEmpty()) c->setAttribute("DeviceType","SF2Player");
        m_Instrument->Load(c->toString());
    }
    ui->SF2->setText(QFileInfo(m_Instrument->FileName()).completeBaseName());
    buildPresetMenu();
    for (int i=0;i<Effect.count();i++)
    {
        Effect[i]->setValue(Channel.attributeValue("Effect"+QString::number(i+1),100));
    }
    checkPeak();
    checkPreset();
}

void CSF2ChannelWidget::showEvent(QShowEvent *)
{
    ui->frame->setMargin(ui->verticalSlider->grooveMargin());
    ui->PeakLeft->setMargin(ui->verticalSlider->grooveMargin());
    ui->PeakRight->setMargin(ui->verticalSlider->grooveMargin());
}

void CSF2ChannelWidget::buildPresetMenu()
{
    foreach (QSignalMenu* m,presetMenus) m->clear();
    bankMenu->clear();
    qDeleteAll(presetMenus);
    presetMenus.clear();
    togglePatchChangeAction=bankMenu->addAction("Change Patch");
    togglePatchChangeAction->setCheckable(true);
    connect(togglePatchChangeAction,SIGNAL(triggered()),this,SLOT(togglePatchChange()));
    QStringList l=m_Instrument->Banks();
    if (l.count() > 1)
    {
        foreach (QString s,l)
        {
            QSignalMenu* m=new QSignalMenu("Bank "+s,this);
            QStringList pl=m_Instrument->Presets(s.toInt());
            foreach (QString ps,pl)
            {
                m->addAction(ps,(((unsigned int)s.toInt()) * 0xFFF)+ps.left(3).toInt());
            }
            bankMenu->addMenu(m);
            presetMenus.append(m);
            connect(m,SIGNAL(menuClicked(int)),this,SLOT(selectProgram(int)));
        }
    }
    else
    {
        QStringList pl=m_Instrument->Presets();
        if (pl.count() > 1)
        {
            QSignalMenu* m=new QSignalMenu("Presets",this);
            for (int i=0;i<pl.count();i++)
            {
                m->addAction(pl[i],i);
            }
            bankMenu->addMenu(m);
            presetMenus.append(m);
            connect(m,SIGNAL(menuClicked(int)),this,SLOT(selectProgram(int)));
        }
    }
}

void CSF2ChannelWidget::showBankMenu()
{
    togglePatchChangeAction->setChecked(m_Instrument->GetParameterValue("Patch Change"));
    int menuIndex=(m_Instrument->CurrentBank(m_MIDIChannel)*0xFFF)+m_Instrument->CurrentPreset(m_MIDIChannel);
    qDebug() << m_Instrument->CurrentBank(m_MIDIChannel) << m_Instrument->CurrentPreset(m_MIDIChannel) << menuIndex;
    foreach(QSignalMenu* m,presetMenus) m->checkAction(menuIndex);
    bankMenu->popup(ui->frame_2->mapToGlobal(ui->Patch->geometry().bottomLeft()));
}

void CSF2ChannelWidget::selectProgram(int program)
{
    int Bnk=program/0xFFF;
    int Prg=program & 0xFFF;
    m_Instrument->SetParameterValue("Patch Change",0);
    m_Instrument->SetCurrentPreset(Bnk,Prg);
}

void CSF2ChannelWidget::togglePatchChange()
{
    m_Instrument->SetParameterValue("Patch Change",!(m_Instrument->GetParameterValue("Patch Change")));
}
