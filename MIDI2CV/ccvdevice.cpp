#include "ccvdevice.h"

CCVDevice::CCVDevice()
{
    //Notes=new CVNote[CVDevice::CVVoices];
    //reset();
    Tune=440;
}

CCVDevice::~CCVDevice()
{
    //delete [] Notes;
}

void CCVDevice::noteOn(const short channel, const short key, const short velocity)
{
    if (channelSettings[channel].portNote)
    {
        for (CVNote& n : Notes)
        {
            if ((channel) == n.Channel)
            {
                if (n.MIDIKey==channelSettings[channel].portNote)
                {
                    n.MIDIKey=key;
                    n.Voltage += MIDIkey2voltage(key-channelSettings[channel].portNote);
                    break;
                }
            }
        }
    }
    else
    {
        int FreeIndex=-1;
        for (int i=0;i<CVDevice::CVVoices;i++)
        {
            if (Notes[i].MIDIVelocity==0)
            {
                FreeIndex=i;
                break;
            }
        }
        if (FreeIndex>-1)
        {
            CVNote& n = Notes[FreeIndex];
            n.Channel=channel;
            n.MIDIKey=key;
            n.Voltage=MIDIkey2voltage(key, Tune);
            //qDebug() << n.Frequency;
            n.MIDIVelocity=velocity;
            n.Velocity=float(velocity & 0x7F) / float(0x7F);
        }
    }
    channelSettings[channel].portNote=0;
}

void CCVDevice::noteOff(const short channel, const short pitch)
{
    for (CVNote& n : Notes)
    {
        if (channel==n.Channel)
        {
            if (channelSettings[channel].pedal)
            {
                channelSettings[channel].pedalnotes.append(pitch);
            }
            else if (pitch==n.MIDIKey)
            {
                n.Off();
                break;
            }
        }
    }
}
/*
void CCVDevice::controller(const short channel, const short controller, const short value)
{
    switch (controller)
    {
    case 84: // portamento note
        channelSettings[channel].portNote=value+m_Transpose;
        break;
    default:
        ISoundDevice::controller(channel,controller,value);
        break;
    }
}
*/
int CCVDevice::voiceCount() const
{
    return CVDevice::CVVoices;
}

void CCVDevice::reset()
{
    //zeroMemory(Notes,CVDevice::CVVoices*sizeof(CVNote));
    for (CVNote& n : Notes) n.Zero();
    for (ChannelData& d : channelSettings) d.resetAll();
}

void CCVDevice::allNotesOff()
{
    for (CVNote& n : Notes) n.Zero();
/*
    for (int i=0;i<CVDevice::CVVoices;i++)
    {
        Notes[i].MIDIPitch=0;
        Notes[i].Velocity=0;
        Notes[i].MIDIVelocity=0;
    }
    */
}

short CCVDevice::voiceChannel(const int voice) const
{
    return Notes[voice].Channel;
}

float CCVDevice::getPitchbend(const int voice) const
{
    return channelSettings[voiceChannel(voice)].pitchWheel;
}

float CCVDevice::Vol(const int voice) const
{
    const int ch=voiceChannel(voice);
    return channelSettings[ch].volume*channelSettings[ch].expression;
}
