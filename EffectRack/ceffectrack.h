#ifndef CEFFECTRACK_H
#define CEFFECTRACK_H

#ifdef devicejacks
#undef devicejacks
#endif
#define devicejacks stereoin,stereoout
#ifdef devicecategory
#undef devicecategory
#endif
#define devicecategory Effect | Container

#include "cdevicelist.h"
#include "cparameterscontainer.h"
#include "qsignalmenu.h"
//#include <QLabel>

class CEffectRackForm : public CSoftSynthsForm, public IHost
{
    Q_OBJECT
public:
    explicit CEffectRackForm(IDevice* Device, QWidget *parent = 0);
    ~CEffectRackForm();
    COutJack* insideOut;
    CInJack* insideIn;
    void updateConnections();
    void init(CInJack* in, COutJack* out);
    int deviceCount();
    void unserializeCustom(const QDomLiteElement* xml);
    void serializeCustom(QDomLiteElement* xml) const;
    void parameterChange(IDevice* device, const CParameter* parameter);
    void updateDeviceJacks();
    void closeAutomation(IDevice* /*device*/);
protected:
    bool event(QEvent* e);
    //void dragEnterEvent(QDragEnterEvent* e);
    //void dropEvent(QDropEvent* e);
public slots:
    void addDevice();
    void removeDevice(IDevice*);
    void PluginMenuClicked(QString);
    void reorderDevices(int deviceIndex, int move);
private:
    CParametersContainer* m_Rack;
    CDeviceList m_DeviceList;
    QWidget* m_Toolbar;
    QSignalMenu* PluginsPopup;
private slots:
    //void rackMousePressed(IDevice*, QPoint);
    void rackSizeChanged();
signals:
    void controlChanged(IDevice*, const CParameter*);
    void connectionsChanged();
};

class CEffectRack : public IDevice
{
public:
    CEffectRack();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
    void process();
    void mixerChannelProc(CStereoBuffer* buffer);
private:
    enum JackNames
    {devicejacks,jnInsideIn};
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    CStereoBuffer* InBuffer;
};

#endif // CEFFECTRACK_H
