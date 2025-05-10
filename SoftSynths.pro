cache()

include(SoftSynthsArcitecture.pri)

ios {
    QMAKE_RPATHDIR += $$OUT_PWD
}

QT -= network opengl sql svg xml xmlpatterns qt3support
QT += core gui widgets
QT += concurrent

macx {
    LIBS += -framework AppKit
}

ios {
    LIBS += -framwork UIKit
}

ios {
    LIBS += -framework CoreMidi -framework CoreAudio -framework CoreFoundation
    LIBS += -framework AudioToolbox
    LIBS += -framework AVFoundation
}

macx {
    LIBS += -framework CoreMidi -framework CoreAudio -framework CoreFoundation
    LIBS += -framework AudioUnit -framework AudioToolbox -framework CoreAudioKit
}

TEMPLATE = subdirs

SUBDIRS = PluginLoader \
    SoftSynthsClasses \
    SoftSynthsWidgets \
    WaveBank \
    WaveGenerator \
    SF2Generator \
    MIDIFileReader \
    AudioPlugInHost \
    Amplifier \
    MIDIFilePlayer \
    ToneGenerator \
    Delay \
    LFO \
    Filter \
    Panner \
    Envelope \
    MacroBox \
    MIDI2CV \
    PolyBox \
    DrumMachine \
    SF2Player \
    Sequenser \
    RingModulator \
    Limiter \
    Exciter \
    Mixer \
    PitchShifter \
    Chorus \
    KarlsenFilter \
    MoogVCF \
    NoiseGate \
    Phaser \
    VSTHost \
    AudioUnitHost \
    Equalizer \
    #AudacityProject \
    PitchTracker \
    Scope \
    Unifilter \
    WaveShaper \
    StereoBox \
    StereoSplitBox \
    StereoSplitter \
    Compressor \
    PlugInBox \
    SampleAndHold \
    Preamp \
    ProgramBox \
    PresetBox \
    StereoMixer \
    WaveRecorder \
    Sampler \
    MIDIFile2Wave \
    Tuner \
    AutoTune \
    Vocoder \
    IIRFilter \
    Spectrum \
    Harmonizer \
    Keyboard \
    SignalSplitter \
    EffectRack \
    ObjectComposer \
    RtAudioBuffer


