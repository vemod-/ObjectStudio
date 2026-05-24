#ifndef CSTEREOCONTAINERBASE_H
#define CSTEREOCONTAINERBASE_H

//#define devicejacks stereoin,midiin,stereoout

#include "idevice.h"
#include "qsignalmenu.h"

class CStereoContainerBase;

class monoItems : public QSignalMenu {
    Q_OBJECT
public:
    monoItems(CStereoContainerBase* c);
    CStereoContainerBase* container;
public slots:
    void setDeviceType(QString s);
};

struct extraInJack {
    CInJack* inJack;
    COutJack* insideInJack;
    CInJack* deviceInJack[2];
    int index;
    float value;
};

class CStereoContainerBase : public IDevice
{
public:
    CStereoContainerBase(const QString& name);
    ~CStereoContainerBase();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
    CMIDIBuffer* getNextP(const int ProcIndex);
    float getNext(const int ProcIndex);
    void unserializeParameters(const QDomLiteElement* xml);
    void serializeParameters(QDomLiteElement* xml) const;
    void execute(bool /*show*/);
    void activate();
    void raiseForm();
    void hideForm();
    void cascadeForm(QPoint& p);
    bool hasUI() const;
    QWidget* UI() const;
    const QPixmap* picture() const;
    const QString filename() const;
    const QString currentBankPresetName(const short channel) const;
    const QStringList bankNames() const;
    const QStringList presetNames(const int bank) const;
    long currentBankPreset(const short channel) const;
    int bankPresetNumber(const int bank, const int preset) const;
    void setCurrentBankPreset(const int index);
    void NoteOn(byte Pitch, byte Channel, byte Velocity, byte Patch, byte Bank);
    void NoteOff(byte Pitch, byte Channel);
    void setParameterValue(const QString &name, const int value);
    int parameterValue(const QString &name) const;
    IDevice* childDevice(const int index) const;
    int childDeviceCount() const;
    void setDeviceType(const QString &Filter);
    void ClearDevice();
    void selectDevice();
private:
    enum JackNames
    {stereoin,midiin,stereoout,jnInsideInLeft,jnInsideInRight,jnInsideMIDIIn};
    enum ParameterNames
    {};
    void updateDeviceParameter(const CParameter* p = nullptr);
    void setDeviceParameters(const int i);
    void process();
    bool hasDevice() const;
    IDevice* m_Devices[2];
    COutJack* InsideIn[2];
    COutJack* InsideMIDIIn;
    //COutJack* InsideModulationIn;
    //CInJack* InsideOut;
    CInJack* DeviceIn[2];
    CInJack* DeviceMIDIIn[2];
    //CInJack* DeviceModulationIn[2];
    //COutJack* DeviceOut;
    QList<IJack*> JacksCreated;
    QList<extraInJack> ExtraJacks;
    CStereoBuffer* InBuffer;
    //float modulation;
    CMIDIBuffer* MIDIBuffer;
    CMIDIBuffer MIDIInBuffer;
    CMIDIBuffer tempBuffer;
    bool m_Bypass;
    int outProcIndex;
    QString m_DeviceType;
};

#endif // CSTEREOCONTAINERBASE_H
