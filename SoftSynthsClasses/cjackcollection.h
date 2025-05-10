#ifndef CJACKCOLLECTION_H
#define CJACKCOLLECTION_H

#include "idevice.h"

class CJackCollection
{
private:
    QVector<IJack*> m_Jacks;
    QVector<QString> m_Keys;
    QVector<CInJack*> m_InJacks;
    QVector<COutJack*> m_OutJacks;
public:
    CJackCollection() {}
    ~CJackCollection() {}
    IJack* addJack(IJack* Jack)
    {
        m_Jacks.push_back(Jack);
        m_Keys.append(Jack->jackID());
        if (Jack->isInJack()) m_InJacks.push_back(static_cast<CInJack*>(Jack));
        else m_OutJacks.push_back(static_cast<COutJack*>(Jack));
        return Jack;
    }
    void removeJack(const QString& Key)
    {
        if (IJack* j = item(Key))
        {
            (j->isInJack()) ? m_InJacks.removeOne(static_cast<CInJack*>(j)) : m_OutJacks.removeOne(static_cast<COutJack*>(j));
            m_Jacks.removeOne(j);
            m_Keys.removeOne(Key);
        }
    }
    void addDevice(const IDevice* Device)
    {
        for (int i=0;i<Device->jackCount();i++) addJack(Device->jack(i));
    }
    void removeDevice(const IDevice* Device)
    {
        for (int i=0;i<Device->jackCount();i++) removeJack(Device->jackID(i));
    }
    void disconnectFrom(IJack* Jack)
    {
        for (IJack* j : std::as_const(m_Jacks)) j->disconnectFrom(Jack);
    }
    inline IJack* item(const QString& Key) const
    {
        const int Index=m_Keys.indexOf(Key);
        return (Index > -1) ? m_Jacks[Index] : nullptr;
    }
    inline bool contains(const QString& Key) const
    {
        return m_Keys.contains(Key);
    }
    inline IJack* at(const int Index) const
    {
        return (Index < m_Jacks.size()) ? m_Jacks[Index] : nullptr;
    }
    inline int size() const { return m_Jacks.size(); }
    void clear()
    {
        m_Jacks.clear();
        m_Keys.clear();
    }
    inline IJack* operator [] (const int Index) const
    {
        return m_Jacks[Index];
    }
    inline IJack* operator [] (const QString& Key) const
    {
        const int Index=m_Keys.indexOf(Key);
        return (Index > -1) ? m_Jacks[Index] : nullptr;
    }
    inline int inJackCount() const { return m_InJacks.size(); }
    inline int outJackCount() const { return m_OutJacks.size(); }
    inline CInJack* inJack(const int Index) const { return m_InJacks[Index]; }
    inline COutJack* outJack(const int Index) const { return m_OutJacks[Index]; }
};

#endif // CJACKCOLLECTION_H
