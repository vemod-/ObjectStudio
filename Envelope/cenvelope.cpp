#include "cenvelope.h"
#include "cenvelopeform.h"

CEnvelope::CEnvelope() : VolumeFactor(0) {}

void CEnvelope::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackModulationIn("Trigger In");
    addJackModulationOut(jnOut,"Out");
    startParameterGroup();
    addParameterTime("Delay Time");
    addParameterTime("Attack Time");
    addParameterTime("Hold Time");
    addParameterTime("Decay Time");
    addParameterPercent("Sustain Level",100);
    addParameterTime("Release Time");
    endParameterGroup();
    addParameterLevel();
    addParameterSelect("Input Mode","Analog§Binary");
    m_Form=new CEnvelopeForm(this,MainWindow);
    updateDeviceParameter();
}

float CEnvelope::getNext(int /*ProcIndex*/)
{
    return ADSR.GetVol(Fetch(jnTriggerIn))*VolumeFactor;
}

void inline CEnvelope::updateDeviceParameter(const CParameter* /*p*/)
{
    ADSR.AP.Delay=m_Parameters[pnDelayTime]->Value;
    ADSR.AP.Attack=m_Parameters[pnAttackTime]->Value;
    ADSR.AP.Hold=m_Parameters[pnHoldTime]->Value;
    ADSR.AP.Decay=m_Parameters[pnDecayTime]->Value;
    ADSR.AP.Sustain=m_Parameters[pnSustainLevel]->Value;
    ADSR.AP.Release=m_Parameters[pnReleaseTime]->Value;
    ADSR.Mode=m_Parameters[pnMode]->Value;
    VolumeFactor=m_Parameters[pnVolume]->PercentValue;
    ADSR.calcParams();
}

