#ifndef C3BANDFILTER_H
#define C3BANDFILTER_H

#include "softsynthsdefines.h"

#define EPSILON_FLOAT 1.0e-5f

class C3BandFilter
{
public:
    void inline init(float lowfreq, float highfreq, float mixfreq)
    {
        // Clear state
        lf=0;       // Frequency
        f1p0=0;     // Poles ...
        f1p1=0;
        f1p2=0;
        f1p3=0;
        // Filter #2 (High band)
        hf=0;       // Frequency
        f2p0=0;     // Poles ...
        f2p1=0;
        f2p2=0;
        f2p3=0;
        // Sample history buffer
        sdm1=0;     // Sample data minus 1
        sdm2=0;     //                   2
        sdm3=0;     //                   3

        // Set Low/Mid/High gains to unity
        lg = 1.0;
        mg = 1.0;
        hg = 1.0;
        // Calculate filter cutoff frequencies
        lf = 2.f * sinf(PI_F * (lowfreq / mixfreq));
        hf = 2.f * sinf(PI_F * (highfreq / mixfreq));
    }
    float inline apply(const float input)
    {
        float  l,m,h;      // Low / Mid / High - Sample Values
        // Filter #1 (lowpass)
        f1p0  += (lf * (input   - f1p0)) + EPSILON_FLOAT;
        f1p1  += (lf * (f1p0 - f1p1));
        f1p2  += (lf * (f1p1 - f1p2));
        f1p3  += (lf * (f1p2 - f1p3));
        l          = f1p3;
        // Filter #2 (highpass)
        f2p0  += (hf * (input   - f2p0)) + EPSILON_FLOAT;
        f2p1  += (hf * (f2p0 - f2p1));
        f2p2  += (hf * (f2p1 - f2p2));
        f2p3  += (hf * (f2p2 - f2p3));
        h          = sdm3 - f2p3;
        // Calculate midrange (signal - (low + high))
        m          = sdm3 - (h + l);
        // Scale, Combine and store
        l         *= lg;
        m         *= mg;
        h         *= hg;
        // Shuffle history buffer
        sdm3   = sdm2;
        sdm2   = sdm1;
        sdm1   = input;
        // Return result
        return l + m + h;
    }
    // Gain Controls

    float  lg;       // low  gain
    float  mg;       // mid  gain
    float  hg;       // high gain
private:
    // Filter #1 (Low band)
    float  lf;       // Frequency
    float  f1p0;     // Poles ...
    float  f1p1;
    float  f1p2;
    float  f1p3;

    // Filter #2 (High band)

    float  hf;       // Frequency
    float  f2p0;     // Poles ...
    float  f2p1;
    float  f2p2;
    float  f2p3;

    // Sample history buffer

    float  sdm1;     // Sample data minus 1
    float  sdm2;     //                   2
    float  sdm3;     //                   3
};

#endif // C3BANDFILTER_H
