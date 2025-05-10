#include "csf2channelwidget.h"
#include "ui_csf2channelwidget.h"
#include <QMainWindow>

CSF2ChannelWidget::CSF2ChannelWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CSF2ChannelWidget)
{
    ui->setupUi(this);
    preset=-1;
    bank=-1;
    m_Ch=nullptr;
    m_Instrument=nullptr;
    ui->Name->setEffect(EffectLabel::Raised);
    ui->Name->setShadowColor(QColor(255,255,255,200));
    ui->Name->setTextColor(QColor(0,0,0,200));
    connect(ui->SF2,&QLCDLabel::rightClicked,this,&CSF2ChannelWidget::loadDialog);
    connect(ui->ChannelEffects,&CChannelEffects::soloTriggered,this,&CSF2ChannelWidget::soloTriggered);
    connect(ui->ChannelVol,&CChannelVol::volChanged,this,&CSF2ChannelWidget::setVolume);
    instrumentMenu=new QSignalMenu(this);
    showUIAction = instrumentMenu->addAction("Show UI","Show UI");
    connect(ui->SF2,&QLCDLabel::leftClicked,showUIAction,&QAction::trigger);
    for (const QString& s:CDeviceContainer::instrumentList()) instrumentMenu->addAction(s,s);
    connect(instrumentMenu,qOverload<QString>(&QSignalMenu::menuClicked),this,&CSF2ChannelWidget::selectInstrument);
    bankMenu=new QSignalMenu(this);
    connect(ui->Patch,&QLCDLabel::leftClicked,this,&CSF2ChannelWidget::showBankMenu);
    volSlider = ui->ChannelVol;
    effectsPanel = ui->ChannelEffects;
    connect(ui->RackLabel,&QLCDLabel::leftClicked,this,&CSF2ChannelWidget::toggleEffectRack);
}

CSF2ChannelWidget::~CSF2ChannelWidget()
{
    delete ui;
}


void CSF2ChannelWidget::init(CStereoMixerChannel *ch, const QString& Name, CDeviceContainer *SF2, short MIDIChannel)
{
    m_Ch = ch;
    ui->ChannelEffects->init(m_Ch);
    ui->ChannelEQ->init(m_Ch);
    ui->ChannelGain->init(m_Ch);
    setMinimumHeight(524+(42*int(ch->sendCount)));
    if (!SF2)
    {
        m_Name=Name;
        m_Instrument = nullptr;
        setSender(QString());
        ui->SF2Frame->setVisible(false);
        ui->NameFrame->setVisible(false);
        ui->Sender->setVisible(true);
    }
    else
    {
        ui->Name->setText(Name);
        m_Instrument = SF2;
        m_MIDIChannel=MIDIChannel;
        ui->NameFrame->setVisible(true);
        ui->Sender->setVisible(false);
    }
}

void CSF2ChannelWidget::setVolume(int Vol)
{
    m_Ch->Level=Vol*0.01f;
}

void CSF2ChannelWidget::load(const QString& filename)
{
    if (m_Instrument->deviceType().isEmpty())
    {
        const QDomLiteElement fileElement=QDomLite::elementFromString(R"(<Custom File=")"+filename+R"(" DeviceType="SF2Player"/>)");
        m_Instrument->unserializeCustom(&fileElement);
        m_Instrument->setParameterValue("Patch Change",1);
    }
    if (m_MIDIChannel>-1) m_Instrument->setParameterValue("MIDI Channel",m_MIDIChannel+1);
    ui->SF2->setText(m_Instrument->caption());
    buildPresetMenu();
    checkPreset();
}

void CSF2ChannelWidget::checkPreset()
{
    QString s=m_Instrument->currentBankPresetName(m_MIDIChannel);
    if (s != ui->Patch->text())
    {
        ui->Patch->setText(s);
    }
    s=m_Instrument->caption();
    if (s != ui->SF2->text())
    {
        ui->SF2->setText(s);
        buildPresetMenu();
    }
}

void CSF2ChannelWidget::checkEffects() {
    int effCount = ui->RackLabel->text().split(" ").first().toInt();
    if (m_Ch->EffectRack.childDeviceCount() != effCount) {
        ui->RackLabel->setText(QString::number(m_Ch->EffectRack.childDeviceCount()) + " Effects");
    }
}

void CSF2ChannelWidget::setSender(const QString& s)
{
    if (s.isEmpty())
    {
        setFontSizeScr(ui->Sender,13);
        ui->Sender->setText(m_Name);
    }
    else
    {
        setFontSizeScr(ui->Sender,9);
        ui->Sender->setText(s);
    }
}

void CSF2ChannelWidget::checkPeak()
{
    if (m_Ch) {
        ui->ChannelVol->peak(m_Ch->PeakL,m_Ch->PeakR);
        m_Ch->PeakL=0;
        m_Ch->PeakR=0;
    }
}

void CSF2ChannelWidget::checkAll()
{
    if (m_Instrument) checkPreset();
    if (m_Ch) checkEffects();
    checkPeak();
}

void CSF2ChannelWidget::resetPeak()
{
    ui->ChannelVol->resetPeak();
    m_Ch->PeakL=0;
    m_Ch->PeakR=0;
}

void CSF2ChannelWidget::loadDialog()
{
    showUIAction->setVisible(!m_Instrument->deviceType().isEmpty());
    instrumentMenu->popup(ui->SF2Frame->mapToGlobal(ui->SF2->geometry().bottomLeft()));
}

void CSF2ChannelWidget::selectInstrument(QString instrument)
{
    if (instrument=="Show UI")
    {
        m_Instrument->execute(true);
        m_Instrument->raiseForm();
        return;
    }
    if (m_Instrument->deviceType()!=instrument)
    {
        m_Instrument->setDeviceType(instrument);
    }
    m_Instrument->execute(true);
}

void CSF2ChannelWidget::soloButton(bool pressed)
{
    ui->ChannelEffects->setSolo(pressed);
}

void CSF2ChannelWidget::muteButton(bool pressed)
{
    ui->ChannelEffects->setMute(pressed);
}

void CSF2ChannelWidget::serialize(QDomLiteElement* xml) const
{
    xml->setAttribute("ID",ID);
    ui->ChannelVol->serialize(xml);
    ui->ChannelEffects->serialize(xml);
    ui->ChannelEQ->serialize(xml);
    ui->ChannelGain->serialize(xml);
    if (m_Ch) m_Ch->EffectRack.serializeDevice(xml->appendChild("EffectRack"));
    //qDebug() << "ChannelWidget Serialize" << m_Instrument;
    if (m_Instrument) m_Instrument->serializeCustom(xml->appendChild("Custom"));
}

void CSF2ChannelWidget::unserialize(const QDomLiteElement* xml)
{
    if (!xml) return;
    ID = xml->attribute("ID");
    ui->ChannelVol->unserialize(xml);
    ui->ChannelEffects->unserialize(xml);
    ui->ChannelEQ->unserialize(xml);
    ui->ChannelGain->unserialize(xml);
    if (m_Ch) {
        if (const QDomLiteElement* eff = xml->elementByTag("EffectRack")) {
            m_Ch->EffectRack.unserializeDevice(eff);
            m_Ch->EffectRack.raiseForm();
        }
    }
    if (m_Instrument)
    {
        if (QDomLiteElement* c=xml->elementByTag("Custom"))
        {
            if (c->attribute("DeviceType").isEmpty()) c->setAttribute("DeviceType","SF2Player");
            m_Instrument->unserializeCustom(c);
        }
        ui->SF2->setText(m_Instrument->caption());
        buildPresetMenu();
    }
    checkPeak();
    if (m_Instrument) checkPreset();
    if (m_Ch) checkEffects();
}

void CSF2ChannelWidget::buildPresetMenu()
{
    for(QSignalMenu* m : std::as_const(presetMenus)) m->clear();
    bankMenu->clear();
    qDeleteAll(presetMenus);
    presetMenus.clear();
    togglePatchChangeAction=bankMenu->addAction("Change Patch");
    togglePatchChangeAction->setCheckable(true);
    connect(togglePatchChangeAction,&QAction::triggered,this,&CSF2ChannelWidget::togglePatchChange);
    transposeMenu=new QSignalMenu("Transpose",this);
    for (int i = 24; i >= -24; i--)
    {
        QAction* a = transposeMenu->addAction(QString::number(i),i);
        a->setCheckable(true);
    }
    bankMenu->addMenu(transposeMenu);
    connect(transposeMenu,qOverload<int>(&QSignalMenu::menuClicked),this,&CSF2ChannelWidget::setTranspose);
    QStringList l=m_Instrument->bankNames();
    if (l.size() > 1)
    {
        for (const QString& s : std::as_const(l))
        {
            int b=s.left(3).toInt();
            QSignalMenu* m=new QSignalMenu("Bank "+s,this);
            QStringList pl=m_Instrument->presetNames(b);
            for (const QString& ps : std::as_const(pl))
            {
                m->addAction(ps,m_Instrument->bankPresetNumber(b,ps.left(3).toInt()));
            }
            bankMenu->addMenu(m);
            presetMenus.append(m);
            connect(m,qOverload<int>(&QSignalMenu::menuClicked),this,&CSF2ChannelWidget::selectProgram);
        }
    }
    else
    {
        QStringList pl=m_Instrument->presetNames();
        if (pl.size() > 1)
        {
            QSignalMenu* m=new QSignalMenu("Presets",this);
            for (int i=0;i<pl.size();i++)
            {
                m->addAction(pl[i],i);
            }
            bankMenu->addMenu(m);
            presetMenus.append(m);
            connect(m,qOverload<int>(&QSignalMenu::menuClicked),this,&CSF2ChannelWidget::selectProgram);
        }
    }
}

void CSF2ChannelWidget::showBankMenu()
{
    togglePatchChangeAction->setChecked(m_Instrument->parameterValue("Patch Change"));
    transposeMenu->checkAction(m_Instrument->parameterValue("Transpose"));
    for(QSignalMenu* m : std::as_const(presetMenus)) m->checkAction(m_Instrument->currentBankPreset(m_MIDIChannel));
    bankMenu->popup(ui->SF2Frame->mapToGlobal(ui->Patch->geometry().bottomLeft()));
}

void CSF2ChannelWidget::selectProgram(int program)
{
    m_Instrument->setParameterValue("Patch Change",0);
    m_Instrument->setCurrentBankPreset(program);
}

void CSF2ChannelWidget::togglePatchChange()
{
    m_Instrument->setParameterValue("Patch Change",!(m_Instrument->parameterValue("Patch Change")));
}

void CSF2ChannelWidget::setTranspose(int transpose)
{
    m_Instrument->setParameterValue("Transpose",transpose);
}

void CSF2ChannelWidget::toggleEffectRack() {
    m_Ch->EffectRack.toggleUI();
}
