#ifndef CSOUNDDEVICE_H
#define CSOUNDDEVICE_H

#include "imidiparser.h"
;
#pragma pack(push,1)

class ChannelData
{
private:
    float inline panL(float pan) const
    {
        return (pan<=1) ? 1 : 1-(pan-1);
    }
    float inline panR(float pan) const
    {
        return (pan>=1) ? 1 : pan;
    }
public:
    ChannelData() { resetAll(); }
    void reset()
    {
        portNote=0;
        expression=1;
        volume=1;
        pan=1;
        balance=1;
        pitchWheel=0;
        pressure=1;
        pedal=false;
        pedalnotes.clear();
    }
    void resetPatch()
    {
        patch=0;
        bank=0;
    }
    void resetAll()
    {
        reset();
        resetPatch();
    }
    float inline volL() const { return expression*volume*panL(pan)*panL(balance)*pressure; }
    float inline volR() const { return expression*volume*panR(pan)*panR(balance)*pressure; }
    float expression;
    float volume;
    float pan;
    float balance;
    float pitchWheel;
    float pressure;
    QList<short> pedalnotes;
    short portNote;
    short patch;
    short bank;
    bool pedal;
};

#pragma pack(pop)

class ISoundDevice : public IMIDIParser
{
public:
    ISoundDevice() : IMIDIParser() { m_IsGM=false; }
    virtual ~ISoundDevice();
    virtual void noteOn(const short /*channel*/, const short /*pitch*/, const short /*velocity*/){}
    virtual void noteOff(const short /*channel*/, const short /*pitch*/){}
    virtual void aftertouch(short /*channel*/,short /*pitch*/,short /*value*/){}
    virtual void controller(const short channel, const short controller, const short value)
    {
        switch (controller)
        {
        case 0: // Bank select
            channelSettings[channel].bank=value;
            break;
        case 32: // Bank select
            //channelSettings[channel].bank=value;
            break;
        case 84: // portamento note
            channelSettings[channel].portNote=value;
            break;
        case 7: // volume
            channelSettings[channel].volume=value/127.f;
            break;
        case 8: // balance
            channelSettings[channel].balance=value/64.f;
            break;
        case 10: // pan
            channelSettings[channel].pan=value/64.f;
            break;
        case 11: // expression
            channelSettings[channel].expression=value/127.f;
            break;
        case 64: // pedal
        case 66:
        case 89:
            channelSettings[channel].pedal=(value > 63);
            if (!channelSettings[channel].pedal)
            {
                const QList<short> l=channelSettings[channel].pedalnotes;
                for (const short& i : l) noteOff(channel,i);
                channelSettings[channel].pedalnotes.clear();
            }
            break;
        case 121: // All controllers off
            channelSettings[channel].reset();
            break;
        default:
            break;
        }
    }
    virtual void PitchBend(const short channel, const short value)
    {
        channelSettings[channel].pitchWheel=value*pitchBendFactor;
    }
    virtual void ChannelPressure(const short channel, const short value)
    {
        channelSettings[channel].pressure=(value*0.001f)+1;
    }
    virtual void patch(const short channel, const short value)
    {
        channelSettings[channel].patch=value;
    }
    virtual void SysEx(const byte* data, const ulong datalen)
    {
        if (datalen == 6)
        {
            if (data[datalen-3]==9)
            {
                m_IsGM=data[datalen-2];
                reset();
            }
        }
    }
    virtual float* getNext(const int /*voice*/) { return nullptr; }
    virtual short voiceChannel(const int /*voice*/) const { return 0; }
    virtual int voiceCount() const { return 0; }
    virtual float volL(const short channel) const { return channelSettings[channel].volL(); }
    virtual float volR(const short channel) const { return channelSettings[channel].volR(); }
    virtual void reset() {}
    void parseEvent(const CMIDIEvent* Event)
    {
        if (Event->isSysEx())
        {
            SysEx(Event->memPtr(),Event->memSize());
        }
        else
        {
            if (!((m_IsGM) && (Event->channel() == 9))) Event->transpose(m_Transpose);
            if (Event->isNoteOff())
            {
                noteOff(Event->channel(),Event->data(0));
            }
            else if (Event->isNoteOn())
            {
                if (Event->data(1) == 0)
                    noteOff(Event->channel(),Event->data(0));
                else
                    noteOn(Event->channel(),Event->data(0),Event->data(1));
            }
            else if (Event->isAftertouch())
            {
                aftertouch(Event->channel(),Event->data(0),Event->data(1));
            }
            else if (Event->isController())
            {
                controller(Event->channel(),Event->data(0),Event->data(1));
            }
            else if (Event->isPatchChange())
            {
                patch(Event->channel(),Event->data(0));
            }
            else if (Event->isChannelPressure())
            {
                ChannelPressure(Event->channel(),Event->data(0));
            }
            else if (Event->isPitchBend())
            {
                PitchBend(Event->channel(),short(from14bit(Event->data(0),Event->data(1)))-0x2000);
            }
        }
    }
protected:
    ChannelData channelSettings[16];
    bool m_IsGM;
};

#endif // CSOUNDDEVICE_H
