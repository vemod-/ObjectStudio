#ifndef CIOSPLUGINREGISTRATION_H
#define CIOSPLUGINREGISTRATION_H

extern "C" void registerPlugin_libAmplifier();
extern "C" void registerPlugin_libAudioPlugInHost();
extern "C" void registerPlugin_libAudioUnitHost();
extern "C" void registerPlugin_libAutoTune();
extern "C" void registerPlugin_libChorus();
extern "C" void registerPlugin_libCompressor();
extern "C" void registerPlugin_libDelay();
extern "C" void registerPlugin_libDrumMachine();
extern "C" void registerPlugin_libEffectRack();
extern "C" void registerPlugin_libEnvelope();
extern "C" void registerPlugin_libEqualizer();
extern "C" void registerPlugin_libExciter();
extern "C" void registerPlugin_libFilter();
extern "C" void registerPlugin_libHarmonizer();
extern "C" void registerPlugin_libIIRFilter();
extern "C" void registerPlugin_libKarlsenFilter();
extern "C" void registerPlugin_libKeyboard();
extern "C" void registerPlugin_libLFO();
extern "C" void registerPlugin_libLimiter();
extern "C" void registerPlugin_libMacroBox();
extern "C" void registerPlugin_libMIDI2CV();
extern "C" void registerPlugin_libMIDIFile2Wave();
extern "C" void registerPlugin_libMIDIFilePlayer();
extern "C" void registerPlugin_libMixer();
extern "C" void registerPlugin_libMoogVCF();
extern "C" void registerPlugin_libNoiseGate();
extern "C" void registerPlugin_libObjectComposer();
extern "C" void registerPlugin_libPanner();
extern "C" void registerPlugin_libPhaser();
extern "C" void registerPlugin_libPitchShifter();
extern "C" void registerPlugin_libPitchTracker();
extern "C" void registerPlugin_libPlugInBox();
extern "C" void registerPlugin_libPolyBox();
extern "C" void registerPlugin_libPreamp();
extern "C" void registerPlugin_libPresetBox();
extern "C" void registerPlugin_libProgramBox();
extern "C" void registerPlugin_libRingModulator();
extern "C" void registerPlugin_libSampleAndHold();
extern "C" void registerPlugin_libSampler();
extern "C" void registerPlugin_libScope();
extern "C" void registerPlugin_libSequenser();
extern "C" void registerPlugin_libSF2Player();
extern "C" void registerPlugin_libSignalSplitter();
extern "C" void registerPlugin_libSpectrum();
extern "C" void registerPlugin_libStereoBox();
extern "C" void registerPlugin_libStereoMixer();
extern "C" void registerPlugin_libStereoSplitBox();
extern "C" void registerPlugin_libStereoSplitter();
extern "C" void registerPlugin_libToneGenerator();
extern "C" void registerPlugin_libTuner();
extern "C" void registerPlugin_libUnifilter();
extern "C" void registerPlugin_libVocoder();
extern "C" void registerPlugin_libVSTHost();
extern "C" void registerPlugin_libWaveRecorder();
extern "C" void registerPlugin_libWaveShaper();

void IOSPluginRegistration() {
 registerPlugin_libAmplifier();
 registerPlugin_libAudioPlugInHost();
 registerPlugin_libAudioUnitHost();
 registerPlugin_libAutoTune();
 registerPlugin_libChorus();
 registerPlugin_libCompressor();
 registerPlugin_libDelay();
 registerPlugin_libDrumMachine();
 registerPlugin_libEffectRack();
 registerPlugin_libEnvelope();
 registerPlugin_libEqualizer();
 registerPlugin_libExciter();
 registerPlugin_libFilter();
 registerPlugin_libHarmonizer();
 registerPlugin_libIIRFilter();
 registerPlugin_libKarlsenFilter();
 registerPlugin_libKeyboard();
 registerPlugin_libLFO();
 registerPlugin_libLimiter();
 registerPlugin_libMacroBox();
 registerPlugin_libMIDI2CV();
 registerPlugin_libMIDIFile2Wave();
 registerPlugin_libMIDIFilePlayer();
 registerPlugin_libMixer();
 registerPlugin_libMoogVCF();
 registerPlugin_libNoiseGate();
 registerPlugin_libObjectComposer();
 registerPlugin_libPanner();
 registerPlugin_libPhaser();
 registerPlugin_libPitchShifter();
 registerPlugin_libPitchTracker();
 registerPlugin_libPlugInBox();
 registerPlugin_libPolyBox();
 registerPlugin_libPreamp();
 registerPlugin_libPresetBox();
 registerPlugin_libProgramBox();
 registerPlugin_libRingModulator();
 registerPlugin_libSampleAndHold();
 registerPlugin_libSampler();
 registerPlugin_libScope();
 registerPlugin_libSequenser();
 registerPlugin_libSF2Player();
 registerPlugin_libSignalSplitter();
 registerPlugin_libSpectrum();
 registerPlugin_libStereoBox();
 registerPlugin_libStereoMixer();
 registerPlugin_libStereoSplitBox();
 registerPlugin_libStereoSplitter();
 registerPlugin_libToneGenerator();
 registerPlugin_libTuner();
 registerPlugin_libUnifilter();
 registerPlugin_libVocoder();
 registerPlugin_libVSTHost();
 registerPlugin_libWaveRecorder();
 registerPlugin_libWaveShaper();
}

#endif // CIOSPLUGINREGISTRATION_H
