#include "cmidifileplayer.h"
//#include <QMessageBox>
//#undef devicename

#undef devicename
#define devicename "MIDIFilePlayer"

CMIDIFilePlayer::CMIDIFilePlayer()
{
    //Playing=false;
    SkipBuffer=false;
}

void CMIDIFilePlayer::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackMIDIOut(jnMIDI);
    addParameterTrack();
    addParameter(CParameter::ParameterTypes::Percent,"TempoAdjust","%",1,200,0,nullptr,100);
    addParameterPercent("Humanize");
    addFileParameter();
    mSecCount.reset();
    mSecCount.setTempo(500000);
}

void CMIDIFilePlayer::tick()
{
    if (!m_Playing) return;
    //qDebug() << "Tick" << mSecCount << SampleCount << SamplesPermSec << SamplesPerTick << CurrentTick << CurrentMilliSecond << SkipBuffer << uSPerTick << uSPQ;
    if (PlayingTracks.isEmpty())
    {
        m_Playing=false;
        return;
    }
    if (!SkipBuffer)
    {
        MIDIBuffer.clear();
    }
    else
    {
        SkipBuffer=false;
    }
    mSecCount.addBuffer();
    while (mSecCount.moreTicks())
    {
        mSecCount.eatTick();
        for (CMIDIFileTrack* T : std::as_const(PlayingTracks))
        {
            if (T->containsMessages())
            {
                do
                {
                    MessageType mt=T->messageType();
                    if (mt==MFREvent)
                    {
                        if ((m_Parameters[pnTrack]->Value==0) || (T->index==m_Parameters[pnTrack]->Value-1))
                        {
                            CMIDIEvent e = T->midiEvent();
                            if (e.isNoteOn()) e.setVelocity(qMin<int>(127, e.velocity() * humanizeFactor(m_Parameters[pnHumanize]->Value)));
                            MIDIBuffer.append(e);
                        }
                    }
                    else if (mt==MFRTempo)
                    {
                        mSecCount.setTempo(T->tempo(),MFR.ticksPerQuarter());
                    }
                    else if (mt==MFREndOfTrack)
                    {
                        PlayingTracks.removeOne(T);
                        if (PlayingTracks.isEmpty()) return;
                        break;
                    }
                    else if (mt==MFRMeta)
                    {
                        if (m_Host) m_Host->takeString(this, T->metaType(), T->string());
                    }
                }
                while (T->moreMessages());
            }
        }
    }
    IDevice::tick();
}

void CMIDIFilePlayer::skip(const ulong64 samples)
{
    ulong SkipTicks=0;
    Reset();
    for (byte j=0;j<16;j++) MIDIBuffer.append(0xB0+j,0x7B);
    for (byte j=0;j<16;j++) MIDIBuffer.append(0xB0+j,0x78);
    for (byte i=0;i<16;i++) {
        for (byte n=1;n<128;n++) MIDIBuffer.append(0x80+i,n,0);
    }
    //qDebug() << "MIDI File Skip" << mSec;
    while (mSecCount.currentSample() < samples)
    {
        mSecCount.addBuffer();
        while (mSecCount.moreTicks())
        {
            if (SkipTicks==0)
            {
                SkipTicks=10000000;
                mSecCount.eatTick();
                for (CMIDIFileTrack* T : std::as_const(PlayingTracks))
                {
                    if (T->containsMessages())
                    {
                        do
                        {
                            MessageType mt=T->messageType();
                            if (mt==MFREvent)
                            {
                                if ((m_Parameters[pnTrack]->Value==0) || (T->index==m_Parameters[pnTrack]->Value-1))
                                {
                                    CMIDIEvent e=T->midiEvent();
                                    if (((e.command() >= 0xB0) && (e.command() <= 0xD0)) || (e.isSysEx()))
                                    {
                                        SkipBuffer=true;
                                        MIDIBuffer.append(e);
                                    }
                                }
                            }
                            else if (mt==MFRTempo)
                            {
                                mSecCount.setTempo(T->tempo(),MFR.ticksPerQuarter());
                            }
                            else if (mt==MFREndOfTrack)
                            {
                                PlayingTracks.removeOne(T);
                                if (PlayingTracks.isEmpty()) {
                                    IDevice::skip(samples);
                                    return;
                                }
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
                const bool slack = mSecCount.skipSlack(SkipTicks,samples);
                for (CMIDIFileTrack* t : std::as_const(PlayingTracks)) t->skipTicks(SkipTicks);
                SkipTicks = 0;
                if (slack) {
                    IDevice::skip(samples);
                    return;
                }
            }
            else
            {
                IDevice::skip(samples);
                return;
            }
        }
    }
    IDevice::skip(samples);
}


CMIDIBuffer* CMIDIFilePlayer::getNextP(const int /*ProcIndex*/)
{
    return (!m_Playing) ? nullptr : &MIDIBuffer;
}

void CMIDIFilePlayer::play(const bool FromStart)
{
    if (MFR.milliSeconds()==0) return;
    if (FromStart) Reset();
    //Playing=true;
    qDebug() << m_Parameters[pnTempoAdjust]->Value << mSecCount.tempoAdjust();
    IDevice::play(FromStart);
}
/*
void CMIDIFilePlayer::pause()
{
    //Playing=false;
    IDevice::pause();
}
*/
void CMIDIFilePlayer::execute(const bool Show)
{
    if (Show) openFile(selectFile(MIDIFilePlayer::MIDIFilter));
}

bool CMIDIFilePlayer::isPlaying()
{
    return m_Playing;
}

ulong CMIDIFilePlayer::ticks() const
{
    return qMax<ulong>(MFR.ticks(), IDevice::ticks());
}

ulong CMIDIFilePlayer::milliSeconds() const
{
    return qMax<ulong>(MFR.milliSeconds(), IDevice::milliSeconds());
}

ulong64 CMIDIFilePlayer::samples() const
{
    return qMax<ulong64>(MFR.samples(), IDevice::samples());
}

bool CMIDIFilePlayer::loadFile(const QString& fn)
{
    if (MFR.load(fn))
    {
        bool TempPlay=m_Playing;
        pause();
        Reset();
        if (TempPlay) play(true);
        return true;
    }
    return false;
}

void CMIDIFilePlayer::assign(const QByteArray& b, const QString& filename)
{
    if (MFR.assign(b))
    {
        m_FileParameter->setFilename(filename);
        bool TempPlay=m_Playing;
        pause();
        Reset();
        if (TempPlay) play(true);
    }
}

void inline CMIDIFilePlayer::updateDeviceParameter(const CParameter* /*p*/)
{
    mSecCount.setTempoAdjust(m_Parameters[pnTempoAdjust]->PercentValue);
    MFR.setTempoAdjust(m_Parameters[pnTempoAdjust]->PercentValue);
}

void CMIDIFilePlayer::Reset()
{
    MFR.reset();
    PlayingTracks.clear();
    PlayingTracks.append(MFR.tracks);
    SkipBuffer=false;
    mSecCount.reset(MFR.ticksPerQuarter());
    mSecCount.setTempo(500000);
    MIDIBuffer.clear();
    updateDeviceParameter();
}

