#ifndef CPITCHDETECT_H
#define CPITCHDETECT_H

//#include <vector>
#include "ciirfilters.h"
#include "cpitchdsp.h"
//#include "softsynthsdefines.h"
#include "cfastcircularbuffer.h"

/// <summary>
/// Tracks pitch
/// </summary>
class CPitchDetect
{
public:
    /// <summary>
    /// Stores one record
    /// </summary>
    struct PitchRecord
    {
        /// <summary>
        /// The detected pitch
        /// </summary>
        double Pitch;
/*
        /// <summary>
        /// The index of the pitch record since the last Reset call
        /// </summary>
        int RecordIndex;
*/
        /// <summary>
        /// The detected MIDI note, or 0 for no pitch
        /// </summary>
        int MidiKey;

        /// <summary>
        /// The offset from the detected MIDI note in cents, from -50 to +50.
        /// </summary>
        int MidiCents;
    };
private:
    static constexpr int kOctaveSteps = 96;
    static constexpr int kStepOverlap = 4;
    static constexpr double kMinFreq = 10;//50.0f;               // A1, Midi note 33, 55.0Hz
    double m_MaxFreq = 20000;//1600.0f;             // A#6. Midi note 92
    static constexpr int kStartCircular = 40;		        // how far into the sample buffer do we start checking (allow for filter settling)
    static constexpr double kDetectOverlapSec = 0.005;
    static constexpr double kMaxOctaveSecRate = 10.0;

    static constexpr double kAvgOffset = 0.005;	        // time offset between pitch averaging values
    static constexpr int kAvgCount = 1;			        // number of average pitch samples to take
    static constexpr double kCircularBufSaveTime = 1.0;    // Amount of samples to store in the history buffer

    CPitchDsp m_dsp;

    CFastCircularBuffer m_FIFOLo;
    CFastCircularBuffer m_FIFOHi;
    double m_Tune;
    uint m_sampleRate;
    float m_detectLevelThreshold = 0.01f;       // -40dB
    int m_pitchRecordsPerSecond = 10;           // default is 50, or one record every 20ms

    std::vector<float> m_pitchBufLo;
    std::vector<float> m_pitchBufHi;
    int m_pitchBufSize;
    int m_samplesPerPitchBlock;
    //int m_curPitchIndex;
    long64 m_curPitchSamplePos;

    int m_detectOverlapSamples;
    double m_maxOverlapDiff;

    bool m_recordPitchRecords;
    uint m_pitchRecordHistorySize;
    std::vector<PitchRecord> m_pitchRecords;
    PitchRecord m_curPitchRecord;

    double m_PrevPitch;

    CIIRFilters m_iirFilterLoHi;
    CIIRFilters m_iirFilterHiLo;
    CIIRFilters m_iirFilterHiHi;
public:
    //delegate void PitchDetectedHandler(PitchTracker sender, PitchRecord pitchRecord);
    //event PitchDetectedHandler PitchDetected;

    /// <summary>
    /// Constructor
    /// </summary>
    CPitchDetect(uint sampleRate);

    /// <summary>
    /// Set the sample rate
    /// </summary>
    uint SampleRate()
    {
        return m_sampleRate;
    }
    void setSampleRate(uint value)
    {
        QMutexLocker locker(&mutex);
        if (m_sampleRate==value) return;
        m_sampleRate = value;
        Setup();
    }

    /// <summary>
    /// Set the detect level threshold, The value must be between 0.0001f and 1.0f (-80 dB to 0 dB)
    /// </summary>
    float DetectLevelThreshold()
    {
        return m_detectLevelThreshold;
    }
    void setDetectLevelThreshold(float value)
    {
        QMutexLocker locker(&mutex);
        float newValue = std::clamp<float>(value,0.0001f,1.0f); //std::max(0.0001f, std::min(1.0f, value));

        if (closeEnough(m_detectLevelThreshold, newValue)) return;

        m_detectLevelThreshold = newValue;
        Setup();
    }

    double tune()
    {
        return m_Tune;
    }
    void setTune(double value)
    {
        m_Tune=value;
    }

    /// <summary>
    /// Return the samples per pitch block
    /// </summary>
    int SamplesPerPitchBlock()
    {
        return m_samplesPerPitchBlock;
    }

    /// <summary>
    /// Get or set the number of pitch records per second (default is 50, or one record every 20ms)
    /// </summary>
    int PitchRecordsPerSecond()
    {
        return m_pitchRecordsPerSecond;
    }
    void setPitchRecordsPerSecond(int value)
    {
        QMutexLocker locker(&mutex);
        value = qBound<int>(1,value,100);
        if (value != m_pitchRecordsPerSecond)
        {
            m_pitchRecordsPerSecond = value;
            Setup();
        }
    }

    /// <summary>
    /// Get or set whether pitch records should be recorded into a history buffer
    /// </summary>
    bool RecordPitchRecords()
    {
        return m_recordPitchRecords;
    }
    void setRecordPitchRecords(bool value)
    {
        if (m_recordPitchRecords == value)
            return;

        m_recordPitchRecords = value;

        if (!m_recordPitchRecords)
            m_pitchRecords.clear();
    }

    /// <summary>
    /// Get or set the max number of pitch records to keep. A value of 0 means no limit.
    /// Don't leave this at 0 when RecordPitchRecords is true and this is used in a realtime
    /// application since the buffer will grow indefinately!
    /// </summary>
    uint PitchRecordHistorySize()
    {
        return m_pitchRecordHistorySize;
    }
    void setPitchRecordHistorySize(uint value)
        {
            m_pitchRecordHistorySize = value;
            if (value < m_pitchRecords.size()) m_pitchRecords.resize(m_pitchRecordHistorySize);
        }

    /// <summary>
    /// Get the current pitch records
    /// </summary>
    std::vector<PitchRecord> PitchRecords()
    {
        return m_pitchRecords;
    }

    /// <summary>
    /// Get the latest pitch record
    /// </summary>
    PitchRecord CurrentPitchRecord()
    {
        return m_curPitchRecord;
    }

    /// <summary>
    /// Get the current pitch position
    /// </summary>
    long64 CurrentPitchSamplePosition()
    {
        return m_curPitchSamplePos;
    }

    /// <summary>
    /// Get the minimum frequency that can be detected
    /// </summary>
    static double MinDetectFrequency()
    {
        return kMinFreq;
    }

    /// <summary>
    /// Get the maximum frequency that can be detected
    /// </summary>
    double MaxDetectFrequency()
    {
        return m_MaxFreq;
    }
    void setMaxDetectFrequency(double v)
    {
        QMutexLocker locker(&mutex);
        if (!closeEnough(v,m_MaxFreq))
        {
            m_MaxFreq=v;
            Setup();
        }
    }

    int Overlap()
    {
        return m_detectOverlapSamples;
    }
    void setOverlap(int v)
    {
        QMutexLocker locker(&mutex);
        if (v != m_detectOverlapSamples)
        {
            m_detectOverlapSamples = v;
            Setup();
        }
    }

    /// <summary>
    /// Get the frequency step
    /// </summary>
    static double FrequencyStep()
    {
        return pow(2.0, 1.0 / kOctaveSteps);
    }

    /// <summary>
    /// Get the number of samples that the detected pitch is offset from the input buffer.
    /// This is just an estimate to sync up the samples and detected pitch
    /// </summary>
    int DetectSampleOffset()
    {
        return (m_pitchBufSize + m_detectOverlapSamples) / 2;
    }

    /// <summary>
    /// Reset the pitch tracker. Call this when the sample position is
    /// not consecutive from the previous position
    /// </summary>
    void Reset();

    /// <summary>
    /// Process the passed in buffer of data. During this call, the PitchDetected event will
    /// be fired zero or more times, depending how many pitch records will fit in the new
    /// and previously cached buffer.
    ///
    /// This means that there is no size restriction on the buffer that is passed into ProcessBuffer.
    /// For instance, ProcessBuffer can be called with one very large buffer that contains all of the
    /// audio to be processed (many PitchDetected events will be fired), or just a small buffer at
    /// a time which is more typical for realtime applications. In the latter case, the PitchDetected
    /// event might not be fired at all since additional calls must first be made to accumulate enough
    /// data do another pitch detect operation.
    /// </summary>
    /// <param name="inBuffer">Input buffer. Samples must be in the range -1.0 to 1.0</param>
    /// <param name="sampleCount">Number of samples to process. Zero means all samples in the buffer</param>
    void ProcessBuffer(float* inBuffer, int sampleCount);
private:
    QRecursiveMutex mutex;
    /// <summary>
    /// Setup
    /// </summary>
    void Setup();

    /// <summary>
    /// The pitch was detected - add the record
    /// </summary>
    /// <param name="pitch"></param>
    void AddPitchRecord(double pitch);
};

#endif // CPITCHDETECT_H
