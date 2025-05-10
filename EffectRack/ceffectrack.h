#ifndef CEFFECTRACK_H
#define CEFFECTRACK_H

#include "cdevicelist.h"
#include "crackcontainer.h"
#include "qsignalmenu.h"
//#include <QLabel>

class CEffectRackForm : public CSoftSynthsForm, public IHost
{
    Q_OBJECT
public:
    explicit CEffectRackForm(IDevice* Device, QWidget *parent = 0);
    ~CEffectRackForm(){}
    COutJack* insideOut;
    CInJack* insideIn;
    void updateConnections();
    void init(CInJack* in, COutJack* out);
    int deviceCount();
    void unserializeCustom(const QDomLiteElement* xml);
    void serializeCustom(QDomLiteElement* xml) const;
    void parameterChange(IDevice* device, const CParameter* parameter);
protected:
    bool event(QEvent* e);
public slots:
    void addDevice();
    void removeDevice(IDevice*);
    void PluginMenuClicked(QString);
private:
    CRackContainer* m_Rack;
    CDeviceList m_DeviceList;
    QWidget* m_Toolbar;
    QSignalMenu* PluginsPopup;
    //MouseEvents eventHandler;
    //QWidget* m_RackWidget;
private slots:
    void rackMousePressed(IDevice*, QPoint);
    void rackDrop(QDropEvent*);
    void rackDragEnter(QDragEnterEvent*);
    //void showAutomation(IDevice* d);
signals:
    void controlChanged(IDevice*, const CParameter*);
};

class CEffectRack : public IDevice
{
public:
    CEffectRack();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
    void process();
    //void tick();
    void mixerChannelProc(CStereoBuffer* buffer);
private:
    enum JackNames
    {jnIn,jnOut,jnInsideIn};
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    CStereoBuffer* InBuffer;
};

#endif // CEFFECTRACK_H
