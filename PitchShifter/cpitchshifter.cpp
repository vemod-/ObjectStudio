#include "cpitchshifter.h"

CPitchShifter::CPitchShifter() : smb(presets.SampleRate)
{
}

void CPitchShifter::init(const int Index, QWidget* MainWindow) {
    //ModFactor=0;
    m_Name="PitchShifter";
    IDevice::init(Index,MainWindow);
    addJackWaveIn();
    addJackModulationIn();
    addJackWaveOut(jnOut);
    addParameterTranspose("Shift");
    startParameterGroup();
    addParameterSelect("Oversampling","1§2§4§8§16§32",3);
    addParameterSelect("Framesize","128§256§512§1048§2048§4096",4);
    endParameterGroup();
    makeParameterGroup(2,"Tune",Qt::green);
    addParameterPercent();
    addParameter(CParameter::Numeric,"Tune","cent",-100,100,0,"",0);
    addParameterPercent("Effect",100);
    Modulator.init(m_Jacks[jnModulation],m_Parameters[pnModulation],m_Parameters[pnTune],CVoltageModulator::Fine);
    updateDeviceParameter();
}

CAudioBuffer *CPitchShifter::getNextA(const int ProcIndex) {
    const CMonoBuffer* InBuffer = FetchAMono(jnIn);
    if (!InBuffer->isValid()) return nullptr;
    CMonoBuffer* OutBuffer=MonoBuffer(jnOut);
    OutBuffer->writeBuffer(InBuffer,Dry);
    const long ModCent = Modulator.execCent() + PitchShift;
    if (!isZero(Wet)) OutBuffer->addBuffer(smb.process(cent2Factor(ModCent), OutBuffer->size(), InBuffer->data()),Wet);
    return m_AudioBuffers[ProcIndex];
}

void CPitchShifter::updateDeviceParameter(const CParameter* /*p*/) {
    PitchShift = m_Parameters[pnShift]->Value*100;	// convert semitones to factor
    smb.setOverSampling(1 << m_Parameters[pnOverSampling]->Value);
    smb.setFrameSize(128 << m_Parameters[pnFrameSize]->Value);
    Wet=m_Parameters[pnMix]->PercentValue;
    Dry=m_Parameters[pnMix]->DryValue;
}
