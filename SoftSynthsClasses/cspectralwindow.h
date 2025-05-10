#ifndef CSPECTRALWINDOW_H
#define CSPECTRALWINDOW_H

#include "softsynthsdefines.h"
//#include <vector>
#include <QMutexLocker>

/*
 These are the window definitions. These windows can be used for either
 FIR filter design or with an FFT for spectral analysis.
 For definitions, see this article:  http://en.wikipedia.org/wiki/Window_function

 This function has 6 inputs
 Data is the array, of length N, containing the data to to be windowed.
 This data is either a FIR filter sinc pulse, or the data to be analyzed by an fft.

 WindowType is an enum defined in the header file.
 e.g. wtKAISER, wtSINC, wtHANNING, wtHAMMING, wtBLACKMAN, ...

 Alpha sets the width of the flat top.
 Windows such as the Tukey and Trapezoid are defined to have a variably wide flat top.
 As can be seen by its definition, the Tukey is just a Hanning window with a flat top.
 Alpha can be used to give any of these windows a partial flat top, except the Flattop and Kaiser.
 Alpha = 0 gives the original window. (i.e. no flat top)
 To generate a Tukey window, use a Hanning with 0 < Alpha < 1
 To generate a Bartlett window (triangular), use a Trapezoid window with Alpha = 0.
 Alpha = 1 generates a rectangular window in all cases. (except the Flattop and Kaiser)


 Beta is used with the Kaiser, Sinc, and Sine windows only.
 These three windows are used primarily for FIR filter design. Then
 Beta controls the filter's transition bandwidth and the sidelobe levels.
 All other windows ignore Beta.

 UnityGain controls whether the gain of these windows is set to unity.
 Only the Flattop window has unity gain by design. The Hanning window, for example, has a gain
 of 1/2.  UnityGain = true  sets the gain to 1, which preserves the signal's energy level
 when these windows are used for spectral analysis.

 Don't use this with FIR filter design however. Since most of the enegy in an FIR sinc pulse
 is in the middle of the window, the window needs a peak amplitude of one, not unity gain.
 Setting UnityGain = true will simply cause the resulting FIR filter to have excess gain.

 If using these windows for FIR filters, start with the Kaiser, Sinc, or Sine windows and
 adjust Beta for the desired transition BW and sidelobe levels (set Alpha = 0).
 While the FlatTop is an excellent window for spectral analysis, don't use it for FIR filter design.
 It has a peak amplitude of ~ 4.7 which causes the resulting FIR filter to have about this much gain.
 It works poorly for FIR filters even if you adjust its peak amplitude.
 The Trapezoid also works poorly for FIR filter design.

 If using these windows with an fft for spectral analysis, start with the Hanning, Gauss, or Flattop.
 When choosing a window for spectral analysis, you must trade off between resolution and amplitude
 accuracy. The Hanning has the best resolution while the Flatop has the best amplitude accuracy.
 The Gauss is midway between these two for both accuracy and resolution. These three were
 the only windows available in the HP 89410A Vector Signal Analyzer. Which is to say, these three
 are the probably the best windows for general purpose signal analysis.
*/

class CSpectralWindow
{
public:
    // Must retain the order on the 1st line for legacy FIR code.
    enum TWindowType {wtFIRSTWINDOW, wtNONE, wtKAISER, wtSINC, wtHANNING,
                      wtHAMMING, wtBLACKMAN, wtFLATTOP, wtBLACKMAN_HARRIS,
                      wtBLACKMAN_NUTTALL, wtNUTTALL, wtKAISER_BESSEL, wtTRAPEZOID,
                      wtGAUSS, wtSINE, wtTEST };
    CSpectralWindow() {}
    CSpectralWindow(uint s) : size(s), m_WinCoeff(s+2) {}
    void setSize(uint s)
    {
        QMutexLocker locker(&mutex);
        size=s;
        m_WinCoeff.resize(s+2);
    }
    void SetWindow(uint s, TWindowType WindowType, float Alpha, float Beta, bool UnityGain)
    {
        QMutexLocker locker(&mutex);
        setSize(s);
        SetWindow(WindowType, Alpha, Beta, UnityGain);
    }
    void SetWindow(TWindowType WindowType, float Alpha, float Beta, bool UnityGain)
    {
        QMutexLocker locker(&mutex);
        if (WindowType == wtNONE)
        {
            WinCoeff = nullptr;
            return;
        }

        if (WindowType == wtKAISER ||  WindowType == wtFLATTOP ) Alpha = 0;

        if (Alpha < 0) Alpha = 0;
        if (Alpha > 1) Alpha = 1;

        if (Beta < 0) Beta = 0;
        if (Beta > 10) Beta = 10;

        uint TopWidth = Alpha * size;
        if (TopWidth % 2 != 0) TopWidth++;
        if (TopWidth > size) TopWidth = size;
        uint M = size - TopWidth;
        float dM = M + 1;

        const float pf = 2 * PI_F / dM;

        WinCoeff = m_WinCoeff.data();

        // Calculate the window for N/2 points, then fold the window over (at the bottom).
        // TopWidth points will be set to 1.
        if (WindowType == wtKAISER)
        {
            for (uint j=0; j<M; j++)
            {
                const float Arg = Beta * sqrtf(1.f - powf( ((2.f*j+2.f) - dM) / dM, 2.f) );
                WinCoeff[j] = Bessel(Arg) / Bessel(Beta);
            }
        }

        else if (WindowType == wtSINC)  // Lanczos
        {
            for (uint j=0; j<M; j++) WinCoeff[j] = Sinc((2.f*j+1.f-M)/dM * PI_F );
            for (uint j=0; j<M; j++) WinCoeff[j] = powf(WinCoeff[j], Beta);
        }

        else if (WindowType == wtSINE)  // Hanning if Beta = 2
        {
            for (uint j=0; j<M/2; j++) WinCoeff[j] = sinf((j+1.f) * PI_F / dM);
            for (uint j=0; j<M/2; j++) WinCoeff[j] = powf(WinCoeff[j], Beta);
        }

        else if (WindowType == wtHANNING)
        {
            for (uint j=0; j<M/2; j++) WinCoeff[j] = 0.5f - 0.5f * cosf((j+1) * pf);
        }

        else if (WindowType == wtHAMMING)
        {
            for (uint j=0; j<M/2; j++)
                WinCoeff[j] = 0.54f - 0.46f * cosf((j+1) * pf);
        }

        else if (WindowType == wtBLACKMAN)
        {
            for (uint j=0; j<M/2; j++)
            {
                const float f = (j+1) * pf;
                WinCoeff[j] = 0.42f
                        - 0.50f * cosf(f)
                        + 0.08f * cosf(f * 2);
            }
        }

        // Defined at: http://www.bth.se/fou/forskinfo.nsf/0/130c0940c5e7ffcdc1256f7f0065ac60/$file/ICOTA_2004_ttr_icl_mdh.pdf
        else if (WindowType == wtFLATTOP)
        {
            for (uint j=0; j<=M/2; j++)
            {
                const float f = (j+1) * pf;
                WinCoeff[j] = 1.0f
                        - 1.93293488969227f * cosf(f)
                        + 1.28349769674027f * cosf(f * 2)
                        - 0.38130801681619f * cosf(f * 3)
                        + 0.02929730258511f * cosf(f * 4);
            }
        }
        else if (WindowType == wtBLACKMAN_HARRIS)
        {
            for (uint j=0; j<M/2; j++)
            {
                const float f = (j+1) * pf;
                WinCoeff[j] = 0.35875f
                        - 0.48829f * cosf(f)
                        + 0.14128f * cosf(f * 2)
                        - 0.01168f * cosf(f * 3);
            }
        }
        else if (WindowType == wtBLACKMAN_NUTTALL)
        {
            for (uint j=0; j<M/2; j++)
            {
                const float f = (j+1) * pf;
                WinCoeff[j] = 0.3535819f
                        - 0.4891775f * cosf(f)
                        + 0.1365995f * cosf(f * 2)
                        - 0.0106411f * cosf(f * 3);
            }
        }
        else if (WindowType == wtNUTTALL)
        {
            for (uint j=0; j<M/2; j++)
            {
                const float f = (j+1) * pf;
                WinCoeff[j] = 0.355768f
                        - 0.487396f * cosf(f)
                        + 0.144232f * cosf(f * 2)
                        - 0.012604f * cosf(f * 3);
            }
        }
        else if (WindowType == wtKAISER_BESSEL)
        {
            for (uint j=0; j<=M/2; j++)
            {
                const float f = (j+1) * pf;
                WinCoeff[j] = 0.402f
                        - 0.498f * cosf(f)
                        + 0.098f * cosf(2 * f)
                        + 0.001f * cosf(3 * f);
            }
        }
        else if (WindowType == wtTRAPEZOID) // Rectangle for Alpha = 1  Triangle for Alpha = 0
        {
            uint K = M/2;
            if (M%2) K++;
            for (uint j=0; j<K; j++) WinCoeff[j] = (j+1) / float(K);
        }
        // This definition is from http://en.wikipedia.org/wiki/Window_function (Gauss Generalized normal window)
        // We set their p = 2, and use Alpha in the numerator, instead of Sigma in the denominator, as most others do.
        // Alpha = 2.718 puts the Gauss window response midway between the Hanning and the Flattop (basically what we want).
        // It also gives the same BW as the Gauss window used in the HP 89410A Vector Signal Analyzer.
        else if (WindowType == wtGAUSS)
        {
            for (uint j=0; j<M/2; j++)
            {
                WinCoeff[j] = ((j+1) - dM/2.0f) / (dM/2.0f) * 2.7183f;
                WinCoeff[j] *= WinCoeff[j];
                WinCoeff[j] = expf(-WinCoeff[j]);
            }
        }
        else // Error.
        {
            WinCoeff = nullptr;
            return;
        }
        // Fold the coefficients over.
        for (uint j=0; j<M/2; j++) WinCoeff[size-j-1] = WinCoeff[j];
        // This is the flat top if Alpha > 0. Cannot be applied to a Kaiser or Flat Top.
        if (WindowType != wtKAISER &&  WindowType != wtFLATTOP)
        {
            for (uint j=M/2; j<size-M/2; j++) WinCoeff[j] = 1.0f;
        }
        // UnityGain = true will set the gain of these windows to 1. Don't use this with FIR filter design.
        if (UnityGain)
        {
            float Sum = 0.0;
            for (uint j=0; j<size; j++) Sum += WinCoeff[j];
            Sum /= size;
            if (Sum != 0.0f) for (uint j=0; j<size; j++) WinCoeff[j] /= Sum;
        }
    }
    void ApplyWindow(float *Data)
    {
        if (WinCoeff) for (uint j=0; j<size; j++) Data[j] *= WinCoeff[j];
    }
    float* WinCoeff = nullptr;
protected:
    // This gets used with the Sinc window.
    static float Sinc(float x)
    {
        if (x > -1.0E-5f && x < 1.0E-5f) return 1;
        return sinf(x)/x;
    }
    uint size = 0;
private:
    QRecursiveMutex mutex;
    // This gets used with the Kaiser window.
    float Bessel(float x)
    {
        float Sum=0.0, XtoIpower;
        int Factorial;
        for (uint i=1; i<10; i++)
        {
            XtoIpower = powf(x/2, i);
            Factorial = 1;
            for (uint j=1; j<=i; j++) Factorial *= j;
            Sum += powf(XtoIpower / Factorial, 2);
        }
        return 1.f + Sum;
    }
    std::vector<float> m_WinCoeff;
};

/*
July 15, 2015
Iowa Hills Software LLC
http://www.iowahills.com

If you find a problem with this code, please leave us a note on:
http://www.iowahills.com/feedbackcomments.html

Source: ~Projects\Common\BasicFIRFilterCode.cpp

This generic FIR filter code is described in most textbooks.
e.g. Discrete Time Signal Processing, Oppenheim and Shafer

A nice paper on this topic is:
http://dea.brunel.ac.uk/cmsp/Home_Saeed_Vaseghi/Chapter05-DigitalFilters.pdf

This code first generates either a low pass, high pass, band pass, or notch
impulse response for a rectangular window. It then applies a window to this
impulse response.

There are several windows available, including the Kaiser, Sinc, Hanning,
Blackman, and Hamming. Of these, the Kaiser and Sinc are probably the most useful
for FIR filters because their sidelobe levels can be controlled with the Beta parameter.

This is a typical function call:
BasicFIR(FirCoeff, NumTaps, PassType, OmegaC, BW, wtKAISER, Beta);
BasicFIR(FirCoeff, 33, LPF, 0.2, 0.0, wtKAISER, 3.2);
33 tap, low pass, corner frequency at 0.2, BW=0 (ignored in the low pass code),
Kaiser window, Kaiser Beta = 3.2

These variables should be defined similar to this:
double FirCoeff[MAXNUMTAPS];
int NumTaps;                        NumTaps can be even or odd, but must be less than the FirCoeff array size.
TPassTypeName PassType;             PassType is an enum defined in the header file. LPF, HPF, BPF, or NOTCH
double OmegaC  0.0 < OmegaC < 1.0   The filters corner freq, or center freq if BPF or NOTCH
double BW      0.0 < BW < 1.0       The filters band width if BPF or NOTCH
TWindowType WindowType;             WindowType is an enum defined in the header to be one of these.
                                    wtNONE, wtKAISER, wtSINC, wtHANNING, .... and others.
double Beta;  0 <= Beta <= 10.0     Beta is used with the Kaiser, Sinc, and Sine windows only.
                                    It controls the transition BW and sidelobe level of the filters.


If you want to use it, Kaiser originally defined Beta as follows.
He derived its value based on the desired sidelobe level, dBAtten.
double dBAtten, Beta, Beta1=0.0, Beta2=0.0;
if(dBAtten < 21.0)dBAtten = 21.0;
if(dBAtten > 50.0)Beta1 = 0.1102 * (dBAtten - 8.7);
if(dBAtten >= 21.0 && dBAtten <= 50.0) Beta2 = 0.5842 * pow(dBAtten - 21.0, 0.4) + 0.07886 * (dBAtten - 21.0);
Beta = Beta1 + Beta2;

*/

class CBasicFIR : protected CSpectralWindow
{
public:
    enum TPassTypeName {LPF, HPF, BPF, NOTCH};
    CBasicFIR() {}
    CBasicFIR(uint s) : CSpectralWindow(s), m_FirCoeff(s+2) {}
    void setSize(uint s)
    {
        CSpectralWindow::setSize(s);
        m_FirCoeff.resize(s+2);
    }
    // This first calculates the impulse response for a rectangular window.
    // It then applies the windowing function of choice to the impulse response.
    void BasicFIR(TPassTypeName PassType, double OmegaC, double BW, TWindowType WindowType, double WinBeta)
    {
        //double OmegaLow, OmegaHigh;
        const double OmegaLow  = OmegaC - BW/2.0;
        const double OmegaHigh = OmegaC + BW/2.0;

        const double sf = double(size-1)/2.0;

        FirCoeff = m_FirCoeff.data();

        switch(PassType)
        {
        case LPF:
            for (uint j = 0; j < size; j++)
            {
                const double Arg = double(j) - sf;
                FirCoeff[j] = OmegaC * Sinc(OmegaC * Arg * M_PI);
            }
            break;

        case HPF:
            if(size % 2 == 1) // Odd tap counts
            {
                for (uint j = 0; j < size; j++)
                {
                    const double Arg = double(j) - sf;
                    FirCoeff[j] = Sinc(Arg * M_PI) - OmegaC * Sinc(OmegaC * Arg * M_PI);
                }
            }
            else  // Even tap counts
            {
                for (uint j = 0; j < size; j++)
                {
                    const double Arg = double(j) - sf;
                    if(Arg == 0.0)FirCoeff[j] = 0.0;
                    else FirCoeff[j] = cos(OmegaC * Arg * M_PI) / M_PI / Arg  + cos(Arg * M_PI);
                }
            }
            break;
        case BPF:
            //const double OmegaLow  = OmegaC - BW/2.0;
            //const double OmegaHigh = OmegaC + BW/2.0;
            for (uint j = 0; j < size; j++)
            {
                const double Arg = double(j) - sf;
                if(Arg == 0.0)FirCoeff[j] = 0.0;
                else FirCoeff[j] =  ( cos(OmegaLow * Arg * M_PI) - cos(OmegaHigh * Arg * M_PI) ) / M_PI / Arg ;
            }
            break;
        case NOTCH:  // If NumTaps is even for Notch filters, the response at Pi is attenuated.
            //const double OmegaLow  = OmegaC - BW/2.0;
            //const double OmegaHigh = OmegaC + BW/2.0;
            for (uint j = 0; j < size; j++)
            {
                const double Arg = double(j) - sf;
                FirCoeff[j] =  Sinc(Arg * M_PI) - OmegaHigh * Sinc(OmegaHigh * Arg * M_PI) - OmegaLow * Sinc(OmegaLow * Arg * M_PI);
            }
            break;
        }
        // WindowData can be used to window data before an FFT. When used for FIR filters we set
        // Alpha = 0.0 to prevent a flat top on the window and
        // set UnityGain = false to prevent the window gain from getting set to unity.
        SetWindow(WindowType, 0.0, WinBeta, false);
        ApplyWindow(FirCoeff);
    }
    float* FirCoeff = nullptr;
private:
    std::vector<float> m_FirCoeff;
};

#endif // CSPECTRALWINDOW_H
