#ifndef CMIDIFILEPLAYER_H
#define CMIDIFILEPLAYER_H

#include "idevice.h"
#include "cmidifilereader.h"
#include "cmseccounter.h"

namespace MIDIFilePlayer
{
const QString MIDIFilter("MIDI files (*.mid;*.kar)");
}

class CMIDIFilePlayer : public IDevice, public IFileLoader
{
public:
    enum ParameterNames
    {pnTrack,pnTempoAdjust,pnHumanize};
    CMIDIFilePlayer();
    ~CMIDIFilePlayer()
    {
    }
/*
    inline ulong currentTick() { return mSecCount.currentTick(); }
    inline ulong currentMilliSecond() { return mSecCount.currentmSec(); }
*/
    void init(const int Index, QWidget* MainWindow);
    void tick();
    void skip(const ulong64 samples);
    CMIDIBuffer* getNextP(const int ProcIndex);
    void play(const bool FromStart);
    //void pause();
    void execute(const bool Show);
    bool isPlaying();
    ulong ticks() const;
    ulong milliSeconds() const;
    ulong64 samples() const;
    void assign(const QByteArray& b, const QString& filename = QString());
    bool loadFile(const QString& fn);
private:
    enum JackNames
    {jnMIDI};
    //bool Playing;
    bool SkipBuffer;
    CMIDIBuffer MIDIBuffer;
    CMIDIFileReader MFR;
    QList<CMIDIFileTrack*> PlayingTracks;

    void inline updateDeviceParameter(const CParameter* p = nullptr);
    void inline Reset();
    CTickCounter mSecCount;
};

#endif // CMIDIFILEPLAYER_H
