#ifndef CMIDIFILEREADER_H
#define CMIDIFILEREADER_H

#include <QtCore>

typedef unsigned char BYTE;
typedef unsigned short* PWORD;

enum MessageType
{
    MFREndOfTrack,MFR3Bytes,MFR2Bytes,MFRBuffer,MFRTempo,MFRTime,MFRKey,MFRSMPTEOffset,MFRUnknown
};

#pragma pack(push,1)

typedef QPair<char*,size_t> MIDIMemoryID;

class MIDIFileMemoryData
{
public:
    MIDIFileMemoryData(const char* Pnt, const size_t Length)
    {
        data=new char[Length];
        memcpy(data,Pnt,Length);
        refcount=0;
    }
    ~MIDIFileMemoryData()
    {
        delete [] data;
    }
    char* data;
    int refcount;
};

class SingleMIDIMap : public QMap<MIDIMemoryID, MIDIFileMemoryData*>
{
    public:
        static SingleMIDIMap* getInstance()
        {
            static SingleMIDIMap    instance; // Guaranteed to be destroyed.
                                  // Instantiated on first use.
            return &instance;
        }
    private:
        SingleMIDIMap() {}                   // Constructor? (the {} brackets) are needed here.
        SingleMIDIMap(SingleMIDIMap const&);              // Don't Implement
        void operator=(SingleMIDIMap const&); // Don't implement
};

struct chunk
{
    char        id[4];
    quint32     size;
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
    unsigned int m_Length;
    unsigned long m_Time;
    short m_Time1;
    short m_Time2;
    short m_SharpFlat;
    short m_Key;
    BYTE m_Message;
    unsigned int m_Tempo;
    MessageType m_MoreMessages;
    BYTE* NextPointer;
    BYTE* TimePointer;
    BYTE* StartPointer;
    BYTE* DataPointer;
public:
    int Index;
    bool Finished;
    unsigned long Counter;
    unsigned long Duration;
    unsigned long NoteCount;
    CMIDIFileTrack();
    ~CMIDIFileTrack();
    size_t Fill(const char* Data, const size_t Pointer);
    unsigned long inline GetTime();
    MessageType MoreMessages();
    short GetData();
    unsigned long Time();
    BYTE Message();
    unsigned int Tempo();
    BYTE Time1();
    BYTE Time2();
    BYTE SharpFlat();
    BYTE Key();
    BYTE* Data();
    int DataIndex;
    int DataSize;
    void Reset();
    bool MessageReady();
};

class CMIDIFileReader
{
private:
    short m_FileType;
    short m_NumOfTracks;
    void GetDuration();
    unsigned long m_MilliSeconds;
    unsigned long m_Ticks;
    //char* m_Pnt;
    MIDIMemoryID m_ID;
public:
    CMIDIFileReader();
    ~CMIDIFileReader();
    QList<CMIDIFileTrack*> Tracks;
    short Ticks;
    bool Open(const QString& Path);
    bool OpenPtr(const char* Pnt, const size_t Length);
    short TrackCount();
    short FileType();
    unsigned long MilliSeconds();
    unsigned long Duration(const int Track=-1);
    unsigned long NoteCount(const int Track);
    void Reset();
    SingleMIDIMap* MIDIFiles;
};

#pragma pack(pop)

#endif // CMIDIFILEREADER_H
