#ifndef CIIRFILTERS_H
#define CIIRFILTERS_H

//#include <vector>
//#include <array>
#include "softsynthsdefines.h"
#include "cfft.h"

/// Infinite impulse response filter (old style analog filters)
/// The type of filter
enum IIRFilterType
{
    NoFilterType = 0,
    LP,
    HP,
    BP
};
/// The filter prototype
enum IIRProtoType
{
    NoProtoType = 0,
    Butterworth,
    Chebyshev
};

class CIIRFilters
{
public:
    CIIRFilters(uint sampleRate) { setSampleRate(sampleRate); }
    /// Returns true if all the filter parameters are valid
    bool FilterValid() {
        if (m_order < 1 || m_order > 16 || m_protoType == NoProtoType || m_filterType == NoFilterType || m_fN <= 0.0) return false;
        switch (m_filterType) {
        case LP:
            if (m_fp2 <= 0.0) return false;
            break;
        case BP:
            if (m_fp1 <= 0.0 || m_fp2 <= 0.0 || m_fp1 >= m_fp2) return false;
            break;
        case HP:
            if (m_fp1 <= 0.0) return false;
            break;
        case NoFilterType:
            break;
        }
        return true;
    }
    /// Set the filter prototype
    IIRProtoType Proto() { return m_protoType; }
    void setProto(IIRProtoType value) {
        if (value == m_protoType) return;
        m_protoType = value;
        Design();
    }
    /// Set the filter type
    IIRFilterType Type() { return m_filterType; }
    void setType(IIRFilterType value) {
        if (value == m_filterType) return;
        m_filterType = value;
        Design();
    }
    uint Order() { return m_order; }
    void setOrder(uint value) {
        if (m_order == value) return;
        m_order = qBound<uint>(1,value,16);
        if (m_filterType == BP && Odd(m_order)) m_order++;
        Design();
    }
    uint SampleRate() { return m_sampleRate; }
    void setSampleRate(uint value) {
        if (value == m_sampleRate) return;
        m_sampleRate = value;
        m_fN = 0.5 * m_sampleRate;
        Design();
    }
    double FreqLow() { return m_fp1; }
    void setFreqLow(double value) {
        if (closeEnough(value,m_fp1)) return;
        m_fp1 = value;
        Design();
    }
    double FreqHigh() { return m_fp2; }
    void setFreqHigh(double value) {
        if (closeEnough(value,m_fp2)) return;
        m_fp2 = value;
        Design();
    }
    double Ripple() { return m_ripple; }
    void setRipple(double value) {
        if (closeEnough(value,m_ripple)) return;
        m_ripple = value;
        Design();
    }
private:
    uint m_order = 0;
    IIRProtoType m_protoType = NoProtoType;
    IIRFilterType m_filterType = NoFilterType;
    double m_fp1 = 0;
    double m_fp2 = 0;
    double m_fN = 0;
    double m_ripple = 0.5;
    uint m_sampleRate;
    std::vector<cplx<double>> m_W;
    std::vector<double> m_z;
    std::vector<cplx<double>> m_CoeffVector;
    cplx<double>* m_Coeff;
    const uint kHistMask = 31;
    static const uint kHistSize = 32;
    double m_inHistory[kHistSize];
    double m_outHistory[kHistSize];
    uint m_histIdx = 0;
    bool m_invertDenormal = false;
    const float kDenormal = 0.000000000000001f;
    /// Returns true if n is odd
    inline bool Odd(uint n) { return (n & 1) == 1; }
    /// Square
    inline float Sqr(float value) { return value * value; }
    /// Square
    inline double Sqr(double value) { return value * value; }
    /// Determines poles and zeros of IIR filter
    /// based on bilinear transform method
    void LocatePolesAndZeros()
    {
        m_W.assign(m_order + 1, {0,0});
        m_z.assign(m_order + 1, 0);
        cplx<double>* W=m_W.data();
        double* z=m_z.data();
        const double ln10 = log(10.0);
        // Butterworth, Chebyshev parameters
        uint n = m_order;
        if (m_filterType == BP) n = n / 2;
        const uint ir = n % 2;
        const uint n1 = n + ir;
        const uint n2 = (3 * n + ir) / 2 - 1;
        double f1 = 0;
        if (m_filterType == LP)
        {
            f1 = m_fp2;
        }
        else if (m_filterType == HP)
        {
            f1 = m_fN - m_fp1;
        }
        else if (m_filterType == BP)
        {
            f1 = m_fp2 - m_fp1;
        }
        const double tanw1 = tan(0.5 * M_PI * f1 / m_fN);
        const double tansqw1 = Sqr(tanw1);
        // Real and Imaginary parts of low-pass poles
        double a = 1.0;
        cplx<double> temp = {1,1};
        for (uint k = n1; k <= n2; k++)
        {
            const double t = 0.5 * (2 * k + 1 - ir) * M_PI / n;
            if (m_protoType==Butterworth)
            {
                const double b3 = 1.0 - 2.0 * tanw1 * cos(t) + tansqw1;
                temp = {(1.0 - tansqw1) / b3, (2.0 * tanw1 * sin(t)) / b3};
            }
            else if (m_protoType==Chebyshev)
            {
                const double d = 1.0 - exp(-0.05 * m_ripple * ln10);
                const double e = 1.0 / sqrt(1.0 / Sqr(1.0 - d) - 1.0);
                const double x = pow(sqrt(e * e + 1.0) + e, 1.0 / n);
                a = 0.5 * (x - 1.0 / x);
                const double b = 0.5 * (x + 1.0 / x);
                const double c3 = a * tanw1 * cos(t);
                const double c4 = b * tanw1 * sin(t);
                const double c5 = Sqr(1.0 - c3) + Sqr(c4);
                temp = {2.0 * (1.0 - c3) / c5 - 1.0, 2.0 * c4 / c5};
            }
            const uint m = 2 * (n2 - k) + 1;
            W[m + ir] = {temp.real, fabs(temp.imag)};
            W[m + ir + 1] = cplxConj(W + m + ir);
        }
        if (Odd(n))
        {
            if (m_protoType == Butterworth)
            {
                temp.real = (1.0 - tansqw1) / (1.0 + 2.0 * tanw1 + tansqw1);
            }
            else if (m_protoType == Chebyshev)
            {
                temp.real = 2.0 / (1.0 + a * tanw1) - 1.0;
            }
            W[1] = {temp.real,0};
        }
        if (m_filterType==LP)
        {
            for (uint m = 1; m <= n; m++) z[m] = -1.0;
        }
        else if (m_filterType==HP) // Low-pass to high-pass transformation
        {
            for (uint m = 1; m <= n; m++)
            {
                W[m].real = -W[m].real;
                z[m] = 1.0;
            }
        }
        else if (m_filterType==BP) // Low-pass to bandpass transformation
        {
            for (uint m = 1; m <= n; m++)
            {
                z[m] = 1.0;
                z[m + n] = -1.0;
            }
            const double f4 = 0.5 * M_PI * m_fp1 / m_fN;
            const double f5 = 0.5 * M_PI * m_fp2 / m_fN;
            const double aa = cos(f4 + f5) / cos(f5 - f4);
            cplx<double> p1 = {0,0}, p2={0,0};
            for (uint m1 = 0; m1 <= (m_order - 1) / 2; m1++)
            {
                const uint m = 1 + 2 * m1;
                const cplx<double> a = W[m];
                if (fabs(a.imag) < 0.0001)
                {
                    const double h1 = 0.5 * aa * (1.0 + a.real);
                    const double h2 = Sqr(h1) - a.real;
                    if (h2 > 0.0)
                    {
                        p1 = {h1 + sqrt(h2), 0};
                        p2 = {h1 - sqrt(h2), 0};
                    }
                    else
                    {
                        p1 = {h1, sqrt(fabs(h2))};
                        p2 = cplxConj(&p1);//{h1, -p1.imag};
                    }
                }
                else
                {
                    const cplx<double> f = {aa * 0.5 * (1.0 + a.real),(aa * 0.5) * a.imag};
                    const cplx<double> g = {Sqr(f.real) - Sqr(f.imag) - a.real,2 * f.real * f.imag - a.imag};
                    const double sR = sqrt(0.5 * fabs(g.real + sqrt(Sqr(g.real) + Sqr(g.imag))));
                    const cplx<double> s = {sR,g.imag / (2.0 * sR)};
                    p1 = cplxAdd(&f,&s);
                    p2 = cplxSub(&f,&s);
                    /*
                    p1.real = f.real + s.real;
                    p1.imag = f.imag + s.imag;
                    p2.real = f.real - s.real;
                    p2.imag = f.imag - s.imag;
                    */
                }
                W[m] = p1;
                W[m + 1] = p2;
            }
            if (Odd(n))
            {
                W[2] = W[n + 1];
            }
            for (uint k = n; k >= 1; k--)
            {
                const uint m = 2 * k - 1;
                /*
                W[m].real = W[k].real;
                W[m + 1].real = W[k].real;
                W[m].imag = std::fabs(W[k].imag);
                W[m + 1].imag = -std::fabs(W[k].imag);
                */
                W[m] = {W[k].real,fabs(W[k].imag)};
                W[m + 1] = cplxConj(W + m);// {W[k].real,-std::fabs(W[k].imag)};
            }
        }
    }
public:
    /// <summary>
    /// Calculate all the values
    /// </summary>
    void Design()
    {
        if (!FilterValid()) return;
        // For bandpass, the order must be even
        if (m_filterType == BP && (m_order & 1) != 0) m_order++;
        std::vector<cplx<double>> temp(m_order + 1,{0,0});
        LocatePolesAndZeros(); // Find filter poles and zeros
        m_CoeffVector.assign(m_order + 1,{0,0});
        m_Coeff=m_CoeffVector.data();
        cplx<double>* W=m_W.data();
        double* z=m_z.data();
        cplx<double>* tmp=temp.data();
        m_Coeff[0] = {1,1};  // Compute filter coefficients from pole/zero values
        uint k = 0;
        const uint n = m_order;
        const uint pairs = n / 2;
        if (Odd(m_order)) // First subfilter is first order
        {
            m_Coeff[1] = cplxCreate<double>(-z[1], -W[1].real);
            k = 1;
        }
        for (uint p = 1; p <= pairs; p++)
        {
            const uint m = 2 * p - 1 + k;
            const double alpha1 = -(z[m] + z[m + 1]);
            const double alpha2 = z[m] * z[m + 1];
            const double beta1 = -2.0 * W[m].real;
            const double beta2 = Sqr(W[m].real) + Sqr(W[m].imag);
            tmp[1].real = m_Coeff[1].real + alpha1 * m_Coeff[0].real;
            tmp[1].imag = m_Coeff[1].imag + beta1 * m_Coeff[0].imag;
            for (uint i = 2; i <= n; i++)
            {
                tmp[i].real = m_Coeff[i].real + alpha1 * m_Coeff[i - 1].real + alpha2 * m_Coeff[i - 2].real;
                tmp[i].imag = m_Coeff[i].imag + beta1 * m_Coeff[i - 1].imag + beta2 * m_Coeff[i - 2].imag;
            }
            memcpy(m_Coeff+1,tmp+1,n*sizeof(cplx<double>));
        }
        // Ensure the filter is normalized
        FilterGain(1000);
        Reset();
    }
    /// Reset the history buffers
    void Reset()
    {
        for (uint i = 0; i< kHistSize; i++) m_inHistory[i]=0;
        for (uint i = 0; i< kHistSize; i++) m_outHistory[i]=0;
        m_histIdx = 0;
    }
    /// Reset the filter, and fill the appropriate history buffers with the specified value
    void Reset(double startValue)
    {
        m_histIdx = 0;
        for (uint i = 0; i< kHistSize; i++) m_inHistory[i]=startValue;
        if (m_filterType==LP)
        {
            for (uint i = 0; i< kHistSize; i++) m_outHistory[i]=startValue;
        }
        else
        {
            for (uint i = 0; i< kHistSize; i++) m_outHistory[i]=0;
        }
    }
    /// Apply the filter to the buffer
    void FilterBuffer(float* srcBuf, ulong srcPos, float* dstBuf, ulong dstPos, ulong nLen)
    {
        const float denormal = m_invertDenormal ? -kDenormal : kDenormal;
        m_invertDenormal = !m_invertDenormal;
        srcBuf += srcPos;
        dstBuf += dstPos;
        for (ulong sampleIdx = 0; sampleIdx < nLen; sampleIdx++)
        {
            *(dstBuf++) = FilterSample(*(srcBuf++) + denormal);
        }
    }
    inline double FilterSample(double inVal)
    {
        cplx<double>* coeff = m_Coeff;
        uint histIdx = m_histIdx;
        m_inHistory[histIdx] = inVal;
        double sum = coeff->real * inVal;
        for (uint idx = 1; idx < m_order+1; idx++)
        {
            histIdx--;
            histIdx &= kHistMask;
            coeff++;
            sum += coeff->real * m_inHistory[histIdx];
            sum -= coeff->imag * m_outHistory[histIdx];
        }
        m_outHistory[m_histIdx++] = sum;
        m_histIdx &= kHistMask;
        return sum;
    }
    /// Get the gain at the specified number of frequency points
    std::vector<double> FilterGain(uint freqPoints)
    {
        // Filter gain at uniform frequency intervals
        std::vector<double> g(freqPoints,0);
        double gMax = -100.0;
        const double sc = 10.0 / log(10.0);
        const double t = M_PI / (freqPoints - 1);
        for (uint i = 0; i < freqPoints; i++)
        {
            double theta = i * t;
            if (i == 0) theta = M_PI * 0.0001;
            if (i == freqPoints - 1) theta = M_PI * 0.9999;
            double sac = 0.0;
            double sas = 0.0;
            double sbc = 0.0;
            double sbs = 0.0;
            for (uint k = 0; k <= m_order; k++)
            {
                const double c = cos(k * theta);
                const double s = sin(k * theta);
                sac += c * m_Coeff[k].real;
                sas += s * m_Coeff[k].real;
                sbc += c * m_Coeff[k].imag;
                sbs += s * m_Coeff[k].imag;
            }
            g[i] = sc * log((Sqr(sac) + Sqr(sas)) / (Sqr(sbc) + Sqr(sbs)));
            gMax = fmax(gMax, g[i]);
        }
        // Normalize to 0 dB maximum gain
        for (uint i = 0; i < freqPoints; i++) g[i] -= gMax;
        // Normalize numerator (a) coefficients
        double normFactor = pow(10.0, -0.05 * gMax);
        for (uint i = 0; i <= m_order; i++) m_Coeff[i].real *= normFactor;
        return g;
    }
};

#endif // CIIRFILTERS_H
