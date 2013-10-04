#include "cmidifileplayer.h"
#undef devicename

#undef devicename
#define devicename "MIDIFile"

CMIDIFilePlayer::CMIDIFilePlayer()
{
}

void CMIDIFilePlayer::Init(const int Index,void* MainWindow)
{
    m_Name=devicename;
    IDevice::Init(Index,MainWindow);
    AddJackMIDIOut(jnMIDI);
    AddParameterTrack();
    Playing=false;
    uSPQ = 500000;
    CurrentTick=0;
    CurrentMilliSecond=0;
    SampleCount=0;
    mSecCount=0;
    SamplesPermSec=m_Presets.SampleRate*0.001;
}

void CMIDIFilePlayer::Tick()
{
    if (!Playing) return;
    //qDebug() << "Tick" << mSecCount << SampleCount << SamplesPermSec << SamplesPerTick << CurrentTick << CurrentMilliSecond << SkipBuffer << uSPerTick << uSPQ;
    if (PlayingTracks.isEmpty())
    {
        Playing=false;
        return;
    }
    if (!SkipBuffer)
    {
        MIDIBuffer.Reset();
    }
    else
    {
        SkipBuffer=false;
    }
    SampleCount+=m_BufferSize;
    mSecCount+=m_BufferSize;
    forever
    {
        if (mSecCount-SamplesPermSec < 0) break;
        mSecCount-=SamplesPermSec;
        CurrentMilliSecond++;
    }
    while (SampleCount>=SamplesPerTick)
    {
        CurrentTick++;
        SampleCount-=SamplesPerTick;
        foreach (CMIDIFileTrack* T,PlayingTracks)
        {
            if (T->MessageReady())
            {
                do
                {
                    MessageType mt=T->MoreMessages();
                    if (mt==MFRTempo)
                    {
                        uSPQ=T->Tempo();
                        CalcTempo();
                    }
                    else if (mt==MFREndOfTrack)
                    {
                        PlayingTracks.removeOne(T);
                        if (PlayingTracks.isEmpty()) return;
                        break;
                    }
                    else
                    {
                        BYTE Message=T->Message();
                        if ((m_ParameterValues[pnTrack]==0) | (T->Index==m_ParameterValues[pnTrack]-1) | (Message==0xF0) | (Message==0XF7) | (Message==0xFF))
                        {
                            MIDIBuffer.Push(Message,T->Data(),T->DataSize);
                        }
                    }
                }
                while (T->Time()==0);
            }
        }
    }
}

void CMIDIFilePlayer::Skip(const unsigned long mSec)
{
    unsigned long SkipTicks=0;
    Reset();
    //qDebug() << "Skip" << mSecCount;
    while (CurrentMilliSecond < mSec)
    {
        SampleCount+=m_BufferSize;
        mSecCount+=m_BufferSize;
        forever
        {
            if (mSecCount-SamplesPermSec < 0) break;
            mSecCount-=SamplesPermSec;
            CurrentMilliSecond++;
        }
        while (SampleCount>=SamplesPerTick)
        {
            if (SkipTicks==0)
            {
                SkipTicks=10000000;
                CurrentTick++;
                SampleCount-=SamplesPerTick;
                foreach (CMIDIFileTrack* T,PlayingTracks)
                {
                    if (T->MessageReady())
                    {
                        do
                        {
                            MessageType mt=T->MoreMessages();
                            if (mt==MFRTempo)
                            {
                                uSPQ=T->Tempo();
                                CalcTempo();
                            }
                            else if (mt==MFREndOfTrack)
                            {
                                PlayingTracks.removeOne(T);
                                if (PlayingTracks.isEmpty()) return;
                                break;
                            }
                            else
                            {
                                if ((m_ParameterValues[pnTrack]==0) | (T->Index==m_ParameterValues[pnTrack]-1))
                                {
                                    BYTE Message=T->Message();
                                    if (((Message >= 0xB0) & (Message <= 0xDF)) | ((Message >= 0xF0) & (Message < 0xFF)))
                                    {
                                        SkipBuffer=true;
                                        MIDIBuffer.Push(Message,T->Data(),T->DataSize);
                                    }
                                }
                            }
                        }
                        while (T->Time()==0);
                    }
                    unsigned int TickDiff=T->Time()-T->Counter;
                    if (TickDiff < SkipTicks) SkipTicks=TickDiff;
                }
            }
            else if (SkipTicks != 10000000)
            {
                foreach (CMIDIFileTrack* T, PlayingTracks) T->Counter+=SkipTicks;
                CurrentTick+=SkipTicks;
                SampleCount-=(float)SkipTicks*SamplesPerTick;
                unsigned long elapsedmSec=qMax(ceil((-SampleCount+SamplesPerTick)/SamplesPermSec),0.0);
                SampleCount+=(SamplesPermSec*elapsedmSec);
                CurrentMilliSecond+=elapsedmSec;
                SkipTicks=0;
            }
            else
            {
                return;
            }
        }
    }
}

void* CMIDIFilePlayer::GetNextP(const int /*ProcIndex*/)
{
    if (!Playing) return NULL;
    return (void*)&MIDIBuffer;
}

void CMIDIFilePlayer::Play(const bool FromStart)
{
    if (MFR.Duration()==0) return;
    if (FromStart) Reset();
    Playing=true;
}

void CMIDIFilePlayer::Pause()
{
    Playing=false;
}

void CMIDIFilePlayer::Execute(const bool /*Show*/)
{
    QString fn=OpenFile(MIDIFilePlayer::MIDIFilter);
    if (!fn.isEmpty()) OpenMidiFile(fn);
}

const QString CMIDIFilePlayer::Save()
{
    QDomLiteElement xml("Custom");
    xml.setAttribute("File",QDir().relativeFilePath(FileName()));
    return xml.toString();
}

void CMIDIFilePlayer::Load(const QString& XML)
{
    QDomLiteElement xml;
    xml.fromString(XML);
    if (xml.tag=="Custom")
    {
        QString Path=CPresets::ResolveFilename(xml.attribute("File"));
        OpenMidiFile(Path);
    }
}

bool CMIDIFilePlayer::IsPlaying()
{
    return Playing;
}

unsigned long CMIDIFilePlayer::Duration()
{
    return MFR.Duration();
}

unsigned long CMIDIFilePlayer::MilliSeconds()
{
    return MFR.MilliSeconds();
}

void CMIDIFilePlayer::OpenMidiFile(const QString& fn)
{
    if (QFileInfo(fn).exists())
    {
        if (MFR.Open(fn))
        {
            m_FileName=fn;
            bool TempPlay=Playing;
            Pause();
            Reset();
            if (TempPlay) Play(true);
            return;
        }
    }
    QMessageBox::warning(0,"Warning","Could not open "+fn);
}

void CMIDIFilePlayer::OpenPtr(const char* Pnt, const int Length)
{
    if (MFR.OpenPtr(Pnt,Length))
    {
        m_FileName.clear();
        bool TempPlay=Playing;
        Pause();
        Reset();
        if (TempPlay) Play(true);
    }
}

void inline CMIDIFilePlayer::CalcParams()
{
    CalcTempo();
}

void CMIDIFilePlayer::Reset()
{
    MFR.Reset();
    Ticks=MFR.Ticks;
    PlayingTracks.clear();
    PlayingTracks.append(MFR.Tracks);
    SkipBuffer=false;
    CurrentTick=0;
    CurrentMilliSecond=0;
    SampleCount=0;
    mSecCount=0;
    uSPQ=500000;
    uSPerTick=(float)uSPQ/(float)Ticks;
    MIDIBuffer.Reset();
    CalcParams();
    //qDebug() << "Reset" << mSecCount << SampleCount << SamplesPermSec << SamplesPerTick << CurrentTick << CurrentMilliSecond << SkipBuffer << uSPerTick << uSPQ;
    //qDebug() << PlayingTracks.first()->MoreMessages() << PlayingTracks.first()->Message() << PlayingTracks.first()->Tempo();
}

void CMIDIFilePlayer::CalcTempo()
{
    uSPerTick=(float)uSPQ/(float)Ticks;
    SamplesPerTick=uSPerTick/m_Presets.uSPerSample;
}
