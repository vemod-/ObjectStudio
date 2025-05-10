#ifndef CDEVICECONTAINER_H
#define CDEVICECONTAINER_H

#include <QWidget>
#include "idevice.h"

class CDeviceContainer : public IDevice
{
public:
    CDeviceContainer(const QString &name);
    ~CDeviceContainer();
    CAudioBuffer* getNextA(const int ProcIndex);
    CMIDIBuffer* getNextP(const int ProcIndex);
    void init(const int Index, QWidget* MainWindow);
    void unserializeCustom(const QDomLiteElement* xml);
    void serializeCustom(QDomLiteElement* xml) const;
    void execute(const bool Show);
    void raiseForm();
    void hideForm();
    void cascadeForm(QPoint& p);
    bool hasUI() const;
    QWidget* UI() const;
    const QPixmap* picture() const;
    IDevice* childDevice(const int index) const;
    int childDeviceCount() const;
    const QString filename() const;
    //void tick();
    void updateHostParameter(const CParameter* p = nullptr);
    void setHost(IHost* host);
    void activate();
    const QString currentBankPresetName(const short channel=-1) const;
    void setDeviceType(const QString &Filter);
    const QString deviceType() const;
    void ClearDevice();
    static const QStringList instrumentList();
    static const QStringList effectList();
    const QString caption() const;
    const QStringList bankNames() const;
    const QStringList presetNames(const int bank=0) const;
    int bankPresetNumber(const int bank, const int preset) const;
    void setCurrentBankPreset(const int index);
    long currentBankPreset(const short channel=-1) const;
    void NoteOn(byte Pitch, byte Channel=0, byte Velocity=127, byte Patch=0, byte Bank=0);
    void NoteOff(byte Pitch, byte Channel=0);
    void setParameterValue(const QString &name, const int value);
    int parameterValue(const QString &name) const;
protected:
    enum JackNames
    {jnIn,jnMIDIIn,jnOut,jnInsideIn,jnInsideMIDIIn};
    void process();
    IDevice* m_Device;
    QString m_DeviceType;
    int m_Program;
    COutJack* InsideIn;
    COutJack* InsideMIDIIn;
    //CInJack* InsideOut;
    CInJack* DeviceIn;
    CInJack* DeviceMIDIIn;
    //COutJack* DeviceOut;
    QList<IJack*> JacksCreated;
    CStereoBuffer* InBuffer;
    CMIDIBuffer* MIDIBuffer;
    CMIDIBuffer MIDIInBuffer;
    CMIDIBuffer tempBuffer;
    bool m_Bypass;
    int outProcIndex;
};

#endif // CDEVICECONTAINER_H
