#include "cpitchdetect.h"
#include <QDebug>

CPitchDetect::CPitchDetect(uint sampleRate) : m_dsp(sampleRate),
    m_FIFOLo(sampleRate*2), m_FIFOHi(sampleRate*2), m_sampleRate(sampleRate),
    m_iirFilterLoHi(sampleRate), m_iirFilterHiLo(sampleRate), m_iirFilterHiHi(sampleRate)
{
    m_pitchBufSize=0;
    m_samplesPerPitchBlock=0;
    //m_curPitchIndex=0;
    m_curPitchSamplePos=0;

    m_detectOverlapSamples=0;
    m_maxOverlapDiff=0;

    m_recordPitchRecords=false;
    m_pitchRecordHistorySize=0;
    m_Tune=440;
    Setup();
}

void CPitchDetect::Reset()
{
    //m_curPitchIndex = 0;
    m_curPitchSamplePos = 0;
    m_pitchRecords.clear();
    m_iirFilterLoHi.Reset();
    m_iirFilterHiLo.Reset();
    m_iirFilterHiHi.Reset();
    m_FIFOLo.reset();
    m_FIFOHi.reset();

    memset(m_pitchBufLo.data(),0,m_pitchBufLo.size()*sizeof(float));
    memset(m_pitchBufHi.data(),0,m_pitchBufHi.size()*sizeof(float));
    m_PrevPitch = 0.0;
}

void CPitchDetect::ProcessBuffer(float *inBuffer, int sampleCount)
{
    if (!inBuffer) return; //throw new ArgumentNullException("inBuffer", "Input buffer cannot be null");
    float* pitchBufLo=m_pitchBufLo.data();
    float* pitchBufHi=m_pitchBufHi.data();
    if (!pitchBufHi) return;
    if (!pitchBufLo) return;
    int samplesProcessed = 0;
    while (samplesProcessed < sampleCount)
    {
        int frameCount = qMin<int>(sampleCount - samplesProcessed, m_pitchBufSize + m_detectOverlapSamples);
        m_iirFilterHiLo.FilterBuffer(inBuffer, samplesProcessed, pitchBufHi, 0, frameCount);
        //m_iirFilterHiHi.FilterBuffer(pitchBufHi, 0, pitchBufHi, 0, frameCount);
        m_iirFilterLoHi.FilterBuffer(inBuffer, samplesProcessed, pitchBufLo, 0, frameCount);
        m_FIFOLo.write(pitchBufLo, frameCount);
        m_FIFOHi.write(pitchBufHi, frameCount);

        while (m_FIFOLo.isAvail(m_pitchBufSize + m_detectOverlapSamples))
        { // Loop while there is enough samples in the circular buffer
            pitchBufLo=m_FIFOLo.read(m_pitchBufSize + m_detectOverlapSamples);
            pitchBufHi=m_FIFOHi.read(m_pitchBufSize + m_detectOverlapSamples);
            double detectedPitch = m_PrevPitch;
            const double pitch1 = m_dsp.DetectPitch(pitchBufLo, pitchBufHi, m_pitchBufSize);
            if (pitch1 > 0.0)
            {
                const double pitch2 = (m_detectOverlapSamples > 0) ? m_dsp.DetectPitch(pitchBufLo  + m_detectOverlapSamples, pitchBufHi  + m_detectOverlapSamples, m_pitchBufSize) : m_PrevPitch;
                if (pitch2 > 0.0)
                {
                    const double fDiff = fmax(pitch1, pitch2) / fmin(pitch1, pitch2) - 1.0;
                    qDebug() << fDiff << m_maxOverlapDiff;
                    if (fDiff < m_maxOverlapDiff) {
                        detectedPitch = (pitch1 + pitch2) * 0.5;
                    }
                    else {
                        detectedPitch = fmax(pitch1,pitch2);
                    }
                }
                qDebug() << pitch1 << pitch2 << detectedPitch << m_PrevPitch;
            }
            AddPitchRecord(detectedPitch); // Log the pitch record
            m_PrevPitch = pitch1;
            m_curPitchSamplePos += m_samplesPerPitchBlock;
        }
        samplesProcessed += frameCount;
    }
}

void CPitchDetect::Setup()
{
    if (m_sampleRate < 1) return;

    m_dsp = CPitchDsp(m_sampleRate, kMinFreq, m_MaxFreq, m_detectLevelThreshold);

    m_iirFilterLoHi.setProto(IIRProtoType::Butterworth);
    m_iirFilterLoHi.setType(IIRFilterType::LP);
    m_iirFilterLoHi.setOrder(5);
    m_iirFilterLoHi.setFreqHigh(1500.0);//280

    m_iirFilterHiLo.setProto(IIRProtoType::Butterworth);
    m_iirFilterHiLo.setType(IIRFilterType::HP);
    m_iirFilterHiLo.setOrder(5);
    m_iirFilterHiLo.setFreqLow(200.0);//45

    m_iirFilterHiHi.setProto(IIRProtoType::Butterworth);
    m_iirFilterHiHi.setType(IIRFilterType::LP);
    m_iirFilterHiHi.setOrder(5);
    m_iirFilterHiHi.setFreqHigh(m_MaxFreq/2); // 1500

    //m_detectOverlapSamples = 0; //int(kDetectOverlapSec * m_sampleRate);
    double OverlapSec = (m_detectOverlapSamples > 0) ? m_detectOverlapSamples / double(m_sampleRate) : 1.0 / m_pitchRecordsPerSecond;
    m_maxOverlapDiff = kMaxOctaveSecRate * OverlapSec;

    m_pitchBufSize = int(((1.0 / kMinFreq) * 2.0 + ((kAvgCount - 1) * kAvgOffset)) * double(m_sampleRate)) + 16;
    m_pitchBufLo.resize(m_pitchBufSize + m_detectOverlapSamples);
    m_pitchBufHi.resize(m_pitchBufSize + m_detectOverlapSamples);
    m_samplesPerPitchBlock = qRound(double(m_pitchBufSize) / m_pitchRecordsPerSecond);
    m_FIFOLo.setStepSize(m_samplesPerPitchBlock);
    //qDebug() << m_pitchBufSize << m_samplesPerPitchBlock;
    m_FIFOLo.clear(m_pitchBufSize - m_samplesPerPitchBlock);
    m_FIFOHi.setStepSize(m_samplesPerPitchBlock);
    m_FIFOHi.clear(m_pitchBufSize - m_samplesPerPitchBlock);
    memset(&m_curPitchRecord,0,sizeof(PitchRecord));
    Reset();
    qDebug() << "PitchDetect" << m_pitchBufSize << m_samplesPerPitchBlock << m_pitchRecordsPerSecond << OverlapSec << m_detectOverlapSamples << m_maxOverlapDiff;
}

void CPitchDetect::AddPitchRecord(double pitch)
{
    int midiNote = 0;
    int midiCents = 0;
    CPitchDsp::PitchToMidiNote(pitch, midiNote, midiCents, m_Tune);
    PitchRecord r{pitch,midiNote,midiCents};
    m_curPitchRecord=r;
    if (m_recordPitchRecords)
    {
        while (m_pitchRecords.size() >= m_pitchRecordHistorySize) m_pitchRecords.erase(m_pitchRecords.begin());
        m_pitchRecords.push_back(r);
    }
}
