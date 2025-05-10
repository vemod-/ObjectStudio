#ifndef IMIDIPARSER_H
#define IMIDIPARSER_H

#include "cmidibuffer.h"

class IMIDIParser
{
public:
    IMIDIParser()
    {
        m_Transpose=0;
        m_ChannelMode=0;
        m_PatchResponse=true;
    }
    virtual ~IMIDIParser();
    void parseMIDI(const CMIDIBuffer* MB)
    {
        if (!MB) return;
        const CMIDIEventList l=MB->eventList();
        for (uint i=0; i<l.size(); i++)
        {
            const CMIDIEvent* Event=l[i];
            if (Event->isSysEx())
            {
                if (!Event->dataEmpty()) parseEvent(Event);
            }
            else if (isResponding(Event->channel()))
            {
                if (Event->dataSize()==1)
                {
                    if (Event->isPatchChange())
                    {
                        if (m_PatchResponse) parseEvent(Event);
                    }
                    else
                    {
                        parseEvent(Event);
                    }
                }
                else if (Event->dataSize()==2)
                {
                    if ((Event->isController()) && ((Event->data(0)==0) || (Event->data(0)==0x20)))
                    {
                        if (m_PatchResponse) parseEvent(Event);
                    }
                    else
                    {
                        parseEvent(Event);
                    }
                }
            }
        }
    }
    virtual void parseEvent(const CMIDIEvent* /*Event*/) {}
    virtual void allNotesOff(){
        CMIDIBuffer b;
        for (byte j=0;j<16;j++) b.append(0xB0+j,0x7B);
        for (byte j=0;j<16;j++) b.append(0xB0+j,0x78);
        for (byte i=0;i<16;i++) {
            for (byte n=1;n<128;n++) b.append(0x80+i,n,0);
        }
        parseMIDI(&b);
    }
    inline bool isOmniMode() const { return (m_ChannelMode==0); }
    inline bool isMonoMode() const { return (m_ChannelMode > 0); }
    inline short monoModeChannel() const { return m_ChannelMode-1; }
    void setMonoModeChannel(short channel) { m_ChannelMode=channel+1; }
    inline bool isResponding(const byte channel) const { return (isOmniMode() || (monoModeChannel()==channel)); }
    void setChannelMode(const short channel) { m_ChannelMode=channel; }
    inline short channelMode() const { return m_ChannelMode; }
    void setPatchResponse(bool patchResponse) { m_PatchResponse=patchResponse; }
    inline bool patchResponse() const { return m_PatchResponse; }
    void setTranspose(const short transpose) {
        if (transpose != m_Transpose)
        {
            allNotesOff();
            m_Transpose=transpose;
        }
    }
    inline short transpose() const { return m_Transpose; }
protected:
    short m_ChannelMode;
    bool m_PatchResponse;
    short m_Transpose;
};

#endif // IMIDIPARSER_H
