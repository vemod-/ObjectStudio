#ifndef CAUTOMATIONPLAYER_H
#define CAUTOMATIONPLAYER_H

#include "softsynthsdefines.h"
#include "cmseccounter.h"
#include "idevice.h"

class CAutomationPlayer : public ITicker
{
public:
    CAutomationPlayer(){}
    ~CAutomationPlayer(){
        clear();
    }
    void clear() {
        //m_EventLists.clear();
        for (CParameter* p : std::as_const(m_Parameters)) p->events.clear();
        m_Parameters.clear();
    }
    /*
    void unserialize(const QDomLiteElement* XML)
    {
        //for (CParameterEventList& l : m_EventLists)  l.clear();
        m_EventLists.clear();
        for (QDomLiteElement* e : XML->childElements) {
            m_EventLists[e->attribute(ParameterIDAttribute)].push_back(e);
        }
        for (CParameterEventList& l : m_EventLists) {
            sortEvents(l);
        }
    }
    void serialize(QDomLiteElement* xml) const
    {
        for (const CParameterEventList& l : m_EventLists) {
            for (const CParameterEvent& e : l) e.serialize(xml->appendChild("Event"));
        }
    }
    */
    CParameterEventList& parameterEvents(const QString& parameterID) {
        return m_Parameters[parameterID]->events;
    }
    CParameterEventList& parameterEvents(const QString& deviceID, const QString& ID) {
        return parameterEvents(createId(deviceID,ID));
    }
    CParameterEventList parameterPlayEvents(const QString& parameterID) {
        QMutexLocker locker(&mutex);
        CParameterEventList& l=m_Parameters[parameterID]->events;
        if (l.empty()) return CParameterEventList();
        if ((m_Parameters[parameterID]->Type==CParameter::dB) || (m_Parameters[parameterID]->Type==CParameter::Percent) || (m_Parameters[parameterID]->Type==CParameter::Numeric)) {
            CParameterEventList l1;
            for (uint i = 0; i < l.size()-1; i++)
            {
                CParameterEvent& e=l[i];
                CParameterEvent& e1=l[i+1];
                int vSign=sgn(e1.value-e.value);
                l1.push_back(e);
                if (vSign)
                {
                    int vDelta=e.value+vSign;
                    ldouble inc=ldouble(e1.time-e.time) / ldouble(qAbs(e1.value-e.value));
                    while (inc < 50) {
                        inc *= 2;
                        vSign *= 2;
                    }
                    //qDebug() << e.time << e.value << e1.time << e1.value << vDelta << vSign;
                    //for (ldouble deltaTime = e.time+inc; deltaTime < e1.time; deltaTime+=inc) {
                    ldouble deltaTime = e.time + inc;
                    for (int i = 1; deltaTime < e1.time; i++) {
                        deltaTime = e.time + (i*inc);
                        //qDebug() << ulong64(deltaTime) << vDelta << parameterID;
                        l1.push_back(CParameterEvent(deltaTime,vDelta,parameterID));
                        vDelta+=vSign;
                    }
                }
            }
            l1.push_back(l.back());
            return l1;
        }
        CParameterEventList l1;
        for (const CParameterEvent& p : l) l1.push_back(CParameterEvent(p));
        return l1;
    }
    CParameterEventList parameterPlayEvents(const QString& deviceID, const QString& ID) {
        return parameterPlayEvents(createId(deviceID,ID));
    }
    void updatePlaylist() {
        QMutexLocker locker(&mutex);
        m_PlayList.clear();
        for (auto it = m_Parameters.constKeyValueBegin(); it != m_Parameters.constKeyValueEnd(); it++) {
        //for (const QString &k : m_Parameters.keys()) {
            for (const CParameterEvent& e : parameterPlayEvents(it->first)) m_PlayList.push_back(e);
        }
        sortEvents(m_PlayList);
        m_NextEvent=0;
        m_NextTime=0;
        for (uint i = 0; i < m_PlayList.size(); i++) {
            const CParameterEvent& e = m_PlayList.at(i);
            if (e.time > m_Counter.currentmSec()) {
                m_NextEvent = i;
                m_NextTime = e.time;
                break;
            }
        }
    }
    void play(const bool FromStart) {
        if (FromStart) m_Counter.reset();
        updatePlaylist();
        m_BufferCount = m_Counter.currentBuffer() % 5;
        m_Playing = true;
    }
    void pause() {
        m_Playing = false;
    }
    void skip(const ulong64 samples) {
        m_Counter.reset();
        m_Counter.skip(samples);
        updatePlaylist();
        m_BufferCount = m_Counter.currentBuffer() % 5;
    }
    void tick() {
        if (m_Playing)
        {
            m_Counter.skipBuffer();
            if (m_BufferCount==0)
            {
                automate();
            }
            else
            {
                m_BufferCount++;
                if (m_BufferCount>5) m_BufferCount = 0;
            }
        }
    }
    void automate() {
        //QMutexLocker locker(&mutex);
        while (m_PlayList.size() > m_NextEvent)
        {
            if (!m_Playing) return;
            if (m_NextTime <= m_Counter.currentmSec()) {
                if (const CParameterEvent* e = &m_PlayList.at(m_NextEvent)) {
                //qDebug() << "Parameter" << e.id << e.value << e.time << m_NextTime << m_Counter.currentmSec();
                    if (CParameter* p = m_Parameters[e->id]) p->setValue(e->value);
                }
                m_NextEvent++;
                if (m_PlayList.size() > m_NextEvent) m_NextTime=m_PlayList.at(m_NextEvent).time;
            }
            else
            {
                return;
            }
        }
    }
    QStringList parameterIds() { return m_Parameters.keys(); }
    void addDevice(IDevice* d) {
        for (int i = 0; i < d->parameterCount(); i++) appendParameter(d->parameter(i),d->deviceID());
        if (m_Playing) updatePlaylist();
    }
    void removeDevice(IDevice* d) {
        for (int i = 0; i < d->parameterCount(); i++) removeParameter(d->parameter(i),d->deviceID());
        if (m_Playing) updatePlaylist();
    }
    /*
    ulong currentmSec() const {
        return m_Counter.currentmSec();
    }
    ulong currentSample() const {
        return m_Counter.currentSample();
    }
    bool isPlaying() const {
        return m_Playing;
    }
*/
private:
    QRecursiveMutex mutex;
    QMap<QString,CParameter*> m_Parameters;
    CParameterEventList m_PlayList;
    CSampleCounter m_Counter;
    uint m_BufferCount=0;
    bool m_Playing = false;
    uint m_NextEvent=0;
    ulong64 m_NextTime=0;
    QString createId(const QString& deviceID, const QString& parameterName) {
        return deviceID + " " + parameterName;
    }
    void clearParameters() { m_Parameters.clear(); }
    void appendParameter(CParameter* p, const QString& deviceID) { m_Parameters[createId(deviceID, p->Name)]=p; }
    void removeParameter(CParameter* p, const QString& deviceID) { m_Parameters.remove(createId(deviceID, p->Name)); }
    void sortEvents(CParameterEventList& l) {
        std::sort(l.begin(),l.end(), [] (const CParameterEvent& a, const CParameterEvent& b){ return (a.time < b.time); });
    }
    int sgn(int v) {
        if (v>0) return 1;
        if (v<0) return -1;
        return 0;
    }
};

#endif // CAUTOMATIONPLAYER_H
