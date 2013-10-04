#include "csounddevice.h"

float channelData::panL(float pan)
{
    if (pan<=1) return 1;
    return 1-(pan-1);
}

float channelData::panR(float pan)
{
    if (pan>=1) return 1;
    return pan;
}

channelData::channelData()
{
    reset();
}

void channelData::reset()
{
    portNote=0;
    patch=0;
    expression=1;
    volume=1;
    pan=1;
    balance=1;
    pitchWheel=0;
    pressure=1;
    bank=0;
    pedal=false;
    pedalnotes.clear();
}

float channelData::volL()
{
    return expression*volume*panL(pan)*panL(balance)*pressure;
}

float channelData::volR()
{
    return expression*volume*panR(pan)*panR(balance)*pressure;
}

ISoundDevice::ISoundDevice()
{
    m_Transpose=0;
    m_Channel=0;
    isGM=false;
}

void ISoundDevice::NoteOn(short /*channel*/,short /*pitch*/,short /*velocity*/)
{
}

void ISoundDevice::NoteOff(short /*channel*/,short /*pitch*/)
{
}

void ISoundDevice::allNotesOff()
{}

void ISoundDevice::Aftertouch(short /*channel*/,short /*pitch*/,short /*value*/)
{
}

float* ISoundDevice::getNext(const int /*voice*/)
{
    return NULL;
}

short ISoundDevice::voiceChannel(const int /*voice*/)
{
    return 0;
}

int ISoundDevice::voiceCount()
{
    return 0;
}

void ISoundDevice::Patch(const short channel, const short value)
{
    channelSettings[channel].patch=value;
}

void ISoundDevice::Controller(short channel, short controller, short value)
{
    switch (controller)
    {
    case 0: // Bank select
        channelSettings[channel].bank=value;
        break;
    case 32: // Bank select
        //channel[Message & 0xF].bank=Data2;
        //qDebug() << "Bank select" << value;
        break;
    case 84: // portamento note
        channelSettings[channel].portNote=value;
        break;
    case 7: // volume
        channelSettings[channel].volume=(float)value/127.0;
        //qDebug() << "volume" << channelSettings[channel].volume;
        break;
    case 8: // balance
        channelSettings[channel].balance=(float)value/64.0;
        //qDebug() << "balance" << channelSettings[channel].balance;
        break;
    case 10: // pan
        channelSettings[channel].pan=(float)value/64.0;
        //qDebug() << "pan" << channelSettings[channel].pan;
        break;
    case 11: // expression
        channelSettings[channel].expression=(float)value/127.0;
        //qDebug() << "expression" << channelSettings[channel].expression;
        break;
    case 64: // pedal
    case 66:
    case 89:
        channelSettings[channel].pedal=(value > 63);
        if (!channelSettings[channel].pedal)
        {
            foreach (short pitch,channelSettings[channel].pedalnotes)
            {
                NoteOff(channel,pitch);
            }
            channelSettings[channel].pedalnotes.clear();
        }
        break;
    case 121: // All controllers off
        channelSettings[channel].reset();
        break;
    default:
        break;
        //qDebug() << "Controller" << controller << value;
    }
}

void ISoundDevice::PitchBend(short channel, short value)
{
    channelSettings[channel].pitchWheel=value*pitchBendFactor;
}

void ISoundDevice::ChannelPressure(short channel, short value)
{
    channelSettings[channel].pressure=((float)value*0.001)+1;
}

void ISoundDevice::SysEx(char* data, const short datalen)
{
    if (datalen > 3)
    {
        if (data[2]==9)
        {
            isGM=data[3];
            reset();
        }
    }
}

float ISoundDevice::volL(const short channel)
{
    return channelSettings[channel].volL();
}

float ISoundDevice::volR(const short channel)
{
    return channelSettings[channel].volR();
}

void ISoundDevice::reset()
{
    //MessageLength=0;
}

void ISoundDevice::parseMIDI(CMIDIBuffer* MB)
{
    if (!MB) return;
    foreach(CMIDIEvent Event,MB->Events())
    {
        if (Event.message >= 0x80)
        {
            if ((m_Channel==0) | (Event.channel==m_Channel-1))
            {
                if (!Event.data.isEmpty())
                {
                    if (Event.command==0x80)
                    {
                        NoteOff(Event.channel,Event.data.at(0));
                    }
                    else if (Event.command==0xC0)
                    {
                        Patch(Event.channel,Event.data.at(0));
                    }
                    else if (Event.command==0xD0)
                    {
                        ChannelPressure(Event.channel,Event.data.at(0));
                    }
                    else if (Event.command==0xF0)
                    {
                        if ((Event.channel==0) | (Event.channel==7))
                        {
                            SysEx((char*)(Event.data.data()),Event.data.size());
                        }
                        if (Event.channel==0xF)
                        {
                            QString MetaText=QString::fromLatin1((char*)(Event.data.data()),Event.data.size()).trimmed().simplified();
                            if (!MetaText.isEmpty()) qDebug() << "Meta" << MetaText;
                        }
                    }
                    if (Event.data.size() > 1)
                    {
                        if (Event.command==0x90)
                        {
                            if ((int)Event.data.at(1) == 0)
                            {
                                NoteOff(Event.channel,Event.data.at(0));
                            }
                            else
                            {
                                NoteOn(Event.channel,Event.data.at(0),Event.data.at(1));
                            }
                        }
                        else if (Event.command==0xA0)
                        {
                            Aftertouch(Event.channel,Event.data.at(0),Event.data.at(1));
                        }
                        else if (Event.command==0xB0)
                        {
                            Controller(Event.channel,Event.data.at(0),Event.data.at(1));
                        }
                        else if (Event.command==0xE0)
                        {
                            PitchBend(Event.channel,from14bit(Event.data.at(0),Event.data.at(1))-0x2000);
                        }
                    }
                }
            }
        }
    }
}

void ISoundDevice::setTranspose(const short transpose)
{
    m_Transpose=transpose;
}

void ISoundDevice::setChannel(const short channel)
{
    m_Channel=channel;
}
