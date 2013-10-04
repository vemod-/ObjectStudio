#include "cmidibuffer.h"

CMIDIBuffer::CMIDIBuffer()
{
    Buffer=new BYTE[MIDIBuffer::MIDIBufferSize];
    Size=MIDIBuffer::MIDIBufferSize;
    Reset();
}

CMIDIBuffer::~CMIDIBuffer()
{
    delete [] Buffer;
}

void CMIDIBuffer::Expand(unsigned int dataSize)
{
    while (PushCount+dataSize >= Size)
    {
        BYTE* temp=new BYTE[Size+MIDIBuffer::MIDIBufferSize];
        memcpy(temp,Buffer,Size);
        delete [] Buffer;
        Buffer=temp;
        Size+=MIDIBuffer::MIDIBufferSize;
    }
}

void CMIDIBuffer::Push(const BYTE MIDIData)
{
    Expand(1);
    Buffer[PushCount++]=MIDIData;
}

void CMIDIBuffer::Push(const BYTE message, const BYTE data)
{
    Expand(2);
    Buffer[PushCount++]=message;
    Buffer[PushCount++]=data;
}

void CMIDIBuffer::Push(const BYTE message, const BYTE data1, const BYTE data2)
{
    Expand(3);
    Buffer[PushCount++]=message;
    Buffer[PushCount++]=data1;
    Buffer[PushCount++]=data2;
}

void CMIDIBuffer::Push(const BYTE *data, const unsigned int dataSize)
{
    Expand(dataSize);
    memcpy(&Buffer[PushCount],data,dataSize);
    PushCount+=dataSize;
}

void CMIDIBuffer::Push(const BYTE message, const BYTE *data, const unsigned int dataSize)
{
    Expand(dataSize+1);
    Buffer[PushCount++]=message;
    Push(data,dataSize);
}

short CMIDIBuffer::Pop()
{
    if (PushCount != PopCount)
    {
        BYTE P = Buffer[PopCount];
        PopCount++;
        if (PopCount>=Size)
        {
            PopCount=0;
        }
        return P;
    }
    else
    {
        return -1;
    }
}

void CMIDIBuffer::StartRead()
{
    ReadCount=PopCount;
}

short CMIDIBuffer::Read()
{
    if (PushCount != ReadCount)
    {
        BYTE P = Buffer[ReadCount];
        ReadCount++;
        if (ReadCount>=Size)
        {
            ReadCount=0;
        }
        return P;
    }
    else
    {
        return -1;
    }
}

void CMIDIBuffer::Reset()
{
    if (Size > MIDIBuffer::MIDIBufferSize)
    {
        delete [] Buffer;
        Buffer=new BYTE[MIDIBuffer::MIDIBufferSize];
        Size=MIDIBuffer::MIDIBufferSize;
    }
    PopCount=0;
    PushCount=0;
    ReadCount=0;
}

bool CMIDIBuffer::IsEmpty() const
{
    return (PushCount == PopCount);
}

bool CMIDIBuffer::IsRead() const
{
    return (PushCount == ReadCount);
}

void CMIDIBuffer::Append(CMIDIBuffer *MB)
{
    MB->StartRead();
    short lTemp=MB->Read();
    while (lTemp>-1)
    {
        Push(lTemp);
        lTemp=MB->Read();
    }
}

const MIDIEventList CMIDIBuffer::Events()
{
    QList<CMIDIEvent> li;
    StartRead();
    int lTemp=Read();
    while (lTemp > -1)
    {
        CMIDIEvent e;
        if (lTemp >= 0x80)
        {
            e.message=lTemp;
            e.channel=e.message & 0x0F;
            e.command=e.message & 0xF0;
        }
        forever
        {
            lTemp=Read();
            if ((lTemp >= 0x80) | (lTemp < 0)) break;
            e.data.push_back(lTemp);
        }
        li.append(e);
    }
    return li;
}

const std::vector<BYTE> CMIDIBuffer::MIDIData()
{
    std::vector<BYTE> v(PushCount-ReadCount);
    memcpy(v.data(),&Buffer[ReadCount],PushCount-ReadCount);
    return v;
}
