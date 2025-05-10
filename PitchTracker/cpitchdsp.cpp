#include "cpitchdsp.h"

CPitchDsp::CPitchDsp(double sampleRate, double minPitch, double maxPitch, float detectLevelThreshold)
{
    m_sampleRate = sampleRate;
    m_minPitch = minPitch;
    m_maxPitch = maxPitch;
    m_detectLevelThreshold = detectLevelThreshold;

    m_prevPitchIdx=0;

    m_blockLen44 = qRound(m_sampleRate / m_minPitch);
    m_blockLen34 = (m_blockLen44 * 3) / 4;
    m_blockLen24 = m_blockLen44 / 2;
    m_blockLen14 = m_blockLen44 / 4;

    m_numCourseSteps = qRound((log(m_maxPitch / m_minPitch) / log(2.0)) * kCourseOctaveSteps) + 3;

    m_pCourseFreqOffsetVector.assign(m_numCourseSteps + 10000,0);
    m_pCourseFreqOffset=m_pCourseFreqOffsetVector.data();

    m_pCourseFreqVector.assign(m_numCourseSteps + 10000,0);
    m_pCourseFreq=m_pCourseFreqVector.data();

    m_detectCurveVector.assign(m_numCourseSteps,0);
    m_detectCurve=m_detectCurveVector.data();

    const double freqStep = 1.0 / pow(2.0, 1.0 / kCourseOctaveSteps);
    double curFreq = m_maxPitch / freqStep;

    // frequency is stored from high to low
    for (int idx = 0; idx < m_numCourseSteps; idx++)
    {
        m_pCourseFreq[idx] = curFreq;
        m_pCourseFreqOffset[idx] = m_sampleRate / curFreq;
        curFreq *= freqStep;
    }

    for (int idx = 0; idx < kScanHiSize; idx++) m_scanHiOffset[idx] = pow(kScanHiFreqStep, (kScanHiSize / 2) - idx);
}

double CPitchDsp::DetectPitch(float *samplesLo, float *samplesHi, int numSamples)
{
    if (m_detectLevelThreshold > 0.01f)
    {
        if (!LevelIsAbove(samplesLo, numSamples, m_detectLevelThreshold)) return 0.0; // Level is too low
        if (!LevelIsAbove(samplesHi, numSamples, m_detectLevelThreshold)) return 0.0; // Level is too low
    }
    return DetectPitchLo(samplesLo, samplesHi);
}

double CPitchDsp::DetectPitchLo(float *samplesLo, float *samplesHi)
{
    memset(m_detectCurve,0,m_detectCurveVector.size()*sizeof(double));
    const int skipSize = 8;
    const int peakScanSize = 23;
    const int peakScanSizeHalf = peakScanSize / 2;
    const double peakThresh1 = 200.0;
    const double peakThresh2 = 600.0;
    bool bufferSwitched = false;
    // 258 is at 250 Hz, which is the switchover frequency for the two filters
    //const uint filterBreakIndex = 405;//258; approx 1050 Hz
    const int filterBreakIndex = 440;//258; approx 1500 Hz
    for (int idx = 0; idx < m_numCourseSteps; idx += skipSize)
    {
        const int blockLen = qMin<int>(m_blockLen44, int(m_pCourseFreqOffset[idx]) * 2);
        float* curSamples = nullptr;
        if (idx >= filterBreakIndex)
        {
            if (!bufferSwitched)
            {
                //m_detectCurve.Clear(258 - peakScanSizeHalf, 258 + peakScanSizeHalf);
                //memset(m_detectCurve+(filterBreakIndex - peakScanSizeHalf),0,(filterBreakIndex + peakScanSizeHalf)*sizeof(double));
                memset(m_detectCurve+(filterBreakIndex - peakScanSizeHalf),0,peakScanSize*sizeof(double));
                //for (int i=258 - peakScanSizeHalf;i<258 - peakScanSizeHalf+ 258 + peakScanSizeHalf;i++) m_detectCurve[i]=0;
                bufferSwitched = true;
            }
            curSamples = samplesLo;
        }
        else
        {
            curSamples = samplesHi;
        }
        const int stepSizeLoRes = qMax<int>(1, blockLen / 10);
        const int stepSizeHiRes = qBound<int>(1, idx * 5 / m_numCourseSteps, 5);
        const double fValue = RatioAbsDiffLinear(curSamples, idx, blockLen, stepSizeLoRes, false);
        if (fValue > peakThresh1)
        {
            int peakIdx = -1;// Do a closer search for the peak
            double peakVal = 0.0;
            double prevVal = 0.0;
            int dir = 4;		 // start going forward
            int curPos = idx;	 // start at center of the scan range
            int begSearch = qMax<int>(idx - peakScanSizeHalf, 0);
            int endSearch = qMin<int>(idx + peakScanSizeHalf, m_numCourseSteps - 1);
            while (curPos >= begSearch && curPos < endSearch)
            {
                const double curVal = RatioAbsDiffLinear(curSamples, curPos, blockLen, stepSizeHiRes, true);
                if (peakVal < curVal)
                {
                    peakVal = curVal;
                    peakIdx = curPos;
                }
                if (prevVal > curVal)
                {
                    dir = -dir >> 1;
                    if (dir == 0)
                    {
                        if (peakVal > peakThresh2 && peakIdx >= 6 && peakIdx <= m_numCourseSteps - 7)
                        {
                            const double fValL = RatioAbsDiffLinear(curSamples, peakIdx - 5, blockLen, stepSizeHiRes, true);
                            const double fValR = RatioAbsDiffLinear(curSamples, peakIdx + 5, blockLen, stepSizeHiRes, true);
                            const double fPointy = peakVal / (fValL + fValR) * 2.0;
                            const double minPointy = ((m_prevPitchIdx > 0) && (::abs(m_prevPitchIdx - peakIdx) < 10)) ? 1.2 : 1.5;
                            if (fPointy > minPointy)
                            {
                                const double pitchHi = DetectPitchHi(curSamples, peakIdx);
                                if (pitchHi > 1.0)
                                {
                                    m_prevPitchIdx = peakIdx;
                                    return pitchHi;
                                }
                            }
                        }
                        break;
                    }
                }
                prevVal = curVal;
                curPos += dir;
            }
        }
    }
    m_prevPitchIdx = 0;
    return 0.0;
}

double CPitchDsp::DetectPitchHi(float *samples, int lowFreqIdx)
{
    int peakIdx = -1;
    double prevVal = 0.0;
    int dir = 4;     // start going forward
    //m_peakBuf.Clear();
    memset(m_peakBuf,0,kScanHiSize*sizeof(double));
    const double offset = m_pCourseFreqOffset[lowFreqIdx];
    int curPos = kScanHiSize >> 1;	 // start at center of the scan range
    while (curPos >= 0 && curPos < kScanHiSize)
    {
        if (isZero(m_peakBuf[curPos])) m_peakBuf[curPos] = SumAbsDiffHermite(samples, offset * m_scanHiOffset[curPos], m_blockLen44, 1);
        if (peakIdx < 0 || m_peakBuf[peakIdx] < m_peakBuf[curPos]) peakIdx = curPos;
        if (prevVal > m_peakBuf[curPos])
        {
            dir = -dir >> 1;
            if (dir == 0)
            {
                double minVal = fmin(m_peakBuf[peakIdx - 1], m_peakBuf[peakIdx + 1]);// found the peak
                minVal -= minVal * (1.0 / 32.0);
                const double y1 = log10(m_peakBuf[peakIdx - 1] - minVal);
                const double y2 = log10(m_peakBuf[peakIdx] - minVal);
                const double y3 = log10(m_peakBuf[peakIdx + 1] - minVal);
                const double fIdx = double(peakIdx) + (y3 - y1) / (2.0 * (2.0 * y2 - y1 - y3));
                return pow(kScanHiFreqStep, fIdx - (kScanHiSize / 2)) * m_pCourseFreq[lowFreqIdx];
            }
        }
        prevVal = m_peakBuf[curPos];
        curPos += dir;
    }
    return 0.0;
}

double CPitchDsp::CreateSineWave(float *buffer, uint numSamples, double sampleRate, double freq, double amplitude, double startAngle)
{
    const double angleStep = freq / sampleRate * M_PI * 2.0;
    double curAngle = startAngle;

    for (uint idx = 0; idx < numSamples; idx++)
    {
        buffer[idx] = sin(curAngle) * amplitude;

        curAngle += angleStep;

        while (curAngle > M_PI) curAngle -= M_PI * 2.0;
    }

    return curAngle;
}

double CPitchDsp::RatioAbsDiffLinear(const float *samples, const int freqIdx, const int blockLen, const int stepSize, const bool hiRes)
{
    if (hiRes && m_detectCurve[freqIdx] > 0.0) return m_detectCurve[freqIdx];
    const auto offsetInt=int(m_pCourseFreqOffset[freqIdx]);
    const double offsetFrac = m_pCourseFreqOffset[freqIdx] - offsetInt;
    const double invFrac = 1.0 - offsetFrac;
    // Do a scan using linear interpolation and the specified step size
    double rect = 0.0;
    double absDiff = 0.01;   // prevent divide by zero
    for (int idx = 0; idx < blockLen; idx += stepSize)
    {
        const int offsetIdx = offsetInt + idx;
        const double sample = samples[idx];
        const double interp = InterpolateLinear(samples[offsetIdx], samples[offsetIdx + 1], offsetFrac, invFrac);
        absDiff += fabs(sample - interp);
        rect += fabs(sample) + fabs(interp);
    }
    const double finalVal = rect / absDiff * 100.0;
    if (hiRes) m_detectCurve[freqIdx] = finalVal;
    return finalVal;
}

double CPitchDsp::SumAbsDiffHermite(const float *samples, const double fOffset, const int blockLen, const int stepSize)
{
    auto offsetInt = int(fOffset);
    double offsetFrac = fOffset - offsetInt;
    // do a scan using linear interpolation and the specified step size
    double value = 0.001;   // prevent divide by zero
    int count = 0;
    for (int idx = 0; idx < blockLen; idx += stepSize, count++)
    {
        const int offsetIdx = offsetInt + idx;
        value += fabs(samples[idx] - InterpolateHermite(samples[offsetIdx - 1],
                      samples[offsetIdx],
                      samples[offsetIdx + 1],
                samples[offsetIdx + 2],
                offsetFrac));
    }
    return count / value;
}

bool CPitchDsp::PitchToMidiNote(double pitch, int &note, int &cents, double A440)
{
    if (pitch < 2.0)
    {
        note = 0;
        cents = 0;
        return false;
    }
    const long fNote = freq2Cent(pitch,A440);
    note = qRound(fNote*0.01);
    cents = (note*100) - fNote;
    return true;
}

double CPitchDsp::PitchToMidiNote(double pitch, double A440)
{
    return (pitch < 2.0) ? 0.0 : freq2Cent(pitch,A440)*0.01;
}

double CPitchDsp::MidiNoteToPitch(int note, double A440)
{
    return (note < 1) ? 0 : MIDIkey2Freq(note,A440);
}

QString CPitchDsp::GetNoteName(int note, bool sharps, bool showOctave)
{
    if (note < kMinMidiNote || note > kMaxMidiNote)
        return QString();

    note -= (kMinMidiNote+9);

    int octave = (note + 9) / 12;
    note = note % 12;
    QString noteText;

    switch (note)
    {
    case 0:
        noteText = "A";
        break;

    case 1:
        noteText = sharps ? "A#" : "Bb";
        break;

    case 2:
        noteText = "B";
        break;

    case 3:
        noteText = "C";
        break;

    case 4:
        noteText = sharps ? "C#" : "Db";
        break;

    case 5:
        noteText = "D";
        break;

    case 6:
        noteText = sharps ? "D#" : "Eb";
        break;

    case 7:
        noteText = "E";
        break;

    case 8:
        noteText = "F";
        break;

    case 9:
        noteText = sharps ? "F#" : "Gb";
        break;

    case 10:
        noteText = "G";
        break;

    case 11:
        noteText = sharps ? "G#" : "Ab";
        break;
    }

    if (showOctave)
        noteText += " " + QString::number(octave);

    return noteText;
}
