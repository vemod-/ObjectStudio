#include "csfoscillator.h"

void CSFOscillator::Init(const sfData* Data, const short MidiNote, const short MidiVelo)
{
    OscData=*Data;
    Silent=true;
    EnvMod=1;
    EnvVol=1;
    pitchWheel=0;
    transpose=0;
    VolEnv.Init(OscData.shDelayVolEnv,OscData.shAttackVolEnv,OscData.shHoldVolEnv,
                OscData.shDecayVolEnv,OscData.shSustainVolEnv,OscData.shReleaseVolEnv,OscData.shAutoHoldVolEnv,
                OscData.shAutoDecayVolEnv,MidiNote);
    ModEnv.Init(OscData.shDelayModEnv,OscData.shAttackModEnv,OscData.shHoldModEnv,
                OscData.shDecayModEnv,OscData.shSustainModEnv,OscData.shReleaseModEnv,OscData.shAutoHoldModEnv,
                OscData.shAutoDecayModEnv,MidiNote);
    ModLFO.Init(OscData.shFreqModLfo,OscData.shDelayModLfo,MidiNote);
    VibLFO.Init(OscData.shFreqVibLfo,OscData.shDelayVibLfo,MidiNote);
    Filter.Init(OscData.shInitialFilterFc,OscData.shInitialFilterQ);
    Silent=false;
    double Frequency = (OscData.shOverridingRootKey!=-1) ?
                MIDIkey2Freq(OscData.shOverridingRootKey,double(m_Tune)) :
                MIDIkey2Freq(MidiNote,double(m_Tune));
    RelEndLoop=OscData.dwEndloop-OscData.dwStart;
    LoopSize=OscData.dwEndloop-OscData.dwStartloop;
    RealStart=OscData.dwStart;
    SampleSize=OscData.dwEnd-OscData.dwStart;
    VolumeFactor=float(((cB2Percent(OscData.shInstVol)*MidiVelo)/127.0)*MAXSHORTMULTIPLY);
    const double RateFactor=double(OscData.dwSampleRate)/double(presets.SampleRate);
    const byte OrigKey=OscData.shOrigKeyAndCorr >> 8;
    const char Correction=OscData.shOrigKeyAndCorr & 0xFF;
    const double SampleFrequency=MIDIkey2Freq(OrigKey-OscData.shCoarseTune,440,-Correction-OscData.shFineTune);
    PosRate=(Frequency/SampleFrequency)*RateFactor;
    SampleMode=OscData.shSampleModes;
    LeftPan=1;
    RightPan=1;
    Aftertouch=1;
    if (OscData.shPanEffectsSend > 0)
    {
        LeftPan = (OscData.shPanEffectsSend > 500) ? 0 : (500-OscData.shPanEffectsSend)*0.002;
    }
    if (OscData.shPanEffectsSend < 0)
    {
        RightPan = (OscData.shPanEffectsSend < -500) ? 0 : (OscData.shPanEffectsSend+500)*0.002;
    }
    if (OscData.shSampleModes & 4)
    {
        Stereo = (OscData.shSampleModes & 8) ? StereoR : StereoL;
    }
    else
    {
        Stereo=Mono;
    }

    /*
                        qDebug() << "dwStart " + QString::number(OscData.dwStart) +
                                " dwEnd " + QString::number(OscData.dwEnd) +
                                " dwSampleRate " + QString::number(OscData.dwSampleRate) +
                                " dwStartloop " + QString::number(OscData.dwStartloop) +
                                " dwEndloop " + QString::number(OscData.dwEndloop) +
                                " Orig Key " + QString::number((short)OrigKey) +
                                " Correction " + QString::number((short)Correction) +
                                " shCoarseTune " + QString::number(OscData.shCoarseTune) +
                                " shFineTune " + QString::number(OscData.shFineTune) +
                                " shInstVol " + QString::number(OscData.shInstVol) +
                                " SampleFreq " + QString::number(SampleFrequency) +
                                " RateFactor " + QString::number(RateFactor) +
                                " InitialFilterFc " + QString::number(OscData.shInitialFilterFc) +
                                " Pan " + QString::number(OscData.shPanEffectsSend) +
                                " Mode " + QString::number(OscData.shSampleModes) +
                                " Link " + QString::number(OscData.shSampleLink) +
                                " VolumeFactor " + QString::number(VolumeFactor) +
                                " Override root key " + QString::number(OscData.shOverridingRootKey)
                        ;
                        */
}

void CSFOscillator::Modulate()
{
    const float ModLFOVal=ModLFO.GetNext()*Aftertouch;
    const float VibLFOVal=VibLFO.GetNext()*Aftertouch;
    const float ModEnvVal=ModEnv.GetNext();
    const float VolEnvVal=VolEnv.GetNext();
    EnvMod=cent2Factor((ModEnvVal*OscData.shModEnvToPitch)+(ModLFOVal*OscData.shModLfoToPitch)+(VibLFOVal*OscData.shVibLfoToPitch) + pitchWheel+transpose);
    EnvVol=VolEnvVal*cB2Percentf(ModLFOVal*OscData.shModLfoToVolume);
    Filter.SetAmount(cent2Factorf((ModEnvVal*OscData.shModEnvToFilterFc)+(ModLFOVal*OscData.shModLfoToFilterFc)));
    if (VolEnv.isSilent()) Silent=true;
}

void CSFOscillator::Start()
{
    Position=0;
    VolEnv.Start();
    ModEnv.Start();
    ModLFO.Start();
    VibLFO.Start();
}

void CSFOscillator::Finish()
{
    VolEnv.Finish();
    ModEnv.Finish();
}
