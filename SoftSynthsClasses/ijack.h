#ifndef IJACK_H
#define IJACK_H

#include <QVector>
#include "ijackbase.h"
#include "idevicebase.h"
#include "caudiobuffer.h"
#include "cmidibuffer.h"
;
#pragma pack(push,1)

class IJack : public IJackBase
{
protected:
    IDeviceBase* m_OwnerDevice;
    QRecursiveMutex mutex;
    QString m_Name;
    QString m_Owner;
    QString m_JackID;
public:
    inline IJack(const QString& sName,const QString& sOwner,AttachModes tAttachMode,Directions tDirection,IDeviceBase* OwnerClass)
        : IJackBase(tAttachMode,tDirection), m_OwnerDevice(OwnerClass), /*m_BufferSize(CPresets::presets().BufferSize),*/ m_Name(sName), m_Owner(sOwner), audioBuffer(nullptr)
    {
        m_JackID=m_Owner + " " + m_Name;
        //qDebug() << "Jack Created " << sOwner << sName;
        if (attachMode==Wave)
        {
            QMutexLocker locker(&mutex);
            audioBuffer=new CMonoBuffer;
            //qDebug() << "MonoBuffer Created" << sName << sOwner;
        }
        if (attachMode==Stereo)
        {
            QMutexLocker locker(&mutex);
            audioBuffer=new CStereoBuffer;
            //qDebug() << "StereoBuffer Created" << sName << sOwner;
        }
    }
    virtual ~IJack();
    inline const QString name() const { return m_Name; }
    inline const QString owner() const { return m_Owner; }
    CAudioBuffer* audioBuffer;
    inline const QString jackID() const { return m_JackID; }
    virtual bool connectTo(IJack* /*Jack*/) { return false; }
    virtual bool disconnectFrom(IJack* /*Jack*/) { return false; }
    virtual bool isConnectedTo(const IJack* /*Jack*/) const { return false; }
    inline bool canConnectTo(const IJack* Jack) const {
        if (!Jack) return false;
        if (Jack == this) return false;
        if (Jack->owner() == m_Owner) return false;
        if (Jack->direction == direction) return false;
        if (Jack->attachMode & attachMode) return true;
        return false;
    }
    IJack* createInsideJack(int ProcIndex, IDeviceBase *DeviceClass);
};

class CJackList
{
public:
    inline CJackList() {
        size=0;
        capacity=5;
        data=new IJack*[capacity];
    }
    inline ~CJackList() { delete [] data; }
    inline bool process(IJack* Jack) {
        if (size==0) return setFirst(Jack);
        for (uint i=0;i<size;i++) if (data[i]==Jack) return setFirst(Jack);
        append(Jack);
        return false;
    }
private:
    IJack** data;
    uint size;
    uint capacity;
    inline void append(IJack* Jack) {
        if (size >= capacity)
        {
            capacity+=5;
            IJack** temp=new IJack*[capacity];
            copyMemory(temp,data,size*sizeof(IJack*));
            delete [] data;
            data=temp;
        }
        data[size++]=Jack;
    }
    inline bool setFirst(IJack* Jack) {
        data[0]=Jack;
        size=1;
        return true;
    }
};

class CInJack;

class COutJack : public IJack
{
public:
    inline COutJack(const QString& sName,const QString& sOwner,AttachModes tAttachMode,Directions tDirection,IDeviceBase* OwnerClass,const int tProcIndex) :IJack(sName,sOwner,tAttachMode,tDirection,OwnerClass), procIndex(tProcIndex), m_ConnectCount(0), m_LastGet(0), m_LastGetP(nullptr), m_LastGetA(nullptr) {}
    int procIndex;
    inline void addConnection() { m_ConnectCount++; }
    inline void removeConnection() { m_ConnectCount--; }
    inline float getNext(IJack* InJack) {
        if (m_ConnectCount==1) return m_OwnerDevice->getNext(procIndex);
        if (m_InJackCalls.process(InJack)) m_LastGet=m_OwnerDevice->getNext(procIndex);
        return m_LastGet;
    }
    inline CMIDIBuffer* getNextP(IJack* InJack) {
        if (m_ConnectCount==1) return m_OwnerDevice->getNextP(procIndex);
        if (m_InJackCalls.process(InJack)) m_LastGetP=m_OwnerDevice->getNextP(procIndex);
        return m_LastGetP;
    }
    inline CAudioBuffer* getNextA(IJack* InJack) {
        if (m_ConnectCount==1) return m_OwnerDevice->getNextA(procIndex);
        if (m_InJackCalls.process(InJack)) m_LastGetA=m_OwnerDevice->getNextA(procIndex);
        return m_LastGetA;
    }
    bool connectToIn(CInJack* InJack);
    bool disconnectFromIn(CInJack* InJack);
    bool isConnectedToIn(const CInJack* InJack) const;
    bool connectTo(IJack* Jack);
    bool disconnectFrom(IJack* Jack);
    bool isConnectedTo(const IJack* Jack) const;
    int connectCount() const { return m_ConnectCount; }
private:
    int m_ConnectCount;
    float m_LastGet;
    CMIDIBuffer* m_LastGetP;
    CAudioBuffer* m_LastGetA;
    CJackList m_InJackCalls;
};

class CInJack : public IJack
{
public:
    inline CInJack(const QString& sName,const QString& sOwner,AttachModes tAttachMode,Directions tDirection,IDeviceBase* OwnerClass) :IJack(sName,sOwner,tAttachMode,tDirection,OwnerClass), m_LastGetNext(0), m_OutJackCount(0), m_MIDIBuffer(nullptr) {
        //if (m_NullBufferMono.isValid()) m_NullBufferMono.makeNull();
        //if (m_NullBufferStereo.isValid()) m_NullBufferStereo.makeNull();
    }
    virtual ~CInJack();
    inline float getNext() {
        if (m_OutJackCount==1) return m_firstJack->getNext(this);
        if (m_OutJackCount==0) return 0;
        m_LastGetNext=0;
        for (COutJack* j : std::as_const(m_OutJacks)) m_LastGetNext+=j->getNext(this);
        return m_LastGetNext;
    }
    inline CMIDIBuffer* getNextP() {
        if (m_OutJackCount==1) return m_firstJack->getNextP(this);
        if (m_OutJackCount==0) return nullptr;

        std::vector<CMIDIBuffer*> MIDIBuffers;
        for (COutJack* j : std::as_const(m_OutJacks)) {
            CMIDIBuffer* MB=j->getNextP(this);
            if (MB != nullptr) if (!MB->isEmpty()) MIDIBuffers.push_back(MB);
        }
        if (MIDIBuffers.empty()) return nullptr;
        if (MIDIBuffers.size()==1) return MIDIBuffers.front();
        if (m_MIDIBuffer == nullptr) m_MIDIBuffer=new CMIDIBuffer;
        return m_MIDIBuffer->fromBufferList(MIDIBuffers);
    }
    inline CAudioBuffer* getNextA() {
        if (m_OutJackCount==1) {
            const CAudioBuffer* b = m_firstJack->getNextA(this);
            if (!b) return nullBuffer();
            audioBuffer->writeBuffer(b,m_firstJack->attachMode);
            return audioBuffer;
        }
        if (m_OutJackCount==0) return nullBuffer();
        int FetchCount=0;
        for (COutJack* j : std::as_const(m_OutJacks)) audioBuffer->updateBuffer(j->getNextA(this),j->attachMode,FetchCount++ == 0);
        if (FetchCount==0) return nullBuffer();
        audioBuffer->multiplyBuffer(m_MixFactor);
        return audioBuffer;
    }
    inline int outJackCount() const { return m_OutJackCount; }
    inline COutJack* outJack(const int Index) const { return m_OutJacks[Index]; }
    bool connectToOut(COutJack* OutJack)
    {
        if (!OutJack) return false;
        if (!canConnectTo(OutJack)) return false;
        if (isConnectedToOut(OutJack)) return false;
        QMutexLocker locker(&mutex);
        m_OutJacks.append(OutJack);
        OutJack->addConnection();
        m_firstJack=m_OutJacks.first();
        m_MixFactor=mixFactorf(++m_OutJackCount);
        m_OwnerDevice->connectionChanged();
        return true;
    }
    bool disconnectFromOut(COutJack* OutJack)
    {
        if (!OutJack) return false;
        if (!isConnectedToOut(OutJack)) return false;
        QMutexLocker locker(&mutex);
        if (!m_OutJacks.removeOne(OutJack)) return false;
        OutJack->removeConnection();
        if (!m_OutJacks.isEmpty()) m_firstJack=m_OutJacks.first();
        m_MixFactor=mixFactorf(--m_OutJackCount);
        m_OwnerDevice->connectionChanged();
        return true;
    }
    bool isConnectedToOut(const COutJack* OutJack) const { return m_OutJacks.contains(const_cast<COutJack*>(OutJack)); }
    bool connectTo(IJack* Jack) {
        QMutexLocker locker(&mutex);
        return (Jack->isOutJack()) ? connectToOut(static_cast<COutJack*>(Jack)) : false;
    }
    bool disconnectFrom(IJack* Jack) {
        QMutexLocker locker(&mutex);
        return (Jack->isOutJack()) ? disconnectFromOut(static_cast<COutJack*>(Jack)) : false;
    }
    bool isConnectedTo(const IJack* Jack) const {
        return (Jack->isOutJack()) ? isConnectedToOut(static_cast<const COutJack*>(Jack)) : false;
    }
private:
    QVector<COutJack*> m_OutJacks;
    float m_LastGetNext;
    int m_OutJackCount;
    COutJack* m_firstJack;
    float m_MixFactor;
    CMIDIBuffer* m_MIDIBuffer;
    static CMonoBuffer m_NullBufferMono;
    static CStereoBuffer m_NullBufferStereo;
    inline CAudioBuffer* nullBuffer() const {
        return (attachMode==AttachModes::Stereo) ? static_cast<CAudioBuffer*>(&m_NullBufferStereo) :
                                                   static_cast<CAudioBuffer*>(&m_NullBufferMono);
    }
};

#pragma pack(pop)

#endif // IJACK_H
