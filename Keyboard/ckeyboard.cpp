#include "ckeyboard.h"
#include "ckeyboardform.h"

CKeyboard::CKeyboard()
{
    pitchBend=0;
    pbOld=0;
    mod1=0;
    modOld1=0;
    mod2=0;
    modOld2=0;
}

void CKeyboard::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackMIDIOut(jnMIDI);
    addJackModulationOut(jnFrequency,"Frequency");
    addJackModulationOut(jnTrigger,"Trigger Out");
    addJackModulationOut(jnModulationOut1,"Modulation 1 Out");
    addJackModulationOut(jnModulationOut2,"Modulation 2 Out");
    addJackModulationIn("Modulation 1 In");
    addJackModulationIn("Modulation 2 In");
    addParameterMIDIChannel();
    addParameterTune();
    addParameter(CParameter::Numeric, "Pitch Bend Range","",1,24,0,"",2);
    startParameterGroup("Mod Wheels");
    addParameterSelect("Mod 1 Mode","Thru§Voltage",0);
    addParameterSelect("Mod 2 Mode","Thru§Voltage",0);
    endParameterGroup();
    m_Form=new CKeyboardForm(this,MainWindow);
    updateDeviceParameter();
}

void inline CKeyboard::updateDeviceParameter(const CParameter* /*p*/)
{
}

CMIDIBuffer* CKeyboard::getNextP(const int /*ProcIndex*/)
{
    return &MIDIBuffer;
}

float CKeyboard::getNext(const int ProcIndex)
{
    if (!notesDown.empty())
    {
        if (ProcIndex == jnFrequency) return MIDIkey2voltagef(notesDown.last(), m_Parameters[pnTune]->PercentValue, pitchBend);
        if (ProcIndex == jnTrigger) return 1;
        if (ProcIndex == jnModulationOut1)
        {
            return (m_Parameters[pnMod1Mode]->Value) ? mod1 : mod1 * Fetch(jnModulationIn1);
        }
        if (ProcIndex == jnModulationOut2)
        {
            return (m_Parameters[pnMod2Mode]->Value) ? mod2 : mod2 * Fetch(jnModulationIn2);
        }

    }
    return 0;
}

void CKeyboard::tick()
{
    auto* f = FORMFUNC(CKeyboardForm);
    MIDIBuffer.clear();
    const int pb = f->pitchWheel();
    if (pb != pbOld)
    {
        pbOld = pb;
        pitchBend = pb*m_Parameters[pnPitchBendRange]->Value;
        const auto b=ushort((100+pb)*0x3FFF*0.005);
        MIDIBuffer.append(0xE0,to14bitLSB(b),to14bitMSB(b));
    }
    const int md1 = f->modWheel1();
    if (md1 != modOld1)
    {
        modOld1 = md1;
        mod1 = md1*0.01f;
        MIDIBuffer.append(0xB0,0x01,byte(mod1*0x7F));
    }
    const int md2 = f->modWheel2();
    if (md2 != modOld2)
    {
        modOld2 = md2;
        mod2 = md2*0.01f;
    }
    for (const int& i : std::as_const(notesOn)) MIDIBuffer.append(0x90,i,0x7F);
    for (const int& i : std::as_const(notesOff)) MIDIBuffer.append(0x80,i,0x00);
    notesOn.clear();
    notesOff.clear();
    IDevice::tick();
}
