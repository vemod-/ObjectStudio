#include "smbpitchshifter.h"
#include <QDebug>

smbPitchShifter::smbPitchShifter(double sampleRate,int stepSize,int polyphony, int oversampling)
    : m_win(MAX_FRAME_LENGTH), m_fft(MAX_FRAME_LENGTH), m_SampleRate(sampleRate), m_StepSize(stepSize), m_Polyphony(polyphony), m_OSamp(oversampling)
{
    m_Voices.resize(m_Polyphony);
    for (int i = 0; i < m_Polyphony; i++) {
        smbVoice* v = &m_Voices[i];
        v->shiftFactor = 0;
        v->newFactor = 0;
        v->velocity = 1;
        v->newVelocity = 1;
    }
    reset();
}

void smbPitchShifter::reset()
{
    // initialize our static arrays
    m_FrameSize = m_StepSize * m_OSamp;
    m_NewOSamp = m_OSamp;
    m_win.SetWindow(m_FrameSize, CSpectralWindow::wtHANNING, 0, 0, false);
    m_fft.setSize(m_FrameSize);
    m_InBuffer.reset();
    m_OutBuffer.reset();
    memset(m_LastPhase, 0, ((MAX_FRAME_LENGTH / 2) + 1) * sizeof(double));
    memset(gAnaFreq, 0, ((MAX_FRAME_LENGTH / 2) + 1) * sizeof(double));
    memset(gAnaMagn, 0, ((MAX_FRAME_LENGTH / 2) + 1) * sizeof(double));
    // set up some handy variables
    m_HalfFrameSize = m_FrameSize / 2;
    m_FreqPerBin = m_SampleRate / m_FrameSize;
    m_ExpectedPhaseDiff = 2.0 * M_PI * m_StepSize / m_FrameSize;
    m_Gain = (1.0 / sqrt((double)m_Polyphony)) / m_FrameSize * 2.0 / m_OSamp;
    for (int k = 0; k <= m_HalfFrameSize; k++) {
        m_freqPerBinV[k] = k * m_FreqPerBin;
        m_ExpPhaseDiffV[k] = k * m_ExpectedPhaseDiff;
    }
    for (int i = 0; i < m_Polyphony; i++) {
        smbVoice* v = &m_Voices[i];
        v->newFactor = v->shiftFactor;
        v->newVelocity = v->velocity;
        calcShiftVars(v);
        memset(v->sumPhase, 0, ((MAX_FRAME_LENGTH / 2) + 1) * sizeof(double));
    }
    OS_PI2 = m_FreqPerBin * m_OSamp / PI2;
    PI2_OS = PI2 / m_OSamp / m_FreqPerBin;
}

void smbPitchShifter::process(const float *indata, float* outData, float f)
{
    if (m_NewOSamp != m_OSamp) {
        m_OSamp = m_NewOSamp;
        reset();
    }
    // ================= INPUT (ringbuffer write) =================
    m_InBuffer.write(indata, m_StepSize);
    m_InBuffer.inc(m_StepSize);
    // ***************** ANALYSIS ******************* do transform
    m_fft.Forward(m_InBuffer.readWrap(m_FrameSize), m_win.WinCoeff);
    for (int k = 0; k <= m_HalfFrameSize; k++) // this is the analysis step
    {
        // compute magnitude and phase
        gAnaMagn[k] = m_fft.magn(k);
        const double phase = m_fft.phase(k);
        // compute phase difference
        double tmp = (phase - m_LastPhase[k]) - m_ExpPhaseDiffV[k]; // subtract expected phase difference
        m_LastPhase[k] = phase;
        // map delta phase into +/- Pi interval
        long qpd = tmp * M_1_PI;
        qpd += (qpd >= 0) ? qpd & 1 : -(qpd & 1);
        tmp -= M_PI * qpd;
        // get deviation from bin frequency from the +/- Pi interval  // compute the k-th partials' true frequency
        gAnaFreq[k] = (tmp * OS_PI2) + m_freqPerBinV[k]; // store frequency in analysis array
    }
    bool first = true;
    for (int i = 0; i < m_Polyphony; i++) {
        smbVoice* v = &m_Voices[i];
        if (!closeEnough(v->newFactor,v->shiftFactor)) {
            v->shiftFactor = v->newFactor;
            calcShiftVars(v);
        }
        v->velocity = v->newVelocity;
        if ((v->shiftFactor > 0.0) && (v->velocity > 0.f)) {
            if (first) {
                // ***************** PROCESSING ******************* this does the actual pitch shifting
                for (int k = 0; k <= v->maxBin; k++) {
                    const int index = v->index[k];
                    const double freq = gAnaFreq[index] * v->shiftFactor;
                    const double magn = gAnaMagn[index] * v->velocity;
                    // ***************** SYNTHESIS ******************* this is the synthesis step
                    // get true frequency from array and subtract bin mid frequency // take osamp into account // add the overlap phase advance back in
                    const double tmp = ((freq - m_freqPerBinV[k]) * PI2_OS) + m_ExpPhaseDiffV[k];
                    m_fft.polar(magn,v->sumPhase[k] += tmp,k); // get real and imag part and re-interleave
                }
                first = false;
            }
            else {
                for (int k = 0; k <= v->maxBin; k++) {
                    const int index = v->index[k];
                    const double freq = gAnaFreq[index] * v->shiftFactor;
                    const double magn = gAnaMagn[index] * v->velocity;
                    // ***************** SYNTHESIS ******************* this is the synthesis step
                    // get true frequency from array and subtract bin mid frequency // take osamp into account // add the overlap phase advance back in
                    const double tmp = ((freq - m_freqPerBinV[k]) * PI2_OS) + m_ExpPhaseDiffV[k];
                    m_fft.polarAdd(magn,v->sumPhase[k] += tmp,k); // get real and imag part and re-interleave
                }
            }
        }
        else {
            memcpy(v->sumPhase,m_LastPhase,(m_HalfFrameSize + 1) * sizeof(double));
        }
    }
    // zero negative frequencies
    m_fft.Hermitian();
    // do inverse transform
    // ================= OVERLAP-ADD (ringbuffer!) =================
    if (m_OutBuffer.wrap(m_FrameSize)) {
        m_fft.Inverse(IOBuffer::tempFrame, m_win.WinCoeff, m_Gain);
        m_OutBuffer.wrapAddFromTemp();
    }
    else {
        m_fft.InverseAdd(m_OutBuffer.current(), m_win.WinCoeff, m_Gain);
    }
    // ================= OUTPUT =================
    (isZero(f)) ? m_OutBuffer.read(outData, m_StepSize) : m_OutBuffer.add(outData, m_StepSize, f);
    m_OutBuffer.zero(m_StepSize);
    m_OutBuffer.inc(m_StepSize);
}

