#ifndef CCVDEVICE_H
#define CCVDEVICE_H

#include "csounddevice.h"

namespace CVDevice
{
const int CVVoices = 8;
}

class CCVDevice : public ISoundDevice
{
public:
    class CVNote
    {
    public:
        CVNote() { Zero(); }
        double Voltage;
        float Velocity;
        int MIDIKey;
        int MIDIVelocity;
        int Channel;
        void Zero()
        {
            MIDIKey=0;
            Channel=0;
            Off();
        }
        void Off()
        {
            Voltage=0;
            Velocity=0;
            MIDIVelocity=0;
        }
    };
    CCVDevice();
    ~CCVDevice();
    void noteOn(const short channelMode, const short pitch, const short velocity);
    void noteOff(const short channelMode, const short pitch);
    //void controller(const short channelMode, const short controller, const short value);
    int voiceCount() const;
    void reset();
    void allNotesOff();
    const CVNote& note(const int v) const { return Notes[v]; }
    float Tune;
    short voiceChannel(const int voice) const;
    float getPitchbend(const int voice) const;
    float Vol(const int voice) const;
private:
    CVNote Notes[CVDevice::CVVoices];
};


#endif // CCVDEVICE_H
