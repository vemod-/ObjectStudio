#ifndef CMIDIFILEREADER_H
#define CMIDIFILEREADER_H

#include <QtCore>
#include "../SoftSynthsClasses/csinglemap.h"
#include "../SoftSynthsClasses/cmidibuffer.h"
#include "../SoftSynthsClasses/cpresets.h"

typedef byte byte;
typedef ushort* PWORD;
typedef std::vector<ulong> MIDITimeList;

enum MessageType
{
    MFREndOfTrack,MFREvent,MFRTempo,MFRTime,MFRKey,MFRSMPTEOffset,MFRMeta,MFRUnknown
};

#pragma pack(push,1)

typedef QPair<char*,ulong64> MIDIMemoryID;

class MIDIFileMemoryData : public IRefCounter
{
public:
    MIDIFileMemoryData():IRefCounter() { data=nullptr; }
    void init(const QByteArray& b) { data=b; }
    QByteArray data;
    static MIDIMemoryID makeID(const QByteArray& b) { return qMakePair(const_cast<char*>(b.constData()),b.size()); }
};

struct chunk
{
    char        id[4];
    uint     size;
};

struct MIDIFileHeader
{
    chunk descriptor;
    short fileType;
    short numTracks;
    short ticks;
};

struct MIDIFileTrackHeader
{
    chunk descriptor;
};

class CMIDIFileTrack
{
private:
    uint m_Length;
    ulong m_Delta;
    ulong m_Counter;
    byte* m_MessagePointer;
    byte* m_CurrentPointer;
    byte* m_StartPointer;
    byte* m_DataPointer;
    int m_DataSize;
    ulong inline varlen(byte*& ptr) const
    {
        ulong value;
        byte c;
        if ( (value = *ptr++) & 0x80 )
        {
            value &= 0x7F;
            do
            {
                value = (value << 7) + ((c = *ptr++) & 0x7F);
            } while (c & 0x80);
        }
        return value;
    }
    ulong inline int24(byte* ptr) const
    {
        return qFromBigEndian<int>(*reinterpret_cast<int*>(ptr-1)) & 0xFFFFFF;
    }
    void inline setMessage(const ulong s)
    {
        m_DataPointer = m_CurrentPointer;
        m_DataSize = s;
        m_CurrentPointer += s;
        m_Delta = varlen(m_CurrentPointer);
    }
public:
    int index;
    bool finished;
    ulong ticks;
    ulong noteCount;
    CMIDIFileTrack();
    CMIDIFileTrack(byte* &Pointer,const int I);
    ~CMIDIFileTrack(){}
    void assign(byte* &Pointer);
    MessageType messageType();
    inline bool moreMessages() const { return (m_Delta == 0); }
    inline ulong remainingTicks() const { return m_Delta - m_Counter; }
    inline void skipTicks(const ulong ticks) { m_Counter += ticks; }
    inline uint tempo() const { return int24(m_DataPointer); }
    inline byte time1() const { return *m_DataPointer; }
    inline byte time2() const { return *(m_DataPointer + 1); }
    inline byte sharpFlat() const { return *m_DataPointer; }
    inline byte key() const { return *(m_DataPointer + 1); }
    inline byte metaType() const { return *(m_MessagePointer + 1); }
    inline const QString string() const { return QString::fromLatin1(reinterpret_cast<char*>(m_DataPointer),m_DataSize).trimmed(); }
    void reset();
    inline bool containsMessages() {
        if (m_Delta != m_Counter++) return false;
        m_Counter -= m_Delta;
        return true;
    }
    const inline CMIDIEvent midiEvent() const
    {
        return CMIDIEvent(m_MessagePointer,m_DataPointer,m_DataSize);
    }
};

class CMIDIFileReader
{
private:
    short m_FileType;
    short m_NumOfTracks;
    short m_TicksPerQuarter;
    MIDITimeList getTicks(const MIDITimeList& tickList=MIDITimeList());
    ulong m_MilliSeconds;
    ulong m_Ticks;
    MIDIMemoryID m_ID;
    double m_TempoAdjust;
    short m_ChannelCount;
    short m_MinChannel;
public:
    CMIDIFileReader();
    ~CMIDIFileReader();
    QList<CMIDIFileTrack*> tracks;
    bool load(const QString& Path);
    bool assign(const QByteArray& b);
    inline short trackCount() const { return m_NumOfTracks; }
    inline short channelCount() const { return m_ChannelCount + 1; }
    inline short minChannel() const { return m_MinChannel; }
    inline short fileType() const { return m_FileType; }
    inline short ticksPerQuarter() const { return m_TicksPerQuarter; }
    inline ulong milliSeconds() const { return m_MilliSeconds; }
    inline ulong64 samples() const { return CPresets::mSecsToSamples(m_MilliSeconds); }
    inline ulong ticks(const int Track=-1) const {
        return (Track==-1) ? m_Ticks : tracks.at(Track)->ticks;
    }
    void setTempoAdjust(const double t) {
        if (closeEnough(t, m_TempoAdjust)) return;
        m_TempoAdjust = t;
        getTicks();
    }
    inline ulong noteCount(const int Track) const { return tracks.at(Track)->noteCount; }
    inline void reset() { for (CMIDIFileTrack* t : std::as_const(tracks)) t->reset(); }
    MIDITimeList mSecList(const MIDITimeList& tickList) { return getTicks(tickList); }
    ulong64 mSecsToEvent(const CMIDIEvent& event);


};

#pragma pack(pop)

#endif // CMIDIFILEREADER_H
