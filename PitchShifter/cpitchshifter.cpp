#include "cpitchshifter.h"

CPitchShifter::CPitchShifter() : smb(presets.SampleRate, presets.ModulationRate)
{
}

void CPitchShifter::init(const int Index, QWidget* MainWindow) {
    //ModFactor=0;
    m_Name="PitchShifter";
    IDevice::init(Index,MainWindow);
    addJackMonoIn();
    addJackModulationIn();
    addJackMonoOut(monoout);
    addParameterTranspose("Shift");
    startParameterGroup();
    addParameterSelect("Oversampling","1§2§4§8§16§32",3);
    //addParameterSelect("Framesize","128§256§512§1048§2048§4096",4);
    endParameterGroup();
    makeParameterGroup(2,"Tune",Qt::green);
    addParameterPercent();
    addParameter(CParameterVars::Numeric,"Tune","cent",-100,100,0,"",0);
    //addParameterTune();
    addParameterPercent("Effect",100);
    Modulator.init(m_Jacks[modulationin],m_Parameters[pnModulation],m_Parameters[pnTune],CVoltageModulator::Cents);
    updateDeviceParameter();
}

CAudioBuffer *CPitchShifter::getNextA(const int /*ProcIndex*/) {
    const CMonoBuffer* InBuffer = FetchAMono(monoin);
    if (!InBuffer->isValid()) return nullptr;
    CAudioBuffer* OutBuffer = MonoBuffer(monoout);
    OutBuffer->writeBuffer(InBuffer,m_Parameters[pnMix]->DryValue);
    const long ModCent = Modulator.execCent() + (m_Parameters[pnShift]->Value * 100);
    if (!isZero(m_Parameters[pnMix]->PercentValue)) {
        smb.process(cent2Factor(ModCent), InBuffer->data(), OutBuffer->data(), m_Parameters[pnMix]->PercentValue);
    }
    return m_AudioBuffers[monoout];
}

void CPitchShifter::updateDeviceParameter(const CParameter* /*p*/) {
    smb.setOverSampling(1 << m_Parameters[pnOverSampling]->Value);
}
