#ifndef CMIDIBUFFER_H
#define CMIDIBUFFER_H

#include <memory.h>
#include <QList>
#include <vector>
#include <QtEndian>
#include "softsynthsdefines.h"
;
#pragma pack(push,1)

class CMIDIEvent
{
public:
    inline CMIDIEvent(byte*& data, const byte* limit)
        : m_Message(data), m_Data(data+1), m_DataSize(0)
    {
        if (*data==0xFF) //Meta event
        {
            m_Data++;
        }
        else if (isSysEx())
        {
            while (++data < limit)
            {
                if ((*data >= 0x80) & (*data != 0xF7)) break;
                m_DataSize++;
                if (*data == 0xF7) break;
            }
        }
        else
        {
            while (++data < limit)
            {
                if (*data >= 0x80) break;
                m_DataSize++;
            }
        }
    }
    inline CMIDIEvent(const byte* message, const byte* data, const ushort dataSize)
        : m_Message(message), m_Data(data), m_DataSize(dataSize) {}
    inline byte message() const { return *m_Message; }
    inline byte channel() const { return *m_Message & 0x0F; }
    inline byte command() const { return *m_Message & 0xF0; }
    inline byte data(int i) const { return m_Data[i]; }
    inline const byte* dataPtr() const { return m_Data; }
    inline uint dataSize() const { return m_DataSize; }
    inline const byte* memPtr() const { return m_Message; }
    inline uint memSize() const {return (m_Data + m_DataSize) - m_Message; }
    inline const std::vector<byte> toVector() const
    {
        std::vector<byte> v;
        const uint l = memSize();
        v.resize(l);
        memcpy(v.data(),memPtr(),l);
        return v;
    }
    inline bool dataEmpty() const { return (m_DataSize == 0); }
    inline bool isNoteOff() const { return (command()==0x80); }
    inline bool isNoteOn() const { return (command()==0x90); }
    inline bool isAftertouch() const { return (command()==0xA0); }
    inline bool isController() const { return (command()==0xB0); }
    inline bool isPatchChange() const { return (command()==0xC0); }
    inline bool isChannelPressure() const { return (command()==0xD0); }
    inline bool isPitchBend() const { return (command()==0xE0); }
    inline bool isSystem() const { return (command()==0xF0); }
    inline bool isSysEx() const { return ((*m_Message==0xF0) | (*m_Message==0xF7)); }
    inline bool isMeta() const { return (*m_Message==0xFF); }
    inline byte metaType() const { return *(m_Message+1); }
    inline void transpose(const int transpose) const {
        if (transpose) {
            byte* d = const_cast<byte*>(m_Data);
            if (isNoteOff() || isNoteOn()) *d += transpose;
            if (isController())
            {
                if (*d == 84) *(d+1) += transpose;
            }
        }
    }
    byte velocity() const { return m_Data[1]; }
    inline void setVelocity(const byte v) {
        byte* d = const_cast<byte*>(m_Data);
        if (isNoteOff() || isNoteOn()) *(d+1) = v;
    }
    inline bool matches(const CMIDIEvent& other) const {
        if (*m_Message != other.message()) return false;
        if (*m_Message == 0xFF) if (*(m_Message+1) != other.metaType()) return false;
        if (m_DataSize != other.dataSize()) return false;
        if (memcmp(m_Data,other.dataPtr(),m_DataSize)) return false;
        return true;
    }
private:
    const byte* m_Message;
    const byte* m_Data;
    ushort m_DataSize;
};

class CMIDIEventList
{
public:
    inline CMIDIEventList(byte* data, const uint dataSize)
    {
        if (!data) return;
        const byte* limit=data+dataSize;
        while (data < limit)
        {
            if (*data >= 0x80)
            {
                v.push_back(new CMIDIEvent(data,limit));
            }
            else
            {
                data++;
            }
        }
    }
    ~CMIDIEventList()
    {
        qDeleteAll(v);
    }
    inline CMIDIEvent* at(const uint index) const { return v[index]; }
    inline CMIDIEvent* operator [] (const uint index) const { return v[index]; }
    inline uint size() const { return uint(v.size()); }
    inline bool isEmpty() const { return v.empty(); }
private:
    std::vector<CMIDIEvent*> v;
};

namespace MIDIBuffer
{
const uint MIDIBufferSize=1000;
}

class CMIDIBuffer
{
private:
    byte* m_Buffer;
    uint m_BufferSize;
    uint m_Size;
    inline void expand(const uint dataSize)
    {
        while (m_Size + dataSize >= m_BufferSize)
        {
            byte* temp=new byte[m_BufferSize+MIDIBuffer::MIDIBufferSize];
            memcpy(temp,m_Buffer,m_BufferSize);
            delete [] m_Buffer;
            m_Buffer=temp;
            m_BufferSize+=MIDIBuffer::MIDIBufferSize;
        }
    }
public:
    inline CMIDIBuffer()
    {
        m_BufferSize=MIDIBuffer::MIDIBufferSize;
        m_Buffer=new byte[m_BufferSize];
        clear();
    }
    inline ~CMIDIBuffer() { delete [] m_Buffer; }
    inline void append(const byte message, const byte data)
    {
        expand(2);
        m_Buffer[m_Size++]=message;
        m_Buffer[m_Size++]=data;
    }
    inline void append(const byte message, const byte data1, const byte data2)
    {
        expand(3);
        m_Buffer[m_Size++]=message;
        m_Buffer[m_Size++]=data1;
        m_Buffer[m_Size++]=data2;
    }
    inline void append(const byte* data, const uint dataSize)
    {
        expand(dataSize);
        memcpy(m_Buffer+m_Size,data,dataSize);
        m_Size+=dataSize;
    }
    inline void append(const byte message, const byte* data, const uint dataSize)
    {
        expand(dataSize+1);
        m_Buffer[m_Size++]=message;
        append(data,dataSize);
    }
    inline void append(const CMIDIEvent& e)
    {
        append(e.message(),e.dataPtr(),e.dataSize());
    }
    inline void append(const std::vector<byte>* v)
    {
        const uint s=size();
        expand(s);
        memcpy(m_Buffer+m_Size,v->data(),s);
        m_Size+=s;
    }
    inline void fromVector(const std::vector<byte>* v)
    {
        clear();
        append(v);
    }
    inline void clear()
    {
        m_Size=0;
    }
    inline bool isEmpty() const { return (m_Size == 0); }
    inline void append(const CMIDIBuffer* MB)
    {
        const uint s=MB->size();
        expand(s);
        memcpy(m_Buffer+m_Size,MB->data(),s);
        m_Size+=s;
    }
    inline CMIDIBuffer* fromBufferList(const std::vector<CMIDIBuffer*> BufferList)
    {
        clear();
        for (const CMIDIBuffer* b : BufferList) append(b);
        return this;
    }
    inline const std::vector<byte> toVector() const
    {
        std::vector<byte> v;
        fillVector(&v);
        return v;
    }
    inline void fillVector(std::vector<byte> *v) const
    {
        v->resize(m_Size);
        memcpy(v->data(),m_Buffer,m_Size);
    }
    inline byte* data() const { return m_Buffer; }
    inline uint size() const { return m_Size; }
    const CMIDIEventList eventList() const
    {
        return CMIDIEventList(m_Buffer,m_Size);
    }
};

#pragma pack(pop)

#endif // CMIDIBUFFER_H
