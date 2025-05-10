#ifndef IDEVICE_H
#define IDEVICE_H

#include "csoftsynthsform.h"
#include "cparameter.h"
#include "cprogrambank.h"
#include <QFileDialog>
#include <QPixmap>
#include "qdomlite.h"
#include "cfileparameter.h"
#include "ijack.h"

int nativeAlert(QWidget *parent, QString messageText, QString informativeText, QStringList buttons);

void nativeMessage(QWidget *parent, QString messageText, QString informativeText);

WId windowNumberOfWidget(QWidget* w);

void setWindowInFrontOf(QWidget* w, QWidget* w1);

void mac_raise(QWidget* w);

bool isWindowPartlyVisible(QWidget* w);

#pragma pack(push,1)

class IDevice : public IDeviceBase, public ITicker, protected IPresetRef
{
public:
    inline IDevice() : m_Initialized(false),
        m_DeviceID("No ID"),
        m_FileParameter(nullptr),
        m_Name("Generic Device Class"),
        m_BufferSize(presets.ModulationRate),
        m_Host(nullptr),
        //m_TickerDevice(nullptr),
        m_Process(false),
        m_Playing(false),
        m_Form(nullptr),
        m_DeviceParent(nullptr) {
    }
    virtual ~IDevice();
    inline CParameter* parameter(const int Index) const { return m_Parameters[uint(Index)]; }
    inline int parameterCount() const { return int(m_Parameters.size()); }
    virtual void setHost(IHost* Host) { m_Host=Host; }
    virtual void addTickerDevice(ITicker* Ticker) {
        //m_TickerDevice = Ticker;
        if (Ticker) {
            if (!m_TickerDevices.contains(Ticker)) m_TickerDevices.push_back(Ticker);
        }
    }
    virtual void removeTickerDevice(ITicker* Ticker) {
        m_TickerDevices.removeOne(Ticker);
    }
    virtual void clearTickerDevices() {
        m_TickerDevices.clear();
    }
    virtual void setDeviceParent(IDeviceParent* Parent) { m_DeviceParent = Parent; }
    virtual const QString selectFile(const QString& Filter) {
        return (m_FileParameter) ? m_FileParameter->selectFile(Filter) : QString();
    }
    virtual void raiseForm() {
        if (m_Form != nullptr)
        {
            if (m_Form->isVisible()) {
                m_Form->raise();
                //mac_raise(m_Form);
            }
        }
    }
    virtual void hideForm() {
        if (m_Form)  m_Form->setVisible(false);
        if (m_DeviceParent) m_DeviceParent->hideForms();
    }
    virtual void cascadeForm(QPoint& p) {
        if (m_Form) {
            if (m_Form->isVisible()) {
                m_Form->move(p);
                m_Form->raise();
                p+=QPoint(24,24);
            }
        }
        if (m_DeviceParent) m_DeviceParent->cascadeForms(p);
    }
    virtual int childDeviceCount() const {
        return (m_DeviceParent) ? m_DeviceParent->deviceCount() : 0;
    }
    virtual IDevice* childDevice(const int index) const {
        return (m_DeviceParent) ? m_DeviceParent->device(index) : nullptr;
    }
    inline IJack* jack(const int Index) const { return m_Jacks[uint(Index)]; }
    inline CInJack* inJack(const int Index) const { return m_InJacks[uint(Index)]; }
    inline COutJack* outJack(const int Index) const { return m_OutJacks[uint(Index)]; }
    inline IJack* jack(const IJack::Directions Direction, const IJack::AttachModes Attachmode) const {
        return (Direction == IJack::In) ? static_cast<IJack*>(inJack(Attachmode)) : outJack(Attachmode);
    }
    inline COutJack* outJack(const IJack::AttachModes Attachmode) const {
        for (COutJack* j : m_OutJacks) if (j->attachMode & Attachmode) return j;
        return nullptr;
    }
    inline CInJack* inJack(const IJack::AttachModes Attachmode) const {
        for (CInJack* j : m_InJacks) if (j->attachMode & Attachmode) return j;
        return nullptr;
    }
    inline int jackCount() const { return int(m_Jacks.size()); }
    inline int inJackCount() const { return int(m_InJacks.size()); }
    inline int outJackCount() const { return int(m_OutJacks.size()); }
    inline int index() const { return m_Index; }
    inline const QString name() const { return m_Name; }
    virtual const QPixmap* picture() const {
        if (m_Form) {
            QPixmap* pm = new QPixmap(m_Form->grab().scaled(m_Form->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
            //QPixmap* ret = new QPixmap(pm);
            pm->setDevicePixelRatio(1);
            return pm;
        }
        return nullptr;
    }
    virtual const QString filename() const {
        return (m_FileParameter) ? m_FileParameter->filename() : QString();
    }
    virtual bool fileIsValid(const QString& path) const {
        return (m_FileParameter) ? m_FileParameter->fileIsValid(path) : false;
    }
    virtual bool openFile(const QString& filename) {
        return (m_FileParameter) ? m_FileParameter->openFile(filename) : false;
    }
    inline const QString deviceID() const { return m_DeviceID; }
    inline const QString jackID(const int JackIndex) const { return m_Jacks[uint(JackIndex)]->jackID(); }
    virtual CAudioBuffer* getNextA(const int ProcIndex) {
        if (m_Process) {
            m_Process=false;
            process();
        }
        return m_AudioBuffers[ProcIndex];
    }
    virtual void init(const int Index, QWidget* MainWindow) {
        m_Index=Index;
        m_MainWindow=MainWindow;
        m_DeviceID=m_Name;
        if (Index != 0) m_DeviceID += " " + QString::number(m_Index);
        m_Initialized=true;
        m_Process=false;
        m_Playing=false;
        CProgramBank::init(m_Name);
    }
    virtual void initWithFile(const QString& /*path*/) {}
    virtual void unserializeCustom(const QDomLiteElement* /*xml*/) { }
    virtual void serializeCustom(QDomLiteElement* /*xml*/) const { }
    void unserializeCustomParameters(const QDomLiteElement* xml) {
        if (!xml) return;
        QMutexLocker locker(&mutex);
        if (m_FileParameter) m_FileParameter->unserialize(xml);
        unserializeCustom(xml);
        if (m_Form) m_Form->unserializeCustom(xml);
    }
    void serializeCustomParameters(QDomLiteElement* xml) const {
        if (m_FileParameter) m_FileParameter->serialize(xml);
        serializeCustom(xml);
        if (m_Form) m_Form->serializeCustom(xml);
    }
    void unserializeUI(const QDomLiteElement* xml) {
        if (!xml) return;
        QMutexLocker locker(&mutex);
        if (m_Form) m_Form->unserialize(xml);
    }
    void serializeUI(QDomLiteElement* xml) const {
        if (m_Form) m_Form->serialize(xml);
    }
    virtual void execute(const bool Show) {
        if (m_Form != nullptr) {
            QMutexLocker locker(&mutex);
            (Show) ? m_Form->show() : m_Form->setVisible(false);
        }
    }
    inline bool isPlaying() const {
        return m_Playing;
    }
    //ITicker
    virtual void tick()
    {
        //if (m_TickerDevice) m_TickerDevice->tick();
        for (ITicker* i : std::as_const(m_TickerDevices)) if (i) i->tick();
        m_Process=true;
    }
    virtual void skip(const ulong64 samples)
    {
        //if (m_TickerDevice) m_TickerDevice->skip(milliSeconds);
        for (ITicker* i : std::as_const(m_TickerDevices)) if (i) i->skip(samples);
        //m_Playing = true;
    }
    virtual void play(const bool FromStart)
    {
        qDebug() << "IDevice::play";
        //if (m_TickerDevice) m_TickerDevice->play(FromStart);
        for (ITicker* i : std::as_const(m_TickerDevices)) if (i) i->play(FromStart);
        m_Playing = true;
    }
    virtual void pause()
    {
        //if (m_TickerDevice) m_TickerDevice->pause();
        for (ITicker* i : std::as_const(m_TickerDevices)) if (i) i->pause();
        m_Playing = false;
    }
    virtual ulong ticks() const
    {
        ulong d = 0;
        for (ITicker* i : m_TickerDevices) if (i) d = qMax<ulong>(d, i->ticks());
        return d;
        //return (m_TickerDevice) ? m_TickerDevice->duration() : 0;
    }
    virtual ulong milliSeconds() const
    {
        ulong d = 0;
        for (ITicker* i : m_TickerDevices) if (i) d = qMax<ulong>(d, i->milliSeconds());
        return d;
        //return (m_TickerDevice) ? m_TickerDevice->milliSeconds() : 0;
    }
    virtual ulong64 samples() const
    {
        ulong64 d = 0;
        for (ITicker* i : m_TickerDevices) if (i) d = qMax<ulong64>(d, i->samples());
        return d;
        //return (m_TickerDevice) ? m_TickerDevice->milliSeconds() : 0;
    }
    //MainWindow
    void requestPlay(const bool FromStart) {
        qDebug() << "requestPlay" << FromStart;
        mainPlayer()->play(FromStart);
    }
    void requestSkip(const ulong64 samples) {
        qDebug() << "requestSkip" << samples;
        mainPlayer()->skip(samples);
    }
    void requestPause() {
        qDebug() << "requestPause";
        mainPlayer()->pause();
    }
    bool requestIsPlaying() const {
        //qDebug() << "requestIsPlaying" << mainPlayer()->isPlaying();
        return mainPlayer()->isPlaying();
    }
    ulong requestTicks() const {
        return mainPlayer()->ticks();
    }
    ulong requestMilliSeconds() const {
        return mainPlayer()->milliSeconds();
    }
    ulong64 requestSamples() const {
        return mainPlayer()->samples();
    }
    ulong64 requestCurrentSample() const {
        return mainPlayer()->currentSample();
    }
    ulong requestCurrentMilliSecond() const {
        return mainPlayer()->currentMilliSecond();
    }
    void requestRenderWaveFile(const QString path) {
        mainPlayer()->renderWaveFile(path);
    }
    //IHost
    virtual void updateHostParameter(const CParameter* p = nullptr) { if (m_Host) m_Host->parameterChange(this,p); }
    virtual void activate() { if (m_Host != nullptr) m_Host->activate(this); }
    virtual bool hasUI() const { return (m_Form != nullptr); }
    virtual QWidget* UI() const { return static_cast<QWidget*>(m_Form); }
    virtual bool UIisVisible() {
        if (m_Form == nullptr) return false;
        return isWindowPartlyVisible(m_Form);
    }
    virtual void toggleUI() {
        if (UIisVisible()) {
            execute(false);
            return;
        }
        activate();
        execute(true);
        raiseForm();
    }
    virtual void setCurrentBankPreset(const int index) { setCurrentProgram(index); }
    virtual long currentBankPreset(const short /*channel*/=-1) const { return currentProgram(); }
    virtual const QString currentBankPresetName(const short /*channel*/=-1) const { return currentProgramName(); }
    virtual int bankPresetNumber(const int /*bank*/, const int preset) const { return preset; }
    virtual const QStringList bankNames() const { return QStringList(); }
    virtual const QStringList presetNames(const int /*bank*/=0) const { return programNames(); }
    void setCurrentProgram(const int index)
    {
        QMutexLocker locker(&mutex);
        const QStringList& pn = programNames();
        if ((index<pn.size()) && (index>-1)) setCurrentProgram(pn[index]);
    }
    void setCurrentProgram(const QString& programName)
    {
        QMutexLocker locker(&mutex);
        m_CurrentProgram.clear();
        unserializeProgram(CProgramBank::getProgram(programName, m_Name));
    }
    void unserializeStandardParameters(const QDomLiteElement* Parameters)
    {
        if (!Parameters) return;
        QMutexLocker locker(&mutex);
        for (const QDomLiteElement* XMLParameter : (const QDomLiteElementList)Parameters->elementsByTag(ParameterTag))
        {
            const QString n = XMLParameter->attribute(ParameterNameAttribute);
            for (CParameter* p : m_Parameters) {
                if (p->Name == n) {
                    p->unserialize(XMLParameter);
                    break;
                }
            }
        }
    }
    void unserializeParameters(const QDomLiteElement* Parameters)
    {
        if (!Parameters) return;
        QMutexLocker locker(&mutex);
        unserializeStandardParameters(Parameters);
        unserializeCustomParameters(Parameters->elementByTag("Custom"));
    }
    void unserializeDevice(const QDomLiteElement* Parameters)
    {
        if (!Parameters) return;
        QMutexLocker locker(&mutex);
        unserializeParameters(Parameters);
        unserializeUI(Parameters->elementByTag("Custom"));
    }
    void unserializeProgram(const QDomLiteElement* Parameters)
    {
        if (!Parameters) return;
        QMutexLocker locker(&mutex);
        unserializeParameters(Parameters);
        m_CurrentProgram=Parameters->attribute(PresetNameAttribute);
    }
    void serializeStandardParameters(QDomLiteElement* Parameters) const
    {
        if (!Parameters) return;
        for (const CParameter* p : m_Parameters) p->serialize(Parameters->appendChild(ParameterTag));
    }
    void serializeParameters(QDomLiteElement* Parameters) const
    {
        serializeStandardParameters(Parameters);
        serializeCustomParameters(Parameters->appendChild("Custom"));
    }
    void serializeDevice(QDomLiteElement* Parameters) const
    {
        serializeParameters(Parameters);
        serializeUI(Parameters->elementByTagCreate("Custom"));
    }
    void serializeProgram(QDomLiteElement* Parameters) const
    {
        serializeParameters(Parameters);
    }
    const QString currentProgramMatches() const
    {
        QDomLiteElement curr(PresetTag);
        serializeProgram(&curr);
        return CProgramBank::programMatches(&curr,m_Name);
    }
    const QStringList programNames() const { return CProgramBank::programNames(m_Name); }
    void saveCurrentProgram(const QString& programName=QString()) const
    {
        QString n=programName;
        if (n.isEmpty()) n=m_CurrentProgram;
        if (n.isEmpty()) return;
        QDomLiteElement e(PresetTag);
        serializeProgram(&e);
        CProgramBank::saveProgram(&e,n,m_Name);
    }
    inline int currentProgram() const { return programNames().indexOf(m_CurrentProgram); }
    inline const QString currentProgramName() const { return m_CurrentProgram; }
    bool isGroupedParameter(const int parameterIndex) const
    {
        for (const CParameterGroup* p : m_ParameterGroups)
        {
            if ((parameterIndex >= p->startIndex) && (parameterIndex <= p->endIndex)) return true;
        }
        return false;
    }
    int parameterGroupID(const int parameterIndex) const
    {
        for (const CParameterGroup* p : m_ParameterGroups)
        {
            if ((parameterIndex >= p->startIndex) && (parameterIndex <= p->endIndex)) return p->ID;
        }
        return -1;
    }
    CParameterGroup* parameterGroup(const int groupID)
    {
        for (CParameterGroup* p : m_ParameterGroups)
        {
            if (p->ID == groupID) return p;
        }
        return nullptr;
    }
    int parameterGroupCount() { return int(m_ParameterGroups.size()); }
    int showNativeAlert(QString messageText, QString informativeText, QStringList buttons) {
        return nativeAlert(m_MainWindow,messageText,informativeText,buttons);
    }
    void showNativeMessage(QString messageText, QString informativeText) {
        nativeMessage(m_MainWindow,messageText,informativeText);
    }
protected:
    QRecursiveMutex mutex;
    bool m_Initialized;
    QString m_DeviceID;
    int m_Index;
    QWidget* m_MainWindow;
    CFileParameter* m_FileParameter;
    QString m_Name;
    std::vector<IJack*> m_Jacks;
    std::vector<CInJack*> m_InJacks;
    std::vector<COutJack*> m_OutJacks;
    std::vector<CParameter*> m_Parameters;
    std::vector<CParameterGroup*> m_ParameterGroups;
    inline float Fetch(const int ProcIndex) { return (static_cast<CInJack*>(m_Jacks[uint(ProcIndex)]))->getNext(); }
    inline CMIDIBuffer* FetchP(const int ProcIndex) { return (static_cast<CInJack*>(m_Jacks[uint(ProcIndex)]))->getNextP(); }
    inline CAudioBuffer* FetchA(const int ProcIndex) { return (static_cast<CInJack*>(m_Jacks[uint(ProcIndex)]))->getNextA(); }
    inline CMonoBuffer* FetchAMono(const int ProcIndex) { return static_cast<CMonoBuffer*>(FetchA(ProcIndex)); }
    inline CStereoBuffer* FetchAStereo(const int ProcIndex) { return static_cast<CStereoBuffer*>(FetchA(ProcIndex)); }
    std::vector<CAudioBuffer*> m_AudioBuffers;
    inline CStereoBuffer* StereoBuffer(const int ProcIndex) const { return static_cast<CStereoBuffer*>(m_AudioBuffers[ProcIndex]); }
    inline CMonoBuffer* MonoBuffer(const int ProcIndex) const { return static_cast<CMonoBuffer*>(m_AudioBuffers[ProcIndex]); }
    const uint m_BufferSize;
    IHost* m_Host;
    //ITicker* m_TickerDevice;
    QVector<ITicker*> m_TickerDevices;
    bool m_Process;
    bool m_Playing;
    CSoftSynthsForm* m_Form;
    IDeviceParent* m_DeviceParent;
    IMainPlayer* mainPlayer() const {
        return dynamic_cast<IMainPlayer*>(m_MainWindow);
    }
    void addParameter(CParameter::ParameterTypes Type, const QString& Name, const QString& Unit, const int Min, const int Max, const int DecimalFactor, const QString& ListString,int Value)
    {
        QMutexLocker locker(&mutex);
        m_Parameters.push_back(new CParameter(Type,Name,Unit,Min,Max,DecimalFactor,ListString,Value,this,int(m_Parameters.size())));
    }
    IJack* addJack(const QString& Name,IJack::AttachModes AttachMode,IJack::Directions Direction,int ProcIndex=-1)
    {
        QMutexLocker locker(&mutex);
        return (Direction==IJack::In) ? static_cast<IJack*>(addInJack(Name,AttachMode)) :
                                        addOutJack(Name,AttachMode,ProcIndex);
    }
    COutJack* addOutJack(const QString& Name,IJack::AttachModes AttachMode,int ProcIndex=-1)
    {
        QMutexLocker locker(&mutex);
        if (ProcIndex==-1) ProcIndex=int(m_Jacks.size());
        COutJack* OJ=new COutJack(Name,m_DeviceID,AttachMode,IJack::Out,this,ProcIndex);
        if (ProcIndex >= int(m_AudioBuffers.size())) m_AudioBuffers.resize(ProcIndex+1);
        m_AudioBuffers[ProcIndex]=OJ->audioBuffer;
        m_Jacks.push_back(OJ);
        m_OutJacks.push_back(OJ);
        return OJ;
    }
    CInJack* addInJack(const QString& Name,IJack::AttachModes AttachMode)
    {
        QMutexLocker locker(&mutex);
        CInJack* IJ=new CInJack(Name,m_DeviceID,AttachMode,IJack::In,this);
        m_Jacks.push_back(IJ);
        m_InJacks.push_back(IJ);
        return IJ;
    }
    COutJack* addJackWaveOut(int ProcIndex, const QString& Name="Out")
    {
        return addOutJack(Name,IJack::Wave,ProcIndex);
    }
    CInJack* addJackWaveIn(const QString& Name="In")
    {
        return addInJack(Name,IJack::Wave);
    }
    void addJackDualMonoOut(int ProcIndex, const QString& Name="Out")
    {
        addOutJack(Name+" Left",IJack::Wave,ProcIndex);
        addOutJack(Name+" Right",IJack::Wave,ProcIndex+1);
    }
    void addJackDualMonoIn(const QString& Name="In")
    {
        addInJack(Name+" Left",IJack::Wave);
        addInJack(Name+" Right",IJack::Wave);
    }
    COutJack* addJackStereoOut(int ProcIndex, const QString& Name="Out")
    {
        return addOutJack(Name,IJack::Stereo,ProcIndex);
    }
    CInJack* addJackStereoIn(const QString& Name="In")
    {
        return addInJack(Name,IJack::Stereo);
    }
    CInJack* addJackMIDIIn(const QString& Name="MIDI In")
    {
        return addInJack(Name,IJack::MIDI);
    }
    COutJack* addJackMIDIOut(int ProcIndex, const QString& Name="MIDI Out")
    {
        return addOutJack(Name,IJack::MIDI,ProcIndex);
    }
    CInJack* addJackModulationIn(const QString& Name="Modulation")
    {
        return addInJack(Name,IJack::Voltage);
    }
    COutJack* addJackModulationOut(int ProcIndex, const QString& Name="Modulation Out")
    {
        return addOutJack(Name,IJack::Voltage,ProcIndex);
    }
    void addParameterVolume(const QString& Name="Volume")
    {
        addParameter(CParameter::dB,Name,"dB",0,200,0,"",100);
    }
    void addParameterLevel(const QString& Name="Level")
    {
        addParameter(CParameter::Numeric,Name,"V",0,200,100,"",100);
    }
    void addParameterBias(const QString& Name="Bias")
    {
        addParameter(CParameter::Numeric,Name,"V",-100,100,100,"",0);
    }
    void addParameterRectify(const QString& Name="Rectify")
    {
        addParameter(CParameter::Numeric,Name,"%+/-",-100,100,0,"",0);
    }
    void addParameterSelect(const QString& Name, const QString& SelectString, const int Default = 0)
    {
        const int Max=SelectString.split(ParameterListSeparator).size();
        addParameter(CParameter::SelectBox,Name,"",0,Max-1,0,SelectString,Default);
    }
    void addParameterOffOn(const QString& Name)
    {
        addParameter(CParameter::SelectBox,Name,"",0,1,0,"Off§On",0);
    }
    void addParameterPatchChange(const QString& Name="Patch Change")
    {
        addParameterOffOn(Name);
    }
    void addParameterMIDIChannel(const QString& Name="MIDI Channel")
    {
        QString s("All");
        for (int i=0;i<16;i++) s+="§"+QString::number(i+1);
        addParameterSelect(Name,s);
    }
    void addParameterTrack(const QString& Name="Track")
    {
        QString s("All");
        for (int i=0;i<64;i++) s+="§"+QString::number(i+1);
        addParameterSelect(Name,s);
    }
    void addParameterTranspose(const QString& Name="Transpose")
    {
        addParameter(CParameter::Numeric,Name,"Semitones",-24,24,0,"",0);
    }
    void addParameterTune(const QString& Name="Tune")
    {
        addParameter(CParameter::Numeric,Name,"Hz",43600,44800,100,"",44000);
    }
    void addParameterPercent(const QString& Name="Modulation", const int Default=0)
    {
        addParameter(CParameter::Percent,Name,"%",0,100,0,"",Default);
    }
    void addParameterTime(const QString& Name="Time", const int Default=0)
    {
        addParameter(CParameter::Numeric,Name,"mSec",1,20000,0,"",Default);
    }
    void addParameterCutOff(const QString& Name="Cutoff Frequency",const int Default=CPresets::presets().MaxCutoff)
    {
        addParameter(CParameter::Numeric,Name,"Hz",20,presets.MaxCutoff,0,"",Default);
    }
    void addParameterRate(const QString& Name="Rate",const int Default=100)
    {
        addParameter(CParameter::Numeric,Name,"Sweeps/sec",5,1000,100,"",Default);
    }
    void addParameterFrequency(const QString& Name="Frequency",const int Default=44000)
    {
        addParameter(CParameter::Numeric,Name,"Hz",100,int(presets.HalfRate)*100,100,"",Default);
    }
    void addParameterPan(const QString& Name="Pan",const int Default=0)
    {
        addParameter(CParameter::Percent,Name,"%",-100,100,0,"",Default);
    }
    void addFileParameter(IFileLoader* loader = nullptr, const QString& basePath = QString())
    {
        m_FileParameter = (loader) ? new CFileParameter(this,loader) : new CFileParameter(this);
        if (!basePath.isEmpty()) m_FileParameter->setBasePath(basePath);
    }
    void makeParameterGroup(const int size, const QString& name, const QColor& color = Qt::black)
    {
        m_ParameterGroups.push_back(new CParameterGroup(name, int(m_Parameters.size()),int(m_ParameterGroups.size()),color,int(m_Parameters.size())+(size-1)));
    }
    void startParameterGroup(const QString& name = QString(), const QColor& color = Qt::black)
    {
        m_ParameterGroups.push_back(new CParameterGroup(name, int(m_Parameters.size()),int(m_ParameterGroups.size()),color));
    }
    void endParameterGroup()
    {
        m_ParameterGroups.back()->endIndex=int(m_Parameters.size())-1;
    }
    virtual void updateParameter(const CParameter* p = nullptr) {
        //QMutexLocker locker(&mutex);
        updateDeviceParameter(p);
        updateHostParameter(p);
    }
    virtual void updateDeviceParameter(const CParameter* /*p*/ = nullptr) {}
    virtual void process() {}
    QString m_CurrentProgram;
};

#pragma pack(pop)

void fixMaximizeButton(QWidget *w, bool resizable);

typedef IDevice*(*instancefunc)();

#define instancefn(idx) reinterpret_cast<instancefunc>(CAddIns::addInInstanceFunction(idx))


#endif // IDEVICE_H
