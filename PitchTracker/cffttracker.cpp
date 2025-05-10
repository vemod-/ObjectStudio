#include "cffttracker.h"
//#include <QStringList>
#include <QDebug>

static QStringList noteNames = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

void computeSecondOrderLowPassParameters( float srate, float f, float *a, float *b )
{
    float a0;
    float w0 = 2 * M_PI * f/srate;
    float cosw0 = cosf(w0);
    float sinw0 = sinf(w0);
    //float alpha = sinw0/2;
    float alpha = sinw0/2 * sqrtf(2);

    a0   = 1 + alpha;
    a[0] = (-2*cosw0) / a0;
    a[1] = (1 - alpha) / a0;
    b[0] = ((1-cosw0)/2) / a0;
    b[1] = ( 1-cosw0) / a0;
    b[2] = b[0];
}
float processSecondOrderFilter( float x, float *mem, float *a, float *b )
{
    float ret = b[0] * x + b[1] * mem[0] + b[2] * mem[1]
            - a[0] * mem[2] - a[1] * mem[3] ;

    mem[1] = mem[0];
    mem[0] = x;
    mem[3] = mem[2];
    mem[2] = ret;

    return ret;
}

inline double calcParabol(uint index, double v1, double v2, double v3)
{
    double alpha = v1/0.434294481;
    double beta = v2/0.434294481;
    double gamma = v3/0.434294481;
    return index + (gamma-alpha)/(2*((2*beta)-gamma-alpha));
}

inline long cv(double val, double factor=1.0)
{
    return lround((val*5)/factor);
}

double findRoot1(double val1, double val2)
{
    if (cv(val1,2) == cv(val2,5))
    {
        qDebug() << "Fifth" << val1 << val2 << val1/2.0;
        return val1/2.0;
    }
    if (cv(val1,3) == cv(val2,4))
    {
        qDebug() << "Fourth" << val1 << val2 << val1/3.0;
        return val1/3.0;
    }
    if (cv(val1,4) == cv(val2,5))
    {
        qDebug() << "Major third" << val1 << val2 << val1/4.0;
        return val1/4.0;
    }
    if (cv(val1,5) == cv(val2,6))
    {
        qDebug() << "Minor third" << val1 << val2 << val1/5.0;
        return val1/5.0;
    }
    if (cv(val1,6) == cv(val2,7))
    {
        qDebug() << "Minor third" << val1 << val2 << val1/6.0;
        return val1/6.0;
    }
    //1 missing
    if (cv(val1) == cv(val2,3))
    {
        qDebug() << "Octave + fifth" << val1 << val2 << val1;
        return val1;
    }
    if (cv(val1,3) == cv(val2,5))
    {
        qDebug() << "major sixth" << val1 << val2 << val1/3.0;
        return val1/3.0;
    }
    if (cv(val1,5) == cv(val2,7))
    {
        qDebug() << "Tritonus" << val1 << val2 << val1/5.0;
        return val1/5.0;
    }
    //2 missing
    if (cv(val1,2) == cv(val2,5))
    {
        qDebug() << "Decima" << val1 << val2 << val1/2.0;
        return val1/2.0;
    }
    if (cv(val1,4) == cv(val2,7))
    {
        qDebug() << "Septima" << val1 << val2 << val1/4.0;
        return val1/4.0;
    }
    return 0;
}

double octaveCheck(double val1, double val2)
{
    if (cv(val1) == cv(val2,2))
    {
        qDebug() << "Octave" << val1 << val2 << val1;
        return val1;
    }
    if (cv(val1) == cv(val2,4))
    {
        qDebug() << "Double octave" << val1 << val2 << val1;
        return val1;
    }
    if (cv(val1) == cv(val2,8))
    {
        qDebug() << "Tripple octave" << val1 << val2 << val1;
        return val1;
    }
    return 0;
}

double checkTwo(double val1, double val2)
{
    double i=0;
    if (val2 > val1)
    {
        i = findRoot1(val1,val2);
        if (!(i > 0)) i=octaveCheck(val1,val2);
    }
    else
    {
        i = findRoot1(val2,val1);
        if (!(i > 0)) i=octaveCheck(val2,val1);
    }
    return i;
}


CFFTTracker::CFFTTracker(long sampleRate, ulong fftFrameSize) : InFIFO(48000)
{
    m_SampleRate=sampleRate;
    m_FrameSize=fftFrameSize;
    m_win.SetWindow(m_FrameSize, CSpectralWindow::wtHANNING, 0, 0, false);
    m_fft.setSize(m_FrameSize);
    InFIFO.setStepSize(m_FrameSize/8);
    InFIFO.clear(0);

    computeSecondOrderLowPassParameters( m_SampleRate, 330, a, b );
    mem1[0] = 0; mem1[1] = 0; mem1[2] = 0; mem1[3] = 0;
    mem2[0] = 0; mem2[1] = 0; mem2[2] = 0; mem2[3] = 0;
    filterBuffer.resize(m_FrameSize);
}

void CFFTTracker::process(float* inbuffer, uint samplecount)
{
    /*
    for( int j=0; j<samplecount; ++j ) {
        filterBuffer[j] = processSecondOrderFilter( inbuffer[j], mem1, a, b );
        filterBuffer[j] = processSecondOrderFilter( filterBuffer[j], mem2, a, b );
    }
    */
    InFIFO.write(inbuffer,samplecount);
    // main processing loop
    while (InFIFO.isAvail(m_FrameSize))
    {
        // ***************** ANALYSIS ******************* do transform
        m_fft.Forward(InFIFO.read(m_FrameSize), m_win.WinCoeff);

        //find the peaks
        double maxVal = -1;
        uint maxIndex = 0;
        double oldVal5 = 0;
        double oldVal4 = 0;
        double oldVal3 = 0;
        double oldVal2 = 0;
        double oldVal1 = 0;
        peaks.clear();
        for( uint j=0; j<m_FrameSize/2; ++j )
        {
            const double v = m_fft.magn(j);
            oldVal5 = oldVal4;
            oldVal4 = oldVal3;
            oldVal3 = oldVal2;
            oldVal2 = oldVal1;
            oldVal1 = v;
            if (oldVal3 > 1)
            {
                if (j > 4)
                {
                    if (oldVal5 < oldVal4*0.9)
                    {
                        if (oldVal4 < oldVal3*0.9)
                        {
                            if (oldVal2 < oldVal3*0.9)
                            {
                                if (oldVal1 < oldVal2*0.9)
                                {
                                    peaks.push_back(calcParabol(j-2,oldVal4,oldVal3,oldVal2));
                                    if (oldVal3 > maxVal)
                                    {
                                        maxVal = oldVal3;
                                        maxIndex = peaks.size()-1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        if (!peaks.empty())
        {
            if (maxVal > 10)
            {
                double i = 0;
                double maxPeak = peaks[maxIndex];
                qDebug() << peaks;
                if (peaks.size() == 1)
                {
                    qDebug() << "One and only";
                    i = checkTwo(prevIndex, maxPeak);
                    if (!(i > 0)) i = maxPeak;
                }
                else if (peaks.size() > 60)
                {
                    qDebug() << "Too many peaks";
                    i = checkTwo(prevIndex, maxPeak);
                    if (!(i > 0)) i = maxPeak;
                }
                else
                {
                    if (maxIndex == 0)
                    {
                        i=checkTwo(peaks[0],peaks[1]);
                    }
                    else if (maxIndex == peaks.size()-1)
                    {
                        i=checkTwo(peaks[peaks.size()-2],maxPeak);
                    }
                    if (peaks.size() > 2)
                    {
                        for (uint j = maxIndex; j < peaks.size()-2; j++)
                        {
                            if (i > 0) break;
                            i=findRoot1(peaks[j],peaks[j+1]);
                        }
                        for (uint j = maxIndex; j >= 1; j--)
                        {
                            if (i > 0) break;
                            i=findRoot1(peaks[j-1],peaks[j]);
                        }
                    }
                    for (uint j = 0; j < peaks.size()-2; j++)
                    {
                        if (i > 0) break;
                        i=octaveCheck(peaks[j],peaks[j+1]);
                    }
                    if (i > maxPeak) i = maxPeak;
                }
                if (i > 0)
                {
                    prevIndex = i;
                }
                else
                {
                    qDebug() << "Brute force";
                    i = checkTwo(prevIndex, maxPeak);
                    if (!(i > 0)) i = checkTwo(prevIndex, peaks[0]);
                    if (!(i > 0))
                    {
                        if (fabs(maxPeak - prevIndex) > fabs(peaks[0] - prevIndex))
                        {
                            i = peaks[0];
                        }
                        else
                        {
                            i = maxPeak;
                        }
                    }
                }
                const double freq = ( m_SampleRate * i ) / double ( m_FrameSize );
                if (freq > 10)
                {
                    const long fNote = freq2Cent(freq);
                    midiPitch = qRound(fNote*0.01);
                    midiCents = (midiPitch*100) - fNote;
                    qDebug() <<  freq << "Hz" << "max Index" << maxIndex << "max peak" << maxPeak << "chosen peak" << i << "Peaks" << peaks.size() << "Nearest pitch" << MIDIkey2Freq(midiPitch)  << "Nearest Note" << noteNames[midiPitch % 12] << "cents" << midiCents;
                }
            }
        }
    }
}
