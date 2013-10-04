#ifndef CMIDIBUFFER_H
#define CMIDIBUFFER_H

#include <memory.h>
#include <QVarLengthArray>
#include <QList>
#include <vector>

typedef unsigned char BYTE;

namespace MIDIBuffer
{
const unsigned int MIDIBufferSize=1000;
}

class CMIDIEvent
{
public:
    CMIDIEvent(){}
    BYTE message;
    BYTE channel;
    BYTE command;
    QVarLengthArray<BYTE> data;
};

typedef QList<CMIDIEvent> MIDIEventList;

class CMIDIBuffer
{
private:
    BYTE* Buffer;
    unsigned int Size;
    unsigned int PopCount;
    unsigned int PushCount;
    unsigned int ReadCount;
    short Read(void);
    void StartRead(void);
    bool IsRead(void) const;
    short Pop(void);
    void Expand(unsigned int dataSize);
public:
    CMIDIBuffer();
    ~CMIDIBuffer();
    void Push(const BYTE MIDIData);
    void Push(const BYTE message, const BYTE data);
    void Push(const BYTE message, const BYTE data1, const BYTE data2);
    void Push(const BYTE* data, const unsigned int dataSize);
    void Push(const BYTE message, const BYTE* data, const unsigned int dataSize);
    void Reset(void);
    bool IsEmpty(void) const;
    void Append(CMIDIBuffer* MB);
    const MIDIEventList Events();
    const std::vector<BYTE> MIDIData();
};

#endif // CMIDIBUFFER_H
