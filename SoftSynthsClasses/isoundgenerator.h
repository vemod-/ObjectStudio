#ifndef ISOUNDGENERATOR_H
#define ISOUNDGENERATOR_H

class ISoundGenerator
{
public:
    ISoundGenerator()
    {
        ID=0;
        finished=true;
        channel=0;
        m_PitchWheel=0;
        m_PortamentoStep=0;
        m_Aftertouch=0;
        m_Modulation=1;
        m_Tune=440;
    }
    virtual ~ISoundGenerator();
    virtual void startNote(const short /*MIDINote*/, const short /*MIDIVelocity*/){}
    virtual void endNote(){}
    virtual float* getNext(){ return nullptr; }
    virtual void setPitchWheel(const int cent) { m_PitchWheel=cent; }
    virtual void addPortamento(const int steps) { m_PortamentoStep+=steps; }
    virtual void setAftertouch(const int value) { m_Aftertouch=value; }
    virtual void setModulation(const float value) { m_Modulation=value; }
    virtual void setTune(const float value=440) { m_Tune=value; }
    virtual void resetPortamento(){ m_PortamentoStep=0; }
    inline void setID(const short ch, const short pitch) {
        channel=ch;
        ID=pitch;
        resetPortamento();
    }
    inline bool matches(const short ch, const short pitch) const { return ((ID==pitch) && (channel==ch)); }
    bool finished;
    short ID;
    short channel;
protected:
    int m_PitchWheel;
    int m_PortamentoStep;
    int m_Aftertouch;
    float m_Modulation;
    float m_Tune;
};

#endif // ISOUNDGENERATOR_H
