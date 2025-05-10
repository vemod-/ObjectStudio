CONFIG += sdk_no_version_check

LIBS += \
    $$LIBSDIR -lAmplifier \
    $$LIBSDIR -lMIDIFilePlayer \
    -lToneGenerator \
    -lDelay \
    -lLFO \
    -lFilter \
    -lPanner \
    -lEnvelope \
    -lMacroBox \
    -lMIDI2CV \
    -lPolyBox \
    -lDrumMachine \
    -lSF2Player \
    -lSequenser \
    -lRingModulator \
    -lLimiter \
    -lExciter \
    -lMixer \
    -lPitchShifter \
    -lChorus \
    -lKarlsenFilter \
    -lMoogVCF \
    -lNoiseGate \
    -lPhaser \
    -lVSTHost \
    -lAudioUnitHost \
    -lEqualizer \
    ##-lAudacityProject \
    -lPitchTracker \
    -lScope \
    -lUnifilter \
    -lWaveShaper \
    -lStereoBox \
    -lStereoSplitBox \
    -lStereoSplitter \
    -lCompressor \
    -lPlugInBox \
    -lSampleAndHold \
    -lPreamp \
    -lProgramBox \
    -lPresetBox \
    -lStereoMixer \
    -lWaveRecorder \
    -lSampler \
    -lMIDIFile2Wave \
    -lTuner \
    -lAutoTune \
    -lVocoder \
    -lIIRFilter \
    -lSpectrum \
    -lHarmonizer \
    -lKeyboard \
    -lAudioPlugInHost \
    -lEffectRack \
    -lSignalSplitter

