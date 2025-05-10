#include "smbpitchshifter.h"
//#include "softsynthsdefines.h"

smbPitchShifter::smbPitchShifter(double sampleRate,long fftFrameSize, long osamp)
    : InFIFO(48000), OutFIFO(48000), m_SampleRate(sampleRate), m_FrameSize(fftFrameSize), m_OSamp(osamp)
{
    QMutexLocker locker(&mutex);
    memset(m_shiftFactor,0,MAX_POLYPHONY*sizeof(double));
    memset(m_Scale,0,MAX_POLYPHONY*sizeof(float));
    reset();
}

void smbPitchShifter::reset()
{
    QMutexLocker locker(&mutex);
    // initialize our static arrays
    m_win.SetWindow(m_FrameSize, CSpectralWindow::wtHANNING, 0, 0, false);
    m_fft.setSize(m_FrameSize);
    memset(m_OutputAccum, 0, 2*MAX_FRAME_LENGTH*sizeof(float));
    memset(m_LastPhase, 0, (MAX_FRAME_LENGTH/2+1)*sizeof(double));
    memset(m_SumPhase, 0, MAX_POLYPHONY*(MAX_FRAME_LENGTH/2+1)*sizeof(double));
    memset(gAnaFreq, 0, (MAX_FRAME_LENGTH/2+1)*sizeof(double));
    memset(gAnaMagn, 0, (MAX_FRAME_LENGTH/2+1)*sizeof(double));
    memset(gSynFreq, 0, (MAX_FRAME_LENGTH/2+1)*sizeof(double));
    memset(gSynMagn, 0, (MAX_FRAME_LENGTH/2+1)*sizeof(double));
    //gRover = 0;
    // set up some handy variables
    m_HalfFrameSize = m_FrameSize/2;
    m_StepSize = m_FrameSize/m_OSamp;
    m_FreqPerBin = m_SampleRate/m_FrameSize;
    m_ExpectedPhaseDiff = 2.0*M_PI*m_StepSize/m_FrameSize;
    m_InFifoLatency = 0; //m_FrameSize-m_StepSize;
    m_PolyFactor = 1.0/sqrt(m_Polyphony);
    for (int k = 0; k <= m_HalfFrameSize; k++)
    {
        m_freqPerBinV[k] = k * m_FreqPerBin;
        m_ExpPhaseDiffV[k] = k * m_ExpectedPhaseDiff;
    }
    OS_PI2 = m_FreqPerBin * m_OSamp / PI2;
    PI2_OS = PI2 / m_OSamp / m_FreqPerBin;
    InFIFO.setStepSize(m_StepSize);
    InFIFO.clear(m_InFifoLatency);
    OutFIFO.setStepSize(m_StepSize);
    OutFIFO.clear(m_InFifoLatency);
}

float* smbPitchShifter::process(long numSampsToProcess, float *indata)
{
    QMutexLocker locker(&mutex);
    InFIFO.write(indata,numSampsToProcess);
    // main processing loop
    while (InFIFO.isAvail(m_FrameSize))
    {
        // ***************** ANALYSIS ******************* do transform
        m_fft.Forward(InFIFO.read(m_FrameSize), m_win.WinCoeff);

        for (uint k = 0; k <= m_HalfFrameSize; k++) // this is the analysis step
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
        for (uint v = 0; v < m_Polyphony; v++)
        {
            if (m_shiftFactor[v] > 0.0)
            {
                if (m_Scale[v] > 0.f)
                {
                    // ***************** PROCESSING ******************* this does the actual pitch shifting
                    for (uint k = 0; k <= m_HalfFrameSize; k++)
                    {
                        const long index = lround(k / m_shiftFactor[v]);
                        if (index <= m_HalfFrameSize)
                        {
                            gSynMagn[k] = gAnaMagn[index];
                            gSynFreq[k] = gAnaFreq[index] * m_shiftFactor[v];
                        }
                        else
                        {
                            gSynMagn[k] = 0;
                            gSynFreq[k] = 0;
                        }
                    }
                    // ***************** SYNTHESIS ******************* this is the synthesis step
                    for (uint k = 0; k <= m_HalfFrameSize; k++)
                    {
                        // get true frequency from array and subtract bin mid frequency // take osamp into account // add the overlap phase advance back in
                        const double tmp = ((gSynFreq[k] - m_freqPerBinV[k]) * PI2_OS) + m_ExpPhaseDiffV[k];
                        m_fft.polar(gSynMagn[k],m_SumPhase[v][k]+=tmp,k); // get real and imag part and re-interleave
                    }
                    // zero negative frequencies
                    m_fft.Hermitian();
                    // do inverse transform
                    m_fft.Inverse(m_OutputAccum,m_win.WinCoeff,m_Scale[v]*m_PolyFactor/m_FrameSize*2.0/m_OSamp);
                }
                else
                {
                    memcpy(m_SumPhase[v],m_LastPhase,(m_HalfFrameSize+1)*sizeof(double));
                }
            }
            else
            {
                memcpy(m_SumPhase[v],m_LastPhase,(m_HalfFrameSize+1)*sizeof(double));
            }
        }
        OutFIFO.write(m_OutputAccum, m_StepSize);
        // shift accumulator
        memmove(m_OutputAccum, m_OutputAccum + m_StepSize, m_FrameSize * sizeof(float));
    }
    if (OutFIFO.isAvail(numSampsToProcess)) return OutFIFO.read(numSampsToProcess);
    return nullptr;
}

smbPitchShifterOld::smbPitchShifterOld(double sampleRate,long fftFrameSize, long osamp)
{
    //gRover = 0;
    m_SampleRate=sampleRate;
    m_shiftFactor=1;
    m_OSamp=osamp;
    m_FrameSize=fftFrameSize;
    reset();
}

void smbPitchShifterOld::smbFft(double *fftBuffer, long fftFrameSize, long sign)
/*
        FFT routine, (C)1996 S.M.Bernsee. Sign = -1 is FFT, 1 is iFFT (inverse)
        Fills fftBuffer[0...2*fftFrameSize-1] with the Fourier transform of the
        time domain data in fftBuffer[0...2*fftFrameSize-1]. The FFT array takes
        and returns the cosine and sine parts in an interleaved manner, ie.
        fftBuffer[0] = cosPart[0], fftBuffer[1] = sinPart[0], asf. fftFrameSize
        must be a power of 2. It expects a complex input signal (see footnote 2),
        ie. when working with 'common' audio signals our input signal has to be
        passed as {in[0],0.,in[1],0.,in[2],0.,...} asf. In that case, the transform
        of the frequencies of interest is in fftBuffer[0...fftFrameSize].
*/
{
    long i, bitm, j;
    for (i = 2; i < 2*fftFrameSize-2; i += 2)
    {
        for (bitm = 2, j = 0; bitm < 2*fftFrameSize; bitm <<= 1)
        {
            if (i & bitm) j++;
            j <<= 1;
        }
        if (i < j)
        {
            double* p1 = fftBuffer+i;
            double* p2 = fftBuffer+j;
            double temp = *p1; *(p1++) = *p2;
            *(p2++) = temp;
            temp = *p1;
            *p1 = *p2;
            *p2 = temp;
        }
    }
    for (long k = 0, le = 2; k < long(log(fftFrameSize)/log(2.0)); k++)
    {
        le <<= 1;
        const long le2 = le>>1;
        double ur = 1.0;
        double ui = 0.0;
        const double arg = M_PI / (le2>>1);
        const double wr = cos(arg);
        const double wi = sign*sin(arg);
        for (long j = 0; j < le2; j += 2)
        {
            double* p1r = fftBuffer+j;
            double* p1i = p1r+1;
            double* p2r = p1r+le2;
            double* p2i = p2r+1;
            double tr;
            for (i = j; i < 2*fftFrameSize; i += le)
            {
                tr = *p2r * ur - *p2i * ui;
                double ti = *p2r * ui + *p2i * ur;
                *p2r = *p1r - tr;
                *p2i = *p1i - ti;
                *p1r += tr;
                *p1i += ti;
                p1r += le;
                p1i += le;
                p2r += le;
                p2i += le;
            }
            tr = ur*wr - ui*wi;
            ui = ur*wi + ui*wr;
            ur = tr;
        }
    }
}


// -----------------------------------------------------------------------------------------------------------------

/*
    12/12/02, smb

    PLEASE NOTE:

    There have been some reports on domain errors when the atan2() function was used
    as in the above code. Usually, a domain error should not interrupt the program flow
    (maybe except in Debug mode) but rather be handled "silently" and a global variable
    should be set according to this error. However, on some occasions people ran into
    this kind of scenario, so a replacement atan2() function is provided here.

    If you are experiencing domain errors and your program stops, simply replace all
    instances of atan2() with calls to the smbAtan2() function below.
*/
double smbPitchShifterOld::smbAtan2(double x, double y)
{
    const double signx= (x > 0.) ? 1:-1;
    if (x == 0.) return 0.;
    if (y == 0.) return signx * M_PI / 2.;
    return atan2(x, y);
}

void smbPitchShifterOld::reset()
{
    const double PI2=M_PI*2;
    // initialize our static arrays
    memset(gInFIFO, 0, MAX_FRAME_LENGTH*sizeof(float));
    memset(gOutFIFO, 0, MAX_FRAME_LENGTH*sizeof(float));
    memset(gFFTworksp, 0, 2*MAX_FRAME_LENGTH*sizeof(double));
    memset(gLastPhase, 0, MAX_FRAME_LENGTH*sizeof(double)/2+1);
    memset(gSumPhase, 0, MAX_FRAME_LENGTH*sizeof(double)/2+1);
    memset(gOutputAccum, 0, 2*MAX_FRAME_LENGTH*sizeof(double));
    memset(gAnaFreq, 0, MAX_FRAME_LENGTH*sizeof(double));
    memset(gAnaMagn, 0, MAX_FRAME_LENGTH*sizeof(double));
    fftFrameSize2 = m_FrameSize/2;
    stepSize = m_FrameSize/m_OSamp;
    freqPerBin = m_SampleRate/m_FrameSize;
    expct = 2*M_PI*stepSize/m_FrameSize;
    inFifoLatency = m_FrameSize-stepSize;
    gRover = inFifoLatency;
    for (long k=0;k<m_FrameSize;k++) m_WindowFactor[k]=-0.5*cos(PI2*k/m_FrameSize)+0.5;
}

void smbPitchShifterOld::process(long numSampsToProcess, const float *indata, float *outdata)
{
    // set up some handy variables
    const double PI2=M_PI*2;
    //if (gRover == 0) gRover = inFifoLatency;

    // main processing loop
    for (long i = 0; i < numSampsToProcess; i++){

        // As long as we have not yet collected enough data just read in
        gInFIFO[gRover] = indata[i];
        outdata[i] = gOutFIFO[gRover-inFifoLatency];
        gRover++;

        // now we have enough data for processing
        if (gRover >= m_FrameSize) {
            gRover = inFifoLatency;

            // do windowing and re,im interleave
            for (long k = 0; k < m_FrameSize;k++) {
                gFFTworksp[2*k] = gInFIFO[k] * m_WindowFactor[k];
                gFFTworksp[2*k+1] = 0.0;
            }


            // ***************** ANALYSIS *******************
            // do transform
            smbFft(gFFTworksp, m_FrameSize, -1);

            // this is the analysis step
            for (long k = 0; k <= fftFrameSize2; k++) {

                // de-interlace FFT buffer
                const double real = gFFTworksp[2*k];
                const double imag = gFFTworksp[2*k+1];

                // compute magnitude and phase
                const double magn = 2.0 * sqrt(pow(real,2.0) + pow(imag,2.0));
                const double phase = atan2(imag,real);

                // compute phase difference
                double tmp = phase - gLastPhase[k];
                gLastPhase[k] = phase;

                // subtract expected phase difference
                tmp -= k*expct;

                // map delta phase into +/- Pi interval
                long qpd = tmp / M_PI;
                if (qpd >= 0) qpd += qpd & 1;
                else qpd -= qpd & 1;
                tmp -= M_PI * qpd;

                // get deviation from bin frequency from the +/- Pi interval
                tmp = m_OSamp * tmp / PI2;

                // compute the k-th partials' true frequency
                tmp = k * freqPerBin + tmp * freqPerBin;

                // store magnitude and true frequency in analysis arrays
                gAnaMagn[k] = magn;
                gAnaFreq[k] = tmp;

            }



            // ***************** PROCESSING *******************
            // this does the actual pitch shifting
            //memset(gSynMagn, 0, m_FrameSize*sizeof(float));
            //memset(gSynFreq, 0, m_FrameSize*sizeof(float));
            for (long k = 0; k <= fftFrameSize2; k++) {
                const long index = lround(k / m_shiftFactor);
                if (index <= fftFrameSize2) {
                    gSynMagn[k] = gAnaMagn[index];
                    gSynFreq[k] = gAnaFreq[index] * m_shiftFactor;
                }
                else
                {
                    gSynMagn[k]=0;
                    gSynFreq[k]=0;
                }
            }

            // ***************** SYNTHESIS *******************
            // this is the synthesis step
            for (long k = 0; k <= fftFrameSize2; k++) {

                // get magnitude and true frequency from synthesis arrays
                const double magn = gSynMagn[k];
                double tmp = gSynFreq[k];

                // subtract bin mid frequency
                tmp -= k*freqPerBin;

                // get bin deviation from freq deviation
                tmp /= freqPerBin;

                // take osamp into account
                tmp = PI2*tmp/m_OSamp;

                // add the overlap phase advance back in
                tmp += k*expct;

                // accumulate delta phase to get bin phase
                gSumPhase[k] += tmp;
                const double phase = gSumPhase[k];

                // get real and imag part and re-interleave
                gFFTworksp[2*k] = magn*cos(phase);
                gFFTworksp[2*k+1] = magn*sin(phase);
            }

            // zero negative frequencies
            for (long k = m_FrameSize+2; k < 2*m_FrameSize; k++) gFFTworksp[k] = 0.0;

            // do inverse transform
            smbFft(gFFTworksp, m_FrameSize, 1);

            // do windowing and add to output accumulator
            const double f = 2.0 / (fftFrameSize2*m_OSamp);
            for(long k = 0; k < m_FrameSize; k++) {
                gOutputAccum[k] += m_WindowFactor[k]*gFFTworksp[2*k]*f;
            }
            for (long k = 0; k < stepSize; k++) gOutFIFO[k] = gOutputAccum[k];

            // shift accumulator
            memmove(gOutputAccum, gOutputAccum+stepSize, m_FrameSize*sizeof(double));

            // move input FIFO
            for (long k = 0; k < inFifoLatency; k++) gInFIFO[k] = gInFIFO[k+stepSize];
        }
    }
}
