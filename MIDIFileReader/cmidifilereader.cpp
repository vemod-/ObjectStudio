#include "cmidifilereader.h"
#include "../SoftSynthsClasses/softsynthsdefines.h"
#include "../SoftSynthsClasses/cpresets.h"
#include "../SoftSynthsClasses/cmseccounter.h"

CMIDIFileTrack::CMIDIFileTrack()
{
    m_Length=0;
    m_Delta=0;
    m_MessagePointer=nullptr;
    m_Counter=0;
    m_DataPointer=nullptr;
    m_CurrentPointer=nullptr;
    m_StartPointer=nullptr;
    noteCount=0;
    ticks=0;
}

CMIDIFileTrack::CMIDIFileTrack(byte* &Pointer,const int I) : CMIDIFileTrack()
{
    assign(Pointer);
    index=I;
}

void CMIDIFileTrack::assign(byte* &Pointer)
{
    const auto header=reinterpret_cast<MIDIFileTrackHeader*>(Pointer);
    m_Length=qFromBigEndian<uint>(header->descriptor.size);
    m_StartPointer=Pointer+sizeof(MIDIFileTrackHeader);
    reset();
    Pointer+=m_Length+sizeof(MIDIFileTrackHeader);
}

MessageType CMIDIFileTrack::messageType()
{
    if (*m_CurrentPointer < 0x80) // running status
    {
        setMessage(m_DataSize);
        return MFREvent;
    }
    m_MessagePointer = m_CurrentPointer++;
    if ((*m_MessagePointer >= 0xC0) && (*m_MessagePointer <= 0xDF)) // 2 byte event
    {
        setMessage(1);
        return MFREvent;
    }
    if ((*m_MessagePointer >= 0x80) && (*m_MessagePointer <= 0xEF)) // 3byte event
    {
        setMessage(2);
        return MFREvent;
    }
    if ((*m_MessagePointer == 0xF0) | (*m_MessagePointer == 0xF7)) // sysex event
    {
        setMessage(varlen(m_CurrentPointer)); //length of buffer
        //qDebug() << "Sysex" << *m_MessagePointer << *m_DataPointer << m_DataSize << m_CurrentPointer-m_MessagePointer << *(m_CurrentPointer-3) << *(m_CurrentPointer-2) << *(m_CurrentPointer-1) << *m_CurrentPointer;
        return MFREvent;
    }
    if (*m_MessagePointer==0xFF)
    {
        byte metaType=*m_CurrentPointer++; //Meta type;
        setMessage(varlen(m_CurrentPointer)); //length of buffer
        if (metaType==0x2F) //End of track
        {
            m_Delta=0;
            finished=true;
            return MFREndOfTrack;
        }
        if ((metaType <= 7) | (metaType==0x7F)) //Seq specific
        {
            return MFRMeta;
        }
        switch (metaType)
        {
        case 0x51: //Tempo
            return MFRTempo;
        case 0x54: //SMPTE
            return MFRSMPTEOffset;
        case 0x58: //Time
            return MFRTime;
        case 0x59: //Key
            return MFRKey;
        default: //Anything
            return MFRUnknown;
        }
    }
    setMessage(0);
    return MFRUnknown;
}

void CMIDIFileTrack::reset()
{
    m_CurrentPointer=m_StartPointer;
    m_MessagePointer=m_StartPointer;
    setMessage(0);
    m_Counter=0;
    finished=false;
}

CMIDIFileReader::CMIDIFileReader()
{
    m_ID.first=nullptr;
    m_ID.second=0;
    m_FileType=0;
    m_NumOfTracks=0;
    m_TicksPerQuarter=240;
    m_TempoAdjust=1;
    m_ChannelCount=0;
    m_MinChannel=15;
    m_MilliSeconds=0;
    m_Ticks=0;
}

CMIDIFileReader::~CMIDIFileReader()
{
    qDeleteAll(tracks);
    if (m_ID.first != nullptr) CSingleMap<MIDIMemoryID,MIDIFileMemoryData>::removeItem(m_ID);
}

bool CMIDIFileReader::load(const QString& Path)
{
    try
    {
        QFile f(Path);
        if (f.open(QIODevice::ReadOnly))
        {
            bool retVal=assign(f.readAll());
            f.close();
            return retVal;
        }
    }
    catch (...) {}
    return false;
}

bool CMIDIFileReader::assign(const QByteArray& b)
{
    auto header=reinterpret_cast<const MIDIFileHeader*>(b.constData());
    if (!descriptorMatch(header->descriptor.id,"MThd")) return false;
    if (m_ID.first) CSingleMap<MIDIMemoryID,MIDIFileMemoryData>::removeItem(m_ID);
    MIDIMemoryID ID=MIDIFileMemoryData::makeID(b);
    MIDIFileMemoryData* Data = CSingleMap<MIDIMemoryID,MIDIFileMemoryData>::addItem(ID);
    if (Data->refCount==1) Data->init(b);
    m_ID=ID;
    qDeleteAll(tracks);
    tracks.clear();
    const ulong64 len=qFromBigEndian<uint>(header->descriptor.size);
    m_FileType=qFromBigEndian<short>(header->fileType);
    m_NumOfTracks=qFromBigEndian<short>(header->numTracks);
    m_TicksPerQuarter=qFromBigEndian<short>(header->ticks);
    auto Pointer=reinterpret_cast<byte*>(Data->data.data()) + sizeof(chunk) + len;
    for (int i=0;i<m_NumOfTracks;i++) tracks.append(new CMIDIFileTrack(Pointer,i));
    getTicks();
    return true;
}

MIDITimeList CMIDIFileReader::getTicks(const MIDITimeList& tickList)
{
    MIDITimeList retval;
    m_ChannelCount=0;
    m_MinChannel=15;
    m_MilliSeconds=0;
    m_Ticks=0;
    if (m_NumOfTracks==0) return retval;
    reset();
    bool TickPlay=true;
    ulong SkipTicks=0;
    uint barCount=0;
    CTickCounter mSecCount;
    mSecCount.setTempoAdjust(m_TempoAdjust);
    QList<CMIDIFileTrack*> PlayingTracks(tracks);
    while (TickPlay)
    {
        mSecCount.addMilliSecond();
        while (mSecCount.moreTicks())
        {
            if (SkipTicks==0)
            {
                SkipTicks=1000000000;
                mSecCount.eatTick();
                for (CMIDIFileTrack* T : PlayingTracks)
                {
                    if (T->containsMessages())
                    {
                        do
                        {
                            MessageType mt=T->messageType();
                            if (mt==MFREvent)
                            {
                                CMIDIEvent e=T->midiEvent();
                                if (e.isNoteOn())
                                {
                                    if (e.data(1) != 0)
                                    {
                                        T->noteCount++;
                                        if (e.channel() > m_ChannelCount) m_ChannelCount=e.channel();
                                        if (e.channel() < m_MinChannel) m_MinChannel=e.channel();
                                    }
                                }
                            }
                            else if (mt==MFRTempo)
                            {
                                mSecCount.setTempo(T->tempo(),m_TicksPerQuarter);
                            }
                            else if (mt==MFREndOfTrack)
                            {
                                T->ticks=mSecCount.currentTick() - 1;
                                PlayingTracks.removeOne(T);
                                if (PlayingTracks.isEmpty()) TickPlay = false;
                                break;
                            }
                        }
                        while (T->moreMessages());
                    }
                    SkipTicks=qMin<ulong>(T->remainingTicks(),SkipTicks);
                }
            }
            else if (SkipTicks != 1000000000)
            {
                for(CMIDIFileTrack* T : PlayingTracks) T->skipTicks(SkipTicks);
                while ((barCount < tickList.size()) && (mSecCount.currentTick() + SkipTicks > tickList[barCount]))
                {
                    const ulong thisTick = tickList[barCount]-(mSecCount.currentTick() - 1);
                    retval.push_back(mSecCount.remainingmSecs(thisTick - 1));
                    barCount++;
                }
                mSecCount.skipTicks(SkipTicks);
                SkipTicks = 0;
            }
            else
            {
                TickPlay = false;
                break;
            }
        }
    }
    m_MilliSeconds = mSecCount.currentmSec();
    m_Ticks = mSecCount.currentTick();
    reset();
    return retval;
    //qDebug() << m_MilliSeconds << m_Ticks;
}

ulong64 CMIDIFileReader::mSecsToEvent(const CMIDIEvent& event)
{
    ulong SkipTicks=0;
    reset();
    if (m_NumOfTracks==0) return 0;
    CTickCounter mSecCount;
    mSecCount.setTempoAdjust(m_TempoAdjust);
    QList<CMIDIFileTrack*> PlayingTracks(tracks);
    //qDebug() << "MIDI File Skip" << mSec;
    while (true)
    {
        ulong currentmSec = mSecCount.currentmSec();
        mSecCount.addMilliSecond();
        while (mSecCount.moreTicks())
        {
            if (SkipTicks==0)
            {
                SkipTicks=10000000;
                mSecCount.eatTick();
                for (CMIDIFileTrack* T : PlayingTracks)
                {
                    if (T->containsMessages())
                    {
                        do
                        {
                            MessageType mt=T->messageType();
                            if (event.matches(T->midiEvent())) return currentmSec;
                            if (mt==MFREvent)
                            {
                                /*
                                if ((m_Parameters[pnTrack]->Value==0) | (T->index==m_Parameters[pnTrack]->Value-1))
                                {
                                    CMIDIEvent e=T->midiEvent();
                                    if (((e.command() >= 0xB0) && (e.command() <= 0xD0)) | (e.isSysEx()))
                                    {
                                        SkipBuffer=true;
                                        MIDIBuffer.append(e);
                                    }
                                }
                                */
                            }
                            else if (mt==MFRTempo)
                            {
                                mSecCount.setTempo(T->tempo(),m_TicksPerQuarter);
                            }
                            else if (mt==MFREndOfTrack)
                            {
                                PlayingTracks.removeOne(T);
                                if (PlayingTracks.isEmpty()) return 0;
                                break;
                            }
                        }
                        while (T->moreMessages());
                    }
                    SkipTicks=qMin<ulong>(T->remainingTicks(),SkipTicks);
                }
            }
            else if (SkipTicks != 10000000)
            {
                for (CMIDIFileTrack* t : PlayingTracks) t->skipTicks(SkipTicks);
                mSecCount.skipTicks(SkipTicks);
                SkipTicks=0;
            }
            else
            {
                return 0;
            }
        }
    }
}
