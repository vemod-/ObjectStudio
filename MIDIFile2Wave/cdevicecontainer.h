#ifndef CDEVICECONTAINER_H
#define CDEVICECONTAINER_H

#include <QWidget>
#include <softsynthsclasses.h>
#include <csf2player.h>
#include <cvsthost.h>
#include <caudiounithost.h>
#include <QStringList>

class CDeviceContainer : public IDevice
{
public:
    CDeviceContainer(const QString &Name);
    ~CDeviceContainer();
    float* GetNextA(const int ProcIndex);
    void* GetNextP(const int ProcIndex);
    void Init(const int Index, void* MainWindow);
    void Load(const QString &XML);
    const QString Save();
    void Execute(const bool Show);
    void RaiseForm();
    void HideForm();
    const QString FileName();
    void Tick();
    void Play(const bool FromStart);
    void Pause();
    void UpdateHost();
    void Activate();
    const QString CurrentPresetName(const short channel=-1);
    void SetParameterValue(const QString &Name, const int Value);
    int GetParameterValue(const QString &Name);
    const QString OpenFile(const QString &Filter);
    const QStringList Banks();
    const QStringList Presets(const int Bank=0);
    void SetCurrentPreset(const int Bank, const int Preset);
    int CurrentBank(const short channel=-1);
    int CurrentPreset(const short channel=-1);
private:
    enum JackNames
    {jnIn,jnMIDIIn,jnOut,jnInsideIn,jnInsideMIDIIn};
    void Process();
    void ClearDevice();
    IDevice* m_Device;
    QString m_DeviceType;
    QString m_PresetName;
    short bank;
    short preset;
    COutJack* InsideIn;
    COutJack* InsideMIDIIn;
    CInJack* InsideOut;
    CInJack* DeviceIn;
    CInJack* DeviceMIDIIn;
    COutJack* DeviceOut;
    QList<IJack*> JacksCreated;
    float* InBuffer;
    CMIDIBuffer* MIDIBuffer;
    bool m_Bypass;
};

#endif // CDEVICECONTAINER_H
