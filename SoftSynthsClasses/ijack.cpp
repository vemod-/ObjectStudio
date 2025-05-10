#include "ijack.h"

CMonoBuffer CInJack::m_NullBufferMono = CMonoBuffer(nullptr);
CStereoBuffer CInJack::m_NullBufferStereo = CStereoBuffer(nullptr);

IJack::~IJack() {
    if (attachMode & Audio)
    {
        QMutexLocker locker(&mutex);
        delete audioBuffer;
        //qDebug() << "AudioBuffer Deleted" << name << owner;
    }
    m_OwnerDevice=nullptr;
    //qDebug() << "Jack Deleted " << owner << name;
}

IJack* IJack::createInsideJack(int ProcIndex, IDeviceBase *DeviceClass)
{
    QMutexLocker locker(&mutex);
    return (isInJack()) ? static_cast<IJack*>(new COutJack(name(),"This",attachMode,IJack::Out,DeviceClass,ProcIndex)) :
                          new CInJack(name(),"This",attachMode,IJack::In,DeviceClass);
}

bool COutJack::connectToIn(CInJack *InJack) { return InJack->connectToOut(this); }

bool COutJack::disconnectFromIn(CInJack *InJack) { return InJack->disconnectFromOut(this); }

bool COutJack::isConnectedToIn(const CInJack *InJack) const { return InJack->isConnectedToOut(const_cast<COutJack*>(this)); }

bool COutJack::connectTo(IJack *Jack)
{
    QMutexLocker locker(&mutex);
    return (Jack->isInJack()) ? connectToIn(dynamic_cast<CInJack*>(Jack)) : false;
}

bool COutJack::disconnectFrom(IJack *Jack)
{
    QMutexLocker locker(&mutex);
    return (Jack->isInJack()) ? disconnectFromIn(dynamic_cast<CInJack*>(Jack)) : false;
}

bool COutJack::isConnectedTo(const IJack *Jack) const
{
    return (Jack->isInJack()) ? isConnectedToIn(dynamic_cast<const CInJack*>(Jack)) : false;
}

CInJack::~CInJack() {
    QMutexLocker locker(&mutex);
    for (COutJack* j : std::as_const(m_OutJacks)) disconnectFromOut(j);
    if (m_MIDIBuffer != nullptr) delete m_MIDIBuffer;
}
