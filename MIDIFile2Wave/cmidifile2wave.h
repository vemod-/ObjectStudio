#ifndef CMIDIFILE2WAVE_H
#define CMIDIFILE2WAVE_H

//#include <QWidget>
//#include "idevice.h"
#include "cdevicelist.h"
#include "cmidifileplayer.h"
#include "cmixerwidget.h"
//#include <QGridLayout>
#include "cdevicecontainer.h"
#include "cuimap.h"

#define MIDIFILE2WAVECLASS DEVICEFUNC(CMIDIFile2Wave)

namespace MIDIFile2Wave
{
const int effectCount=3;
}

class CMIDIFile2Wave : public IDevice, public IFileLoader
{
public:
    CMIDIFile2Wave();
    ~CMIDIFile2Wave();
    //void tick();
    void play(const bool FromStart);
    //void pause();
    CAudioBuffer* getNextA(const int ProcIndex);
    void init(const int Index, QWidget* MainWindow);
    void unserializeCustom(const QDomLiteElement* xml);
    void loadMixer(const QDomLiteElement* xml);
    void clearMixer();
    void serializeCustom(QDomLiteElement* xml) const;
    void execute(const bool Show);
    //void hideForm();
    //void skip(const ulong milliSeconds);
    ulong ticks() const;
    ulong milliSeconds() const;
    ulong64 samples() const;
    void clear();
    bool isEmpty();
    bool isVisible();
    bool refreshMIDIFile(const QString& filename);
    void assign(const QByteArray& b);
    void center();
    void setTitle(const QString& t);
    void NoteOn(int Track, byte Pitch, byte Channel=0, byte Velocity=127, byte Patch=0, byte Bank=0);
    void NoteOff(int Track, byte Pitch, byte Channel=0);
    MIDITimeList mSecList(const MIDITimeList& tickList);
    ulong64 mSecsToEvent(const CMIDIEvent& event);
    CMixerWidget* mixerWidget;
    bool hideEmptyChannels() const { return HideEmptyChannels; }
    void setHideEmptyChannels(const bool v) { HideEmptyChannels=v; }
    CDeviceListBase* deviceList() { return &DeviceList; }
    QStringList IDList;
private:
    enum JackNames
    {jnOut};
    enum ParameterNames
    {pnTempoAdjust,pnTune,pnHumanize};
    CMIDIFileReader MFR;
    QList<CDeviceContainer*> Effects;
    QList<CMIDIFilePlayer*> MIDIFilePlayers;
    QList<CDeviceContainer*> Instruments;
    bool loadFile(const QString& filename);
    void initWithFile(const QString& path);
    void loadEffect(int index, const QString& filename);
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    CStereoMixer* Mx;
    CDeviceList DeviceList;
    bool HideEmptyChannels;
};

class CMIDI2WavForm : public CSoftSynthsForm
{
    Q_OBJECT
public:
    explicit CMIDI2WavForm(IDevice* Device, QWidget *parent = 0);
    ~CMIDI2WavForm(){}
    CMixerWidget* MW;
    CUIMap* Map;
protected:
    bool event(QEvent* event);
private:
    QMenu* UIMenu;
private slots:
    void showMap();
    void hideUIs();
    void cascadeUIs();
    void hideMap();
};

#endif // CMIDIFILE2WAVE_H
