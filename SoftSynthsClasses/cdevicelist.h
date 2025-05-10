#ifndef CDEVICELIST_H
#define CDEVICELIST_H

#include <QtCore>
#include "cjackcollection.h"
#include "cautomationplayer.h"

class CDeviceListBase : public ITicker, public IDeviceParent
{
protected:
    CJackCollection m_Jacks;
    QVector<IDevice*> m_Devices;
    IDevice* addDevice(IDevice* D)
    {
        QMutexLocker locker(&mutex);
        m_Jacks.addDevice(D);
        m_Devices.append(D);
        D->setHost(m_Host);
        return D;
    } // test purposes only! Must be private!!!
    IHost* m_Host;
private:
    QRecursiveMutex mutex;
public:
    CDeviceListBase() : m_Host(nullptr) {}
    virtual ~CDeviceListBase();
    virtual void setHost(IHost* host) { m_Host=host; }
    virtual inline QVector<IDevice*>* devices() { return &m_Devices; }
    virtual inline int findFreeIndex(const QString& Name) const
    {
        int i=0;
        while (device(Name + " " + QString::number(++i)) != nullptr) ;
        return i;
    }
    virtual inline bool isConnected(const QString& J1, const QString& J2)
    {
        return (!m_Jacks.contains(J1)) ? false : m_Jacks[J1]->isConnectedTo(m_Jacks[J2]);
    }
    virtual ulong ticks() const
    {
        ulong retval=0;
        for (const IDevice* d : m_Devices) retval=qMax<ulong>(d->ticks(),retval);
        return retval;
    }
    virtual ulong milliSeconds() const
    {
        ulong retval=0;
        for (const IDevice* d : m_Devices) retval=qMax<ulong>(d->milliSeconds(),retval);
        return retval;
    }
    virtual ulong64 samples() const
    {
        ulong64 retval=0;
        for (const IDevice* d : m_Devices) retval=qMax<ulong64>(d->samples(),retval);
        return retval;
    }
    virtual inline IJack* jack(const int Index) const { return m_Jacks[Index]; }
    virtual inline IJack* jack(const QString& JackID) const { return m_Jacks[JackID]; }
    virtual inline CInJack* inJack(const int Index) const { return m_Jacks.inJack(Index); }
    virtual inline COutJack* outJack(const int Index) const { return m_Jacks.outJack(Index); }
    virtual inline IDevice* device(const int Index) const { return m_Devices[Index]; }
    virtual inline IDevice* device(const QString& DeviceID) const
    {
        for (IDevice* d : m_Devices) if (d->deviceID()==DeviceID) return d;
        return nullptr;
    }
    virtual inline int jackCount() const { return m_Jacks.size(); }
    virtual inline int inJackCount() const { return m_Jacks.inJackCount(); }
    virtual inline int outJackCount() const { return m_Jacks.outJackCount(); }
    virtual inline int deviceCount() const { return m_Devices.size(); }
    virtual inline int indexOfDevice(IDevice* D) const { return m_Devices.indexOf(D); }
};

class CDeviceList : public virtual CDeviceListBase
{
private:
    QRecursiveMutex mutex;
    QList<CDeviceList*> m_PolyDevices;
public:
    CAutomationPlayer AutomationPlayer;
    CDeviceList() : CDeviceListBase() {}
    virtual ~CDeviceList();
    void setPolyphony(const int Voices)
    {
        QMutexLocker locker(&mutex);
        while (m_PolyDevices.size() > Voices) delete m_PolyDevices.takeLast();
        while (m_PolyDevices.size() < Voices) m_PolyDevices.append(new CDeviceList);
    }
    // for the JackBar!!!
    IJack* addJack(IJack* Jack, int VoiceIndex = 0)
    {
        QMutexLocker locker(&mutex);
        return (VoiceIndex == 0) ? m_Jacks.addJack(Jack) : m_PolyDevices[VoiceIndex - 1]->addJack(Jack);
    }
    void updateParameter(const int DeviceIndex, const CParameter* parameter = nullptr)
    {
        QMutexLocker locker(&mutex);
        if (!parameter)
        {
            if (m_PolyDevices.size())
            {
                IDevice* D = device(DeviceIndex);
                QDomLiteElement e;
                D->serializeParameters(&e);
                //removeVisibleAttribute(&e);
                QDomLiteElement e1;
                m_PolyDevices.first()->device(DeviceIndex)->serializeParameters(&e1);
                //removeVisibleAttribute(&e1);
                if (!e.compare(&e1))
                {
                    for (CDeviceList* l : std::as_const(m_PolyDevices))
                    {
                        l->device(DeviceIndex)->unserializeParameters(&e);
                        l->hideForms();
                    }
                }
            }
        }
        else
        {
            for (CDeviceList* l : std::as_const(m_PolyDevices)) l->device(DeviceIndex)->parameter(parameter->Index)->setValue(parameter->Value);
        }
    }
    IDevice* createDevice(const instancefunc InstanceFunction, const int ID, QWidget* MainWindow)
    {
        QMutexLocker locker(&mutex);
        for (CDeviceList* l : std::as_const(m_PolyDevices)) l->createDevice(InstanceFunction, ID, MainWindow);
        return addDevice(InstanceFunction(),ID,MainWindow);
    }
    IDevice* addDevice(IDevice *device, const int index, QWidget* MainWindow, int VoiceIndex = 0)
    {
        QMutexLocker locker(&mutex);
        if (VoiceIndex == 0)
        {
            device->init(index,MainWindow);
            qDebug() << "addDevice" << device->deviceID() << m_PolyDevices.size();
            AutomationPlayer.addDevice(device);
            return CDeviceListBase::addDevice(device);
        }
        return m_PolyDevices[VoiceIndex - 1]->addDevice(device,index,MainWindow);
    }
    void disconnectDevice(const int index) { disconnectDevice(device(index)); }
    void disconnectDevice(IDevice* device)
    {
        QMutexLocker locker(&mutex);
        for (int i=0;i<device->jackCount();i++) disconnectJack(device->jack(i));
        if (!m_PolyDevices.isEmpty())
        {
            const int i = indexOfDevice(device);
            for (CDeviceList* l : std::as_const(m_PolyDevices)) l->disconnectDevice(l->device(i));
        }
    }
    void disconnectAll() { for (IDevice* d : std::as_const(m_Devices)) disconnectDevice(d); }
    void deleteDevice(const int index) { deleteDevice(device(index)); }
    void deleteDevice(IDevice* device)
    {
        QMutexLocker locker(&mutex);
        const int i = indexOfDevice(device);
        AutomationPlayer.removeDevice(device);
        qDebug() << "deleteDevice" << device->deviceID() << i << m_PolyDevices.size();
        disconnectDevice(device);
        m_Jacks.removeDevice(device);
        delete m_Devices.takeAt(i);
        for (CDeviceList* l : std::as_const(m_PolyDevices)) l->deleteDevice(i);
    }
    void removeDevice(const int index) { removeDevice(device(index)); }
    void removeDevice(IDevice* device)
    {
        QMutexLocker locker(&mutex);
        const int i = indexOfDevice(device);
        AutomationPlayer.removeDevice(device);
        disconnectDevice(device);
        m_Jacks.removeDevice(device);
        m_Devices.removeOne(device);
        for (CDeviceList* l : std::as_const(m_PolyDevices)) l->removeDevice(l->device(i));
    }
    void clear()
    {
        QMutexLocker locker(&mutex);
        while (!m_Devices.empty()) deleteDevice(m_Devices.last());
        for (CDeviceList* l : std::as_const(m_PolyDevices)) l->clear();
    }
    void unserializeDevice(const QDomLiteElement* parameters, IDevice* device)
    {
        QMutexLocker locker(&mutex);
        device->unserializeDevice(parameters);
        updateParameter(indexOfDevice(device));
    }
    void unserializeDevice(const QDomLiteElement* parameters, const int index) { unserializeDevice(parameters, device(index)); }
    bool moveDevice(IDevice* device, int move) {
        return moveDevice(indexOfDevice(device), move);
    }
    bool moveDevice(int index, int move) {
        QMutexLocker locker(&mutex);
        if (index < 0) return false;
        if (index > m_Devices.size() -1) return false;
        if (move == 0) return false;
        //int newIndex = index + move;
        int newIndex = std::clamp<int>(index + move,0,m_Devices.size()-1);
        //newIndex = qMax(0, newIndex);
        //newIndex = qMin(devices.size() -1, newIndex);
        if (newIndex == index) return false;
        IDevice* temp = m_Devices.takeAt(index);
        m_Devices.insert(newIndex, temp);
        for (CDeviceList* l : std::as_const(m_PolyDevices)) l->moveDevice(index, move);
        return true;
    }
    inline bool connect(const QString& J1, const QString& J2)
    {
        QMutexLocker locker(&mutex);
        if (!m_Jacks.contains(J1)) return false;
        if (!m_Jacks.contains(J2)) return false;
        for (CDeviceList* l : std::as_const(m_PolyDevices)) l->connect(J1, J2);
        return m_Jacks[J1]->connectTo(m_Jacks[J2]);
    }
    bool disconnect(const QString& J1, const QString& J2)
    {
        QMutexLocker locker(&mutex);
        if (!m_Jacks.contains(J1)) return false;
        if (!m_Jacks.contains(J2)) return false;
        for (CDeviceList* l : std::as_const(m_PolyDevices)) l->disconnect(J1, J2);
        return m_Jacks[J1]->disconnectFrom(m_Jacks[J2]);
    }
    void disconnectJack(const QString& J)
    {
        QMutexLocker locker(&mutex);
        if (!m_Jacks.contains(J)) return;
        m_Jacks.disconnectFrom(m_Jacks[J]);
        for (CDeviceList* l : std::as_const(m_PolyDevices)) l->disconnectJack(J);
    }
    void disconnectJack(IJack* j) { disconnectJack(j->jackID()); }
    void play(const bool FromStart)
    {
        qDebug() << "DeviceList play";
        QMutexLocker locker(&mutex);
        AutomationPlayer.play(FromStart);
        for (IDevice* d : std::as_const(m_Devices))
        {
            qDebug() << d->deviceID() << "play";
            d->play(FromStart);
        }
        for (CDeviceList* l : std::as_const(m_PolyDevices)) l->play(FromStart);
    }
    void pause()
    {
        QMutexLocker locker(&mutex);
        AutomationPlayer.pause();
        for (IDevice* d : std::as_const(m_Devices)) d->pause();
        for (CDeviceList* l : std::as_const(m_PolyDevices)) l->pause();
    }
    void tick()
    {
        for (IDevice* d : std::as_const(m_Devices)) d->tick();
        for (CDeviceList* l : std::as_const(m_PolyDevices)) l->tick();
        AutomationPlayer.tick();
    }
    void skip(const ulong64 samples)
    {
        QMutexLocker locker(&mutex);
        AutomationPlayer.skip(samples);
        for (IDevice* d : std::as_const(m_Devices)) d->skip(samples);
        //for (IDevice* d : qAsConst(m_Devices)) d->play(false);
        for (CDeviceList* l : std::as_const(m_PolyDevices)) l->skip(samples);
    }
    /*
    ulong currentmSec() const {
        return AutomationPlayer.currentmSec();
    }
    ulong currentSample() const {
        return AutomationPlayer.currentSample();
    }
    bool isPlaying() const {
        return AutomationPlayer.isPlaying();
    }
*/
    void hideForms()
    {
        QMutexLocker locker(&mutex);
        for (CDeviceList* l : std::as_const(m_PolyDevices)) l->hideForms();
        for (IDevice* d : std::as_const(m_Devices)) d->hideForm();
    }
    void updateAutomationPlayer() {
        AutomationPlayer.updatePlaylist();
    }
    void cascadeForms(QPoint& p)
    {
        QMutexLocker locker(&mutex);
        for (CDeviceList* l : std::as_const(m_PolyDevices))
        {
            QPoint p1(p);
            l->cascadeForms(p1);
        }
        for (IDevice* d : std::as_const(m_Devices)) d->cascadeForm(p);
    }
    void execute(const int index, bool show = true) { execute(device(index),show); }
    void execute(IDevice* device, bool show = true)
    {
        QMutexLocker locker(&mutex);
        if (!m_PolyDevices.isEmpty())
        {
            const int i = indexOfDevice(device);
            for (CDeviceList* l : std::as_const(m_PolyDevices)) l->device(i)->execute(false);
        }
        device->execute(show);
    }
};

#endif // CDEVICELIST_H
