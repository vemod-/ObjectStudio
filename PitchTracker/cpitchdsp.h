#ifndef CPITCHDSP_H
#define CPITCHDSP_H

//#include <math.h>
//#include <vector>
//#include <array>
//#include <QString>
#include "softsynthsdefines.h"

#define InverseLog2 (1.f / log10f(2.f))

/// <summary>
/// Pitch related DSP
/// </summary>
class CPitchDsp
{
private:

    static constexpr int kCourseOctaveSteps = 96;
    static constexpr int kScanHiSize = 31;
    static constexpr double kScanHiFreqStep = 1.005;
    static constexpr int kMinMidiNote = 12;  // 21 = A0 // C0
    static constexpr int kMaxMidiNote = 127; // 108 = C8 //?

    double m_minPitch;
    double m_maxPitch;
    //int m_minNote;
    //int m_maxNote;
    int m_blockLen14;	   // 1/4 block len
    int m_blockLen24;	   // 2/4 block len
    int m_blockLen34;	   // 3/4 block len
    int m_blockLen44;	   // 4/4 block len
    double m_sampleRate;
    float m_detectLevelThreshold;

    int m_numCourseSteps;
    std::vector<double> m_pCourseFreqOffsetVector;
    double* m_pCourseFreqOffset;
    std::vector<double> m_pCourseFreqVector;
    double* m_pCourseFreq;
    double m_scanHiOffset[kScanHiSize];
    double m_peakBuf[kScanHiSize];
    int m_prevPitchIdx;
    std::vector<double> m_detectCurveVector;
    double* m_detectCurve;
public:
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="fSampleRate"></param>
    /// <param name="minFreq"></param>
    /// <param name="maxFreq"></param>
    /// <param name="fFreqStep"></param>
    CPitchDsp(double sampleRate, double minPitch=50, double maxPitch=1600, float detectLevelThreshold=0.01f);

    /// <summary>
    /// Get the max detected pitch
    /// </summary>
    double MaxPitch()
    {
        return m_maxPitch;
    }

    /// <summary>
    /// Get the min detected pitch
    /// </summary>
    double MinPitch()
    {
        return m_minPitch;
    }
    /*
        /// <summary>
        /// Get the max note
        /// </summary>
        int MaxNote()
        {
            return m_maxNote;
        }

        /// <summary>
        /// Get the min note
        /// </summary>
        int MinNote()
        {
            return m_minNote;
        }
*/
    /// <summary>
    /// Detect the pitch
    /// </summary>
    double DetectPitch(float* samplesLo, float* samplesHi, int numSamples);
private:
    /// <summary>
    /// Low resolution pitch detection
    /// </summary>
    /// <param name="dataIdx"></param>
    /// <param name="begFreqIdx"></param>
    /// <param name="endFreqIdx"></param>
    /// <param name="blockLen"></param>
    /// <param name="stepSize"></param>
    /// <returns></returns>
    double DetectPitchLo(float* samplesLo, float* samplesHi);

    /// <summary>
    /// High resolution pitch detection
    /// </summary>
    /// <param name="dataIdx"></param>
    /// <param name="lowFreqIdx"></param>
    /// <returns></returns>
    double DetectPitchHi(float* samples, int lowFreqIdx);
public:
    /// <summary>
    /// Create a sine wave with the specified frequency, amplitude and starting angle.
    /// Returns the updated angle.
    /// </summary>
    /// <param name="buffer"></param>
    /// <param name="numSamples"></param>
    /// <param name="freq"></param>
    /// <param name="amplitude"></param>
    /// <param name="startAngle"></param>
    /// <returns></returns>
    static double CreateSineWave(float* buffer, uint numSamples, double sampleRate,
                                 double freq, double amplitude, double startAngle);
private:
    /// <summary>
    /// Returns true if the level is above the specified value
    /// </summary>
    /// <param name="buffer"></param>
    /// <param name="startIdx"></param>
    /// <param name="len"></param>
    /// <param name="level"></param>
    /// <returns></returns>
    inline bool LevelIsAbove(const float* buffer, const int len, const float level)
    {
        for (int idx = 0; idx < len; idx++) if (fabsf(buffer[idx]) >= level) return true;
        return false;
    }
    /// <summary>
    /// // 4-point, 3rd-order Hermite (x-form)
    /// </summary>
    inline double InterpolateHermite(const double fY0, const double fY1, const double fY2, const double fY3, const double frac)
    {
        const double c1 = 0.5 * (fY2 - fY0);
        const double c3 = 1.5 * (fY1 - fY2) + 0.5 * (fY3 - fY0);
        const double c2 = fY0 - fY1 + c1 - c3;
        return ((c3 * frac + c2) * frac + c1) * frac + fY1;
    }
    /// <summary>
    /// Linear interpolation
    /// nFrac is based on 1.0 = 256
    /// </summary>
    inline double InterpolateLinear(const double y0, const double y1, const double frac, const double invFrac)
    {
        return y0 * invFrac + y1 * frac;
    }
    /// <summary>
    /// Medium Low res SumAbsDiff
    /// </summary>
    double RatioAbsDiffLinear(const float* samples, const int freqIdx, const int blockLen, const int stepSize, const bool hiRes);
    /// <summary>
    /// Medium High res SumAbsDiff
    /// </summary>
    double SumAbsDiffHermite(const float* samples, const double fOffset, const int blockLen, const int stepSize);
public:
    /// <summary>
    /// Get the MIDI note and cents of the pitch
    /// </summary>
    /// <param name="pitch"></param>
    /// <param name="note"></param>
    /// <param name="cents"></param>
    /// <returns></returns>
    static bool PitchToMidiNote(double pitch, int& note, int& cents, double A440=440.0);

    /// <summary>
    /// Get the pitch from the MIDI note
    /// </summary>
    /// <param name="pitch"></param>
    /// <returns></returns>
    static double PitchToMidiNote(double pitch, double A440=440.0);

    /// <summary>
    /// Get the pitch from the MIDI note
    /// </summary>
    /// <param name="note"></param>
    /// <returns></returns>
    static double MidiNoteToPitch(int note, double A440=440.0);

    /// <summary>
    /// Format a midi note to text
    /// </summary>
    /// <param name="note"></param>
    /// <param name="sharps"></param>
    /// <param name="showOctave"></param>
    /// <returns></returns>
    static QString GetNoteName(int note, bool sharps, bool showOctave);

};

#endif // CPITCHDSP_H
