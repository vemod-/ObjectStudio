#include "cmidifile2wave.h"
//#include <QDesktopWidget>
//#include <QFileInfo>
#include <QApplication>
#include <QScreen>

#define devicename "MIDIFile2Wave"

CMIDI2WavForm::CMIDI2WavForm(IDevice* Device,QWidget* Parent) :
    CSoftSynthsForm(Device,true,Parent)
{
    MW=new CMixerWidget(this);
    Map=new CUIMap(this);
    Map->setVisible(false);
    auto ly=new QHBoxLayout(this);
    ly->setContentsMargins(0,0,0,0);
    ly->setSpacing(0);
    ly->addWidget(MW);
    ly->addWidget(Map);

    UIMenu=new QMenu("Parameters",this);
    UIMenu->addAction("UI map",this,SLOT(showMap()));
    UIMenu->addAction("Hide UIs",this,SLOT(hideUIs()));
    UIMenu->addAction("Cascade UIs",this,SLOT(cascadeUIs()));

    connect(Map,&CUIMap::deviceSelected,this,&CMIDI2WavForm::hideMap);
}



void CMIDI2WavForm::showMap()
{
    Map->showMap(MIDIFILE2WAVECLASS->deviceList(),this,MW);
}

void CMIDI2WavForm::hideUIs()
{
    MIDIFILE2WAVECLASS->deviceList()->hideForms();
}

void CMIDI2WavForm::cascadeUIs()
{
    QPoint p(24,24);
    MIDIFILE2WAVECLASS->deviceList()->cascadeForms(p);
}

bool CMIDI2WavForm::event(QEvent *event)
{
    if (event->type()==QEvent::NonClientAreaMouseButtonPress)
    {
        if (dynamic_cast<QMouseEvent*>(event)->button()==Qt::RightButton)
        {
            UIMenu->popup(dynamic_cast<QMouseEvent*>(event)->globalPosition().toPoint());
        }
    }
    return CSoftSynthsForm::event(event);
}

void CMIDI2WavForm::hideMap()
{
    Map->setVisible(false);
    MW->setVisible(true);
}

CMIDIFile2Wave::CMIDIFile2Wave()
{
    Q_INIT_RESOURCE(midifile2waveresources);
    Mx=nullptr;
    HideEmptyChannels=true;
    mixerWidget=nullptr;
}

void CMIDIFile2Wave::updateDeviceParameter(const CParameter* /*p*/)
{
    MFR.setTempoAdjust(m_Parameters[pnTempoAdjust]->PercentValue);
    for (CMIDIFilePlayer* p: std::as_const(MIDIFilePlayers)) {
        p->parameter(CMIDIFilePlayer::pnTempoAdjust)->setValue(m_Parameters[pnTempoAdjust]->Value);
        p->parameter(CMIDIFilePlayer::pnHumanize)->setValue(m_Parameters[pnHumanize]->Value);
    }
    for (CDeviceContainer* d: std::as_const(Instruments)) d->setParameterValue(m_Parameters[pnTune]->vars.Name,m_Parameters[pnTune]->Value);
}

void CMIDIFile2Wave::play(const bool FromStart)
{
    if (FromStart) mixerWidget->resetPeak();
    //DeviceList.play(FromStart);
    if (FromStart) updateDeviceParameter();
    IDevice::play(FromStart);
}
/*
void CMIDIFile2Wave::pause()
{
    //DeviceList.pause();
    IDevice::pause();
}
*/
/*
void CMIDIFile2Wave::tick()
{
    DeviceList.tick();
}
*/
CAudioBuffer* CMIDIFile2Wave::getNextA(const int ProcIndex)
{
    return (Mx) ? Mx->getNextA(ProcIndex+CStereoMixer::stereoout) : nullptr;//&m_NullBufferStereo;
}

bool CMIDIFile2Wave::loadFile(const QString& filename)
{
    try
    {
        QFile f(filename);
        if (f.open(QIODevice::ReadOnly))
        {
            assign(f.readAll());
            f.close();
            return true;
        }
    }
    catch (...)
    {
    }
    return false;
}

void CMIDIFile2Wave::initWithFile(const QString& path) {
    openFile(path);
}

bool CMIDIFile2Wave::refreshMIDIFile(const QString& filename) {
    const int c = MFR.channelCount();
    const int t = MFR.trackCount();
    const int ft = MFR.fileType();
    QFile f(filename);
    if (f.open(QIODevice::ReadOnly))
    {
        QByteArray b = f.readAll();

        if (MFR.assign(b)) {
            if (MFR.channelCount() == c) {
                if (MFR.trackCount() == t) {
                    if (MFR.fileType() == ft) {
                        for (int i=0;i<MIDIFilePlayers.size();i++)
                        {
                            CMIDIFilePlayer* MFP=MIDIFilePlayers[i];
                            MFP->assign(b,filename);
                            MFP->parameter(CMIDIFilePlayer::pnTrack)->setValue(0);
                            if (MFR.fileType() != 0) MFP->parameter(CMIDIFilePlayer::pnTrack)->setValue(i+1);
                        }
                        f.close();
                        return true;
                    }
                }
            }
        }
        f.close();
    }
    return false;
}

void CMIDIFile2Wave::assign(const QByteArray& b)
{
    DeviceList.setHost(m_Host);
    if (Mx) Mx->setDisabled(true);
    mixerWidget->stop();
    mutex.lock();

    for (CMIDIFilePlayer* p : std::as_const(MIDIFilePlayers)) DeviceList.disconnectDevice(p);
    MFR.assign(b);
    int MFPCount = 1;
    int channelcount = 16;
    if (MFR.fileType() == 0)
    {
        MFPCount = channelcount;
    }
    else
    {
        int count=0;
        if (HideEmptyChannels)
        {
            for (int i = MFR.trackCount() - 1; i >= 0; i--)
            {
                count = i + 1;
                if (MFR.noteCount(i) != 0) break;
            }
        }
        else
        {
            count = MFR.trackCount();
        }
        MFPCount = count;
        channelcount = count;
    }
    while (MIDIFilePlayers.size() > MFPCount) DeviceList.deleteDevice(MIDIFilePlayers.takeLast());
    while (MIDIFilePlayers.size() < MFPCount) MIDIFilePlayers.append(dynamic_cast<CMIDIFilePlayer*>(DeviceList.addDevice(new CMIDIFilePlayer,MIDIFilePlayers.size()+1,m_MainWindow)));
    for (int i = 0; i < MFPCount; i++)
    {
        CMIDIFilePlayer* MFP = MIDIFilePlayers[i];
        MFP->assign(b,filename());
        MFP->parameter(CMIDIFilePlayer::pnTrack)->setValue(0);
        if (MFR.fileType() != 0) MFP->parameter(CMIDIFilePlayer::pnTrack)->setValue(i + 1);
    }
    QVector<bool> channelVisible;
    channelVisible.assign(channelcount,true);
    if (HideEmptyChannels) {
        for (int i = 0; i < channelcount; i++) {
            if (MFR.fileType() == 0) {
                if ((i < MFR.minChannel()) || (i >= MFR.channelCount())) channelVisible[i] = false;
            }
            else {
                if (MFR.noteCount(i) == 0) channelVisible[i] = false;
            }
        }
    }
    if (!Mx) {
        Mx = new CStereoMixer(0,MIDIFile2Wave::effectCount);
        Mx->setDisabled(true);
        DeviceList.addDevice(Mx,1,nullptr);
        Mx->addEffectRacksToDeviceList(&DeviceList,m_MainWindow);
    }
    m_Form->setUpdatesEnabled(false);
    if (!m_Form->isVisible()) {
        mixerWidget->hide();
        mixerWidget->hideMaster();
    }

    const bool channelMatch = int(Mx->channelCount()) == channelcount;
    bool IDMatch = channelMatch;
    if (mixerWidget->channels.size() != IDList.size()) IDMatch = false;
    if (IDMatch) {
        for (int i = 0; i < mixerWidget->channels.size(); i++) {
            if (mixerWidget->channels[i]->ID != IDList[i]) {
                IDMatch = false;
                break;
            }
        }
    }
    if (!IDMatch) {
        QDomLiteElement channelXML;
        for (int i = 0; i < mixerWidget->channels.size(); i++) {
            QDomLiteElement* e = new QDomLiteElement;
            mixerWidget->channels[i]->serialize(e);
            e->tag = mixerWidget->channels[i]->ID;
            channelXML.appendChild(e);
        }
        if (!channelMatch) {
            DeviceList.disconnectAll();
            Mx->removerEffectRacksFromDeviceList(&DeviceList);
            DeviceList.deleteDevice(Mx);
            Mx=new CStereoMixer(channelcount,MIDIFile2Wave::effectCount);
            Mx->setDisabled(true);
            DeviceList.addDevice(Mx,1,nullptr);
            Mx->addEffectRacksToDeviceList(&DeviceList,m_MainWindow);
            while (mixerWidget->channels.size() > channelcount) {
                DeviceList.deleteDevice(Instruments.takeAt(mixerWidget->channels.size() - 1));
                mixerWidget->removeChannel();
            }
            if (mixerWidget->channels.size() < channelcount) {
                while (mixerWidget->channels.size() > 0) {
                    DeviceList.deleteDevice(Instruments.takeAt(mixerWidget->channels.size() - 1));
                    mixerWidget->removeChannel();
                }
                while (mixerWidget->channels.size() < channelcount) {
                    Instruments.append(dynamic_cast<CDeviceContainer*>(DeviceList.addDevice(new CDeviceContainer("Instrument"),mixerWidget->channels.size() + 1,m_MainWindow)));
                    mixerWidget->appendChannel();
                }
            }
        }
        qDebug() << IDList << channelXML.childTags();
        for (int i = 0; i < channelcount; i++) {
            CSF2ChannelWidget* ch = mixerWidget->channels[i];
            const QString channelNumber = QString::number(i + 1);
            QString n = (MFR.fileType() == 0) ? "Channel " + channelNumber : "Track " + channelNumber;
            if (i < IDList.size()) n = IDList[i];
            if (MFR.fileType() == 0) {
                ch->init(Mx->channels[i], n, Instruments[i], i);
            }
            else {
                ch->init(Mx->channels[i], n, Instruments[i], -1);
            }
            if (i < IDList.size()) {
                if (QDomLiteElement* e = channelXML.elementByTag(IDList[i])) {
                    ch->unserialize(e);
                    if (e->attributeValueBool("Solo")) Mx->SoloChannel = i;
                }
                else {
                    if (channelVisible[i]) ch->load(":/028.5mg Masterpiece GM Bank.sf2");
                }
                ch->ID = IDList[i];
            }
            else if (i < channelXML.childCount()) {
                QDomLiteElement* e = channelXML.childElements[i];
                ch->unserialize(e);
                if (e->attributeValueBool("Solo")) Mx->SoloChannel = i;
            }
            else {
                qDebug() << channelVisible[i] << "Load Masterpiece";
                if (channelVisible[i]) ch->load(":/028.5mg Masterpiece GM Bank.sf2");
            }
            ch->setVisible(channelVisible[i]);
        }
        if (!channelMatch) {
            for (int i = 0; i < channelcount; i++) {
                const QString channelNumber = QString::number(i + 1);
                if (channelVisible[i]) {
                    DeviceList.connect("StereoMixer 1 In " + channelNumber,"Instrument " + channelNumber + " Out");
                }
            }
            for (int i = 0; i < MIDIFile2Wave::effectCount; i++) {
                const QString effectNumber = QString::number(i + 1);
                DeviceList.connect("Effect " + effectNumber + " In","StereoMixer 1 Send " + effectNumber);
                DeviceList.connect("StereoMixer 1 Return " + effectNumber,"Effect " + effectNumber +" Out");
            }
        }
    }
    for (int i = 0; i < channelcount; i++) {
        const QString channelNumber = QString::number(i + 1);
        if (MFR.fileType() == 0) {
            DeviceList.connect("Instrument " + channelNumber + " MIDI In","MIDIFilePlayer 1 MIDI Out");
        }
        else {
            if (channelVisible[i]) DeviceList.connect("Instrument " + channelNumber + " MIDI In","MIDIFilePlayer " + channelNumber + " MIDI Out");
        }
    }
    mixerWidget->showMaster(Mx,&Effects);
    mutex.unlock();
    mixerWidget->adjustSize();
    Mx->setDisabled(false);
    mixerWidget->start();
    mixerWidget->show();
    m_Form->setUpdatesEnabled(true);
}

void CMIDIFile2Wave::center()
{
    //m_Form->setFixedSize(mixerWidget->sizeHint());
    mixerWidget->adjustSize();
    QRect r = QApplication::screens().constFirst()->availableGeometry();
    m_Form->move(r.center()-m_Form->rect().center());
}

void CMIDIFile2Wave::setTitle(const QString& t)
{
    m_Form->setWindowTitle(t);
}

void CMIDIFile2Wave::NoteOn(int Track, byte Pitch, byte Channel, byte Velocity, byte Patch, byte Bank)
{
    if (Track < Instruments.size()) (Instruments[Track])->NoteOn(Pitch,Channel,Velocity,Patch,Bank);
}

void CMIDIFile2Wave::NoteOff(int Track, byte Pitch, byte Channel)
{
    if (Track < Instruments.size()) (Instruments[Track])->NoteOff(Pitch,Channel);
}

MIDITimeList CMIDIFile2Wave::mSecList(const MIDITimeList& tickList)
{
    return MFR.mSecList(tickList);
}

ulong64 CMIDIFile2Wave::mSecsToEvent(const CMIDIEvent &event)
{
    return MFR.mSecsToEvent(event);
}

void CMIDIFile2Wave::unserializeCustom(const QDomLiteElement* xml)
{
    if (!xml) return;
    if (!filename().isEmpty()) loadMixer(xml);
}

void CMIDIFile2Wave::clearMixer()
{
    for (CDeviceContainer* d : std::as_const(Effects)) d->ClearDevice();
    for (CDeviceContainer* d : std::as_const(Instruments)) d->ClearDevice();
}

void CMIDIFile2Wave::loadMixer(const QDomLiteElement* xml)
{
    Mx->setDisabled(true);
    //qDebug() << "CMIDIFile2Wave loadMixer" << xml->toString();
    for (CDeviceContainer* d : std::as_const(Effects)) d->ClearDevice();
    if (const QDomLiteElement* Mixer=xml->elementByTag("Mixer"))
    {
        mixerWidget->unserialize(Mixer);
        for (int i=0;i<Effects.size();i++)
        {
            if (const QDomLiteElement* e=Mixer->elementByTag("Effect"+QString::number(i))) Effects[i]->unserializeCustom(e->elementByTag("Custom"));
        }
    }
    else
    {
        for (CDeviceContainer* d : std::as_const(Instruments)) d->ClearDevice();
    }
    //m_Form->setFixedSize(mixerWidget->sizeHint());
    mixerWidget->adjustSize();
    Mx->setDisabled(false);
}

void CMIDIFile2Wave::serializeCustom(QDomLiteElement* xml) const
{
    QDomLiteElement* Mixer=xml->appendChild("Mixer");
    mixerWidget->serialize(Mixer);
    for (int i=0;i<Effects.size();i++)
    {
        Effects[i]->serializeCustom(Mixer->appendChild("Effect"+QString::number(i))->appendChild("Custom"));
    }
}

void CMIDIFile2Wave::execute(const bool Show)
{
    if (Show)
    {
        if (filename().isEmpty())
        {
            if (!openFile(selectFile(MIDIFilePlayer::MIDIFilter))) return;
        }
    }
    IDevice::execute(Show);
}
/*
void CMIDIFile2Wave::hideForm()
{
    DeviceList.hideForms();
    m_Form->hide();
}
*/
/*
void CMIDIFile2Wave::skip(const ulong MilliSeconds)
{
    DeviceList.skip(MilliSeconds);
}
*/
CMIDIFile2Wave::~CMIDIFile2Wave()
{
    if (m_Initialized) clear();
    //qDebug() << "Exit CMIDIFile2Wave";
}

void CMIDIFile2Wave::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addTickerDevice(&DeviceList);
    setDeviceParent(&DeviceList);
    addJackStereoOut(stereoout);
    addParameter(CParameterVars::Percent,"TempoAdjust","%",1,200,0,nullptr,100);
    addParameterTune();
    addParameterPercent("Humanize");
    addFileParameter();
    m_Form=new CMIDI2WavForm(this,MainWindow);
    mixerWidget=FORMFUNC(CMIDI2WavForm)->MW;
    for (int i=Effects.size();i<MIDIFile2Wave::effectCount;i++)
    {
        Effects.append(dynamic_cast<CDeviceContainer*>(DeviceList.addDevice(new CDeviceContainer("Effect"),i+1,m_MainWindow)));
    }
    updateDeviceParameter();
}

bool CMIDIFile2Wave::isEmpty()
{
    return MIDIFilePlayers.isEmpty();
}

bool CMIDIFile2Wave::isVisible()
{
    return m_Form->isVisible();
}

void CMIDIFile2Wave::clear()
{
    if (Mx) {
        Mx->setDisabled(true);
        Mx->removerEffectRacksFromDeviceList(&DeviceList);
    }
    DeviceList.pause();
    mixerWidget->stop();
    DeviceList.disconnectAll();
    mixerWidget->clear();
    Effects.clear();
    DeviceList.clear();
}

void CMIDIFile2Wave::loadEffect(int index, const QString& filename)
{
    if (Effects[index]->fileIsValid(filename))
    {
        const QDomLiteElement fileElement=QDomLite::elementFromString(R"(<Custom DeviceType="VSTHost" File=")"+filename+R"("/>)");
        Effects[index]->unserializeCustom(&fileElement);
    }
}

ulong CMIDIFile2Wave::ticks() const
{
    return qMax<ulong>(MFR.ticks(), IDevice::ticks());
    //return MFR.duration();
}

ulong CMIDIFile2Wave::milliSeconds() const
{
    return qMax<ulong>(MFR.milliSeconds(), IDevice::milliSeconds());
    //return MFR.milliSeconds();
}

ulong64 CMIDIFile2Wave::samples() const
{
    return qMax<ulong64>(MFR.samples(), IDevice::samples());
    //return MFR.milliSeconds();
}
