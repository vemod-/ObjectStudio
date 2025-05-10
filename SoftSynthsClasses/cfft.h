//   cfft.h - declaration of class
//   of fast Fourier transform - FFT
//
//   The code is property of LIBROW
//   You can use it on your own
//   When utilizing credit LIBROW site

#ifndef CFFT_H
#define CFFT_H

//   Include complex numbers header
#include <complex>
#include "math.h"
#include <QMutexLocker>

typedef std::complex<double> Complex;

template <typename T>
struct cplx
{
    T real;
    T imag;
};

template <typename T>
inline cplx<T> cplxCreate(const float real, const float imag)
{
    cplx<T> r;
    r.real = real;
    r.imag = imag;
    return r;
}

template <typename T>
inline cplx<T> cplxCreate(const double real, const double imag)
{
    cplx<T> r;
    r.real = real;
    r.imag = imag;
    return r;
}

template <typename T>
inline cplx<T> cplxAdd(const cplx<T>* a, const cplx<T>* b)
{
    cplx<T> r;
    r.real = a->real + b->real;
    r.imag = a->imag + b->imag;
    return r;
}

template <typename T>
inline cplx<T> cplxSub(const cplx<T>* a, const cplx<T>* b)
{
    cplx<T> r;
    r.real = a->real - b->real;
    r.imag = a->imag - b->imag;
    return r;
}

template <typename T>
inline cplx<T> cplxMult(const cplx<T>* a, const cplx<T>* b)
{
    cplx<T> r;
    r.real = a->real * b->real - a->imag * b->imag;
    r.imag = a->real * b->imag + a->imag * b->real;
    return r;
}

template <typename T>
inline cplx<T> cplxInv(const cplx<T>* a)
{
    cplx<T> r;
    T denom = a->real * a->real + a->imag * a->imag;
    r.real =  a->real / denom;
    r.imag = -a->imag / denom;
    return r;
}

template <typename T>
inline cplx<T> cplxNeg(const cplx<T>* a)
{
    cplx<T> r;
    r.real = -a->real;
    r.imag = -a->imag;
    return r;
}

template <typename T>
inline cplx<T> cplxConj(const cplx<T>* a)
{
    cplx<T> r;
    r.real =  a->real;
    r.imag = -a->imag;
    return r;
}

class CFFT
{
public:
    CFFT(uint size=256)
    {
        setSize(size);
    }
    ~CFFT()
    {
        if (W)
        {
            delete [] W;
            delete [] Wi;
            delete [] T;
        }
    }
    void setSize(uint size)
    {
        QMutexLocker locker(&mutex);
        N=size;
        if (W)
        {
            delete [] W;
            delete [] Wi;
            delete [] T;
        }
        W=new Complex[N];
        Wi=new Complex[N];
        T=new uint[N];
        uint Target = 0;
        for (uint Position = 0; Position < N; Position++)
        {
            T[Position] = Target;
            uint Mask = N; //   Bit mask
            //   While bit is set
            while (Target & (Mask >>= 1)) Target &= ~Mask; //   Drop bit
            Target |= Mask; //   The current bit is 0 - set it
        }
        for (uint Step = 1; Step < N; Step <<= 1)
        {
            const double delta = -M_PI / Step; //   Angle increment
            const double Sine = sin(delta * 0.5); //   Auxiliary sin(delta / 2)
            W[Step]=Complex(-2.0 * Sine * Sine, sin(delta)); //   Multiplier for trigonometric recurrence
        }
        for (uint Step = 1; Step < N; Step <<= 1)
        {
            const double delta = M_PI / Step; //   Angle increment
            const double Sine = sin(delta * 0.5); //   Auxiliary sin(delta / 2)
            Wi[Step]=Complex(-2.0 * Sine * Sine, sin(delta)); //   Multiplier for trigonometric recurrence
        }
    }
    inline uint size() const { return N; }
    bool Forward(const float * Input, Complex *const Output, const float* Window, const float scale)
    {
        //   Check input parameters
        if (!Input || !Output || N < 1 || N & (N - 1)) return false;
        //   Initialize data
        Rearrange(Input, Output, Window, scale);
        //   Call FFT implementation
        Perform(Output);
        //   Succeeded
        return true;
    }
    bool Forward(const float * Input, Complex *const Output, const float* Window)
    {
        //   Check input parameters
        if (!Input || !Output || N < 1 || N & (N - 1)) return false;
        //   Initialize data
        Rearrange(Input, Output, Window);
        //   Call FFT implementation
        Perform(Output);
        //   Succeeded
        return true;
    }
    bool Forward(const float * Input, Complex *const Output, const float scale)
    {
        //   Check input parameters
        if (!Input || !Output || N < 1 || N & (N - 1)) return false;
        //   Initialize data
        Rearrange(Input, Output, scale);
        //   Call FFT implementation
        Perform(Output);
        //   Succeeded
        return true;
    }
    bool Forward(const float * Input, Complex *const Output)
    {
        //   Check input parameters
        if (!Input || !Output || N < 1 || N & (N - 1)) return false;
        //   Initialize data
        Rearrange(Input, Output);
        //   Call FFT implementation
        Perform(Output);
        //   Succeeded
        return true;
    }
    //   FORWARD FOURIER TRANSFORM
    //     Input  - input data
    //     Output - transform result
    //     N      - length of both input data and result
    bool Forward(const Complex *const Input, Complex *const Output)
    {
        //   Check input parameters
        if (!Input || !Output || N < 1 || N & (N - 1)) return false;
        //   Initialize data
        Rearrange(Input, Output);
        //   Call FFT implementation
        Perform(Output);
        //   Succeeded
        return true;
    }
    //   FORWARD FOURIER TRANSFORM, INPLACE VERSION
    //     Data - both input data and output
    //     N    - length of input data
    bool Forward(Complex *const Data)
    {
        //   Check input parameters
        if (!Data || N < 1 || N & (N - 1)) return false;
        //   Rearrange
        Rearrange(Data);
        //   Call FFT implementation
        Perform(Data);
        //   Succeeded
        return true;
    }
    bool AccumulateInverse(Complex *const Input, float * Output, const float* Filter, const double scale = 1)
    {
        const double Factor = scale / double(N);
        //   Check input parameters
        if (!Input || !Output || N < 1 || N & (N - 1)) return false;
        //   Initialize data
        Rearrange(Input);
        //   Call FFT implementation
        Perform(Input, true);
        //   Scale if necessary
        for (uint Position = 0; Position < N; Position++) Output[Position] += Input[Position].real() * Filter[Position] * Factor;
        //   Succeeded
        return true;
    }
    bool Inverse(Complex *const Input, float * Output, const double scale = 1)
    {
        const double Factor = scale / double(N);
        //   Check input parameters
        if (!Input || !Output || N < 1 || N & (N - 1)) return false;
        //   Initialize data
        Rearrange(Input);
        //   Call FFT implementation
        Perform(Input, true);
        //   Scale if necessary
        for (uint Position = 0; Position < N; Position++) Output[Position] = Input[Position].real() * Factor;
        //   Succeeded
        return true;
    }
    //   INVERSE FOURIER TRANSFORM
    //     Input  - input data
    //     Output - transform result
    //     N      - length of both input data and result
    //     Scale  - if to scale result
    bool Inverse(const Complex *const Input, Complex *const Output, const bool Scale = true)
    {
        //   Check input parameters
        if (!Input || !Output || N < 1 || N & (N - 1)) return false;
        //   Initialize data
        Rearrange(Input, Output);
        //   Call FFT implementation
        Perform(Output, true);
        //   Scale if necessary
        if (Scale) CFFT::Scale(Output);
        //   Succeeded
        return true;
    }
    //   INVERSE FOURIER TRANSFORM, INPLACE VERSION
    //     Data  - both input data and output
    //     N     - length of both input data and result
    //     Scale - if to scale result
    bool Inverse(Complex *const Data, const bool Scale = true)
    {
        //   Check input parameters
        if (!Data || N < 1 || N & (N - 1)) return false;
        //   Rearrange
        Rearrange(Data);
        //   Call FFT implementation
        Perform(Data, true);
        //   Scale if necessary
        if (Scale) CFFT::Scale(Data);
        //   Succeeded
        return true;
    }
    void Hermitian(Complex *const Data)
    {
        if (N <= 2) return; 	// nothing to do
        uint i = (N >> 1) - 1;			// input
        uint j = i + 2;				// output
        while (i > 0) Data[j++] = conj(Data[i--]);
    }
protected:
    //   Rearrange function and its inplace version
    void Rearrange(const float * Input, Complex *const Output, const float* Window, const float scale)
    {
        //   Data entry position
        //   Process all positions of input signal
        for (uint Position = 0; Position < N; Position++)
        {
            Output[T[Position]] = Input[Position] * Window[Position] * scale; //  Set data entry
        }
    }
    void Rearrange(const float * Input, Complex *const Output, const float* Window)
    {
        //   Data entry position
        //   Process all positions of input signal
        for (uint Position = 0; Position < N; Position++)
        {
            Output[T[Position]] = Input[Position] * Window[Position]; //  Set data entry
        }
    }
    void Rearrange(const float * Input, Complex *const Output, const float scale)
    {
        //   Data entry position
        //   Process all positions of input signal
        for (uint Position = 0; Position < N; Position++)
        {
            Output[T[Position]] = Input[Position]*scale; //  Set data entry
        }
    }
    void Rearrange(const float * Input, Complex *const Output)
    {
        //   Data entry position
        //   Process all positions of input signal
        for (uint Position = 0; Position < N; Position++)
        {
            Output[T[Position]] = Input[Position]; //  Set data entry
        }
    }
    void Rearrange(const Complex *const Input, Complex *const Output)
    {
        //   Data entry position
        //   Process all positions of input signal
        for (uint Position = 0; Position < N; Position++)
        {
            Output[T[Position]] = Input[Position]; //  Set data entry
        }
    }
    void Rearrange(Complex *const Data)
    {
        //   Swap position
        //   Process all positions of input signal
        for (uint Position = 0; Position < N; Position++)
        {
            //   Only for not yet swapped entries
            if (T[Position] > Position) std::swap(Data[T[Position]],Data[Position]); //   Swap entries
        }
    }
    //   FFT implementation
    void Perform(Complex *const Data, const bool Inverse = false)
    {
        if (N == 1) return;
        for (uint Pair = 0; Pair < N; Pair += 2) //   Iteration within first group
        {
            const uint Match = Pair + 1; //   Match position
            const Complex Product(Data[Match]); //   Second term of two-point transform
            Data[Match] = Data[Pair] - Product; //   Transform for fi + pi
            Data[Pair] += Product; //   Transform for fi
        }
        if (N == 2) return;
        const Complex* p = Inverse ? Wi : W;
        for (uint Step = 2; Step < N; Step <<= 1) //   Iteration through dyads, quadruples, octads and so on...
        {
            const uint Jump = Step << 1; //   Jump to the next entry of the same transform factor
            Complex Factor(1.0); //   Start value for transform factor, fi = 0
            for (uint Group = 0; Group < Step; Group++) //   Iteration through groups of different transform factor
            {
                if (Group > 0) Factor = p[Step] * Factor + Factor; //   Successive transform factor via trigonometric recurrence
                for (uint Pair = Group; Pair < N; Pair += Jump) //   Iteration within group
                {
                    const uint Match = Pair + Step; //   Match position
                    const Complex Product(Factor * Data[Match]); //   Second term of two-point transform
                    Data[Match] = Data[Pair] - Product; //   Transform for fi + pi
                    Data[Pair] += Product; //   Transform for fi
                }
            }
        }
    }
    //   Scaling of inverse FFT result
    void Scale(Complex *const Data)
    {
        const double Factor = 1.0 / double(N);
        //   Scale all data entries
        for (uint Position = 0; Position < N; Position++) Data[Position] *= Factor;
    }
    uint N;
    Complex* W = nullptr;
    Complex* Wi = nullptr;
    uint* T = nullptr;
    QMutex mutex;
};

/*
>> I have a C++ template class for doing FFTs using
>> a 'Complex' class. It was actually done for research
>> into rounding effects with an integer FFT. So, you
>> can define a 'frac24complex' for instance (24-bit
>> fractional complex #'s) and apply the fft to that class
>> instead of a 'standard' complex class.
>>
>> So, it's fairly canonical and general. It uses a
>> technique I have frequently used, where the 'twiddle'
>> factors are stored in a precomputed bit-reversed table.
>> exp[0], exp[pi*i/2], exp[pi*i/4], exp[3*pi*i/4], exp[pi*i/8] ...
>>

It contains reasonable
self-documentation and has been tested under VC++4 with
a 'complex' class meeting only the minimum requirements
as defined in cfft.h.

 * This class is used as follows:
 */
//   #include <fft.h>
//   ...
//   CFFTpre  FFT256( 256 ); // build an operator object
//               // The constructor builds tables and places them
//               // in the object.
//   Complex Array[256];
//   ...
//   FFT256.fft( Array );		// forward transform
//   FFT256.ifft( Array );     // reverse transform.
//
/*
class CFFTpre
{
private:
    QMutex mutex;
    uint N, log2N;		// these define size of FFT buffer
    Complex *w;			// array [N/2] of cos/sin values
    Complex *wi;
    uint *bitrev;		// bit-reversing table, in 0..N
    void Perform(Complex *buf, int iflag)
    {
        if (log2N == 0) return; // only 1 element !
        // first pass: //  1st element  = sum of 1st & middle, middle element = diff. // repeat N/2 times.
        uint k = N >> 1;
        Complex* buf2 = buf + k;
        for (uint i = 0; i < k; i++)
        {		// first pass is faster
            const Complex T = buf2[i];
            buf2[i] = buf[i] - T;
            buf[i] += T;
        }
        if (log2N == 1) return;	// only 2!

        const Complex* bufe = buf + N;		// past end
        const Complex* wp = iflag ? wi : w; // get w-factor
        for (uint l = N >> 2; l > 0; l >>= 1) // k is N/4 now
        {
            Complex* buf0 = buf;
            for (uint j = 0; buf0 < bufe; j++)
            {
                Complex* buf2 = buf0 + l;
                for (uint i = 0; i < l; i++)
                {	// a butterfly
                    const Complex T = wp[j] * buf2[i];
                    buf2[i] = buf0[i] - T;
                    buf0[i] += T;
                }
                buf0 += (l << 1);
            }
        }
        for (uint i = 0; i < N; i++) // bitrev the sucker
        {
            const uint j = bitrev[i];
            if (i > j) std::swap(buf[i],buf[j]);
        }
    }
    //   Scaling of inverse FFT result
    void Scale(Complex *const Data)
    {
        const double Factor = 1.0 / double(N);
        //   Scale all data entries
        for (uint Position = 0; Position < N; ++Position)
            Data[Position] *= Factor;
    }
public:
    // constructor takes an int, power-of-2.
    CFFTpre(int size = 256) : w(nullptr), bitrev(nullptr)
    {
        setSize(size);
    }
    void setSize(int size)
    {
        QMutexLocker locker(&mutex);
        uint k;
        for (k = 0; ; k++)
        {
            if ((1 << k) == size) break;
            if (k==14 || (1 << k) > size) return; //throw "cfft: size not power of 2";
        }
        N = 1 << k;
        log2N = k;

        if (bitrev) delete [] bitrev;
        bitrev = new uint [N];

        if (w)
        {
            delete [] w;
            delete [] wi;
        }
        w = new Complex[N >> 1];
        wi = new Complex[N >> 1];
        // do bit-rev table
        bitrev[0] = 0;
        for (uint j = 1; j < N; j <<= 1)
        {
            for(uint i = 0; i < j ; i++)
            {
                bitrev[i] <<= 1;
                bitrev[i+j] = bitrev[i]+1;
            }
        }
        //
        // prepare the cos/sin table. This is bit-reversed, and goes
        // like this: 0, 90, 45, 135, 22.5 ...  for N/2 entries.
        k = (1 << (k - 1));
        for (uint i = 0; i < k; i++)
        {
            const double t = double(bitrev[i << 1]) * M_PI /double(k);
            const Complex ww = Complex(cos(t), sin(t));
            wi[i] = ww;
            w[i] = conj(ww);		// force limiting of imag part if applic.
        }
    }
    // destructor frees the memory
    ~CFFTpre()
    {
        if (bitrev) delete [] bitrev;
        if (w)
        {
            delete [] w;
            delete [] wi;
        }
    }
    inline void Forward(Complex *buf) { Perform(buf,0); }
    inline void Inverse(Complex *buf, bool scale=true)
    {
        Perform(buf,1);
        if (scale) Scale(buf);
    }
    inline uint size() const { return N; }

    // hermitian() assumes the array has been filled in with values
    // up to the center and including the center point. It reflects these
    // conjugate-wise into the last half.
    inline void Hermitian(Complex *buf)
    {
        if (N <= 2) return; 	// nothing to do
        int i = (N >> 1) - 1;			// input
        int j = i + 2;				// output
        while (i > 0) buf[j++] = conj(buf[i--]);
    }
}; // class cfft
*/
template <typename T>

class CFFTinterleaved
{
public:
    CFFTinterleaved() : CFFTinterleaved(256) {}
    CFFTinterleaved(uint s)
    {
        setSize(s);
    }
    ~CFFTinterleaved()
    {
        if (buf) delete [] buf;
        if (m_w)
        {
            delete [] m_w;
            delete [] m_wi;
            delete [] m_bitrev;
        }
    }
    void setSize(uint s)
    {
        QMutexLocker locker(&mutex);
        size = s;
        m_log2s = uint(log(size)/log(2.0));
        if (buf) delete [] buf;
        buf = new T[s*2];
        cplxbuf=reinterpret_cast<cplx<T>*>(buf);
        endcplxbuf=cplxbuf+size;
        if (m_w)
        {
            delete [] m_w;
            delete [] m_wi;
            delete [] m_bitrev;
        }
        m_w = new cplx<T>[m_log2s];
        m_wi = new cplx<T>[m_log2s];
        m_bitrev = new uint[size];
        for (long k = 0, jump = 2; k < m_log2s; k++)
        {
            const long step = jump;
            jump <<= 1;
            const T arg = M_PI / (step >> 1);
            m_w[k].real = std::cos(arg);
            m_w[k].imag = std::sin(arg);
            m_wi[k].real = std::cos(arg);
            m_wi[k].imag = -std::sin(arg);
        }
        for (uint i = 1; i < size; i++)
        {
            uint j = 0;
            for (uint bitm = 1; bitm < size; bitm <<= 1)
            {
                if (i & bitm) j++;
                j <<= 1;
            }
            j >>= 1;
            m_bitrev[i]=j;
        }
    }
    void Forward() { Perform(-1); }
    void Forward(float* b)
    {
        interleave(b);
        Forward();
    }
    void Forward(float* b, float* win)
    {
        interleave(b, win);
        Forward();
    }
    void Inverse() { Perform(1); }
    void Inverse(float* b)
    {
        Inverse();
        deinterleave(b);
    }
    void Inverse(float* b, float* win)
    {
        Inverse();
        deinterleave(b,win);
    }
    void Inverse(float* b, float* win, const T f)
    {
        Inverse();
        deinterleave(b,win,f);
    }
    void Hermitian()
    {
        cplx<T>* p1 = cplxbuf + (size >> 1) -1; // input
        cplx<T>* p2 = p1 + 2; // output
        while (p1 >= cplxbuf)
        {
            p2->real = p1->real;
            (p2++)->imag = -(p1--)->imag;
        }
    }
    inline T magn(const uint k) const
    {
        const cplx<T>* p=cplxbuf+k;
        return T(2.0) * std::sqrt(p->real * p->real + p->imag * p->imag);
    }
    inline T phase(const uint k) const
    {
        const cplx<T>* p=cplxbuf+k;
        return std::atan2(p->imag,p->real);
    }
    inline void polar(const T magn, const T phase, const uint k)
    {
        cplx<T>* p=cplxbuf+k;
        p->real = magn*std::cos(phase); p->imag = magn*std::sin(phase);
    }
    inline void interleave(float* b)
    {
        for (cplx<T>* p = cplxbuf; p < endcplxbuf; p++)
        {
            *p = {*b++, 0};
        }
    }
    inline void interleave(float* b, float* win)
    {
        for (cplx<T>* p = cplxbuf; p < endcplxbuf; p++)
        {
            *p = {*b++ * *win++, 0};
        }
    }
    inline void interleave(const T val, const uint k)
    {
        cplxbuf[k] = {val,0};
    }
    inline void deinterleave(float* b)
    {
        for (cplx<T>* p = cplxbuf; p < endcplxbuf; p++)
        {
            *(b++) += p->real;
        }
    }
    inline void deinterleave(float* b, float* win)
    {
        for (cplx<T>* p = cplxbuf; p < endcplxbuf; p++)
        {
            *(b++) += *win++ * p->real;
        }
    }
    inline void deinterleave(float* b, float* win, const T f)
    {
        for (cplx<T>* p = cplxbuf; p < endcplxbuf; p++)
        {
            *(b++) += *win++ * p->real * f;
        }
    }
    inline T real(const uint k)
    {
        return cplxbuf[k].real;
    }
    inline T imag(const uint k)
    {
        return cplxbuf[k].imag;
    }
private:
    QMutex mutex;
    /*
    struct cplx
    {
        T real;
        T imag;
    };
    */
    cplx<T>* cplxbuf;
    cplx<T>* endcplxbuf;
    T* buf=0;
    uint size;
    uint m_log2s;
    cplx<T>* m_w = nullptr;
    cplx<T>* m_wi = nullptr;
    uint* m_bitrev = nullptr;
    void Perform(long sign)
    /*
            FFT routine. Sign = -1 is FFT, 1 is iFFT (inverse)
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
        cplx<T>* p1 = cplxbuf + 1;
        cplx<T>* p2;
        cplx<T> temp;
        cplx<T> u;
        for (uint i = 1; i < size - 1; i++)
        {
            const uint j = m_bitrev[i];
            if (i < j)
            {
                p2 = cplxbuf + j;
                temp.real = p1->real; temp.imag = p1->imag; p1->real = p2->real; p1->imag = p2->imag; p2->real = temp.real; p2->imag = temp.imag;
            }
            p1++;
        }
        uint jump = 2;
        for (p1 = cplxbuf, p2 = p1 + 1; p1 < endcplxbuf; p1 += jump, p2 += jump) // first step
        {
            temp.real = p2->real; temp.imag = p2->imag;
            p2->real = p1->real - temp.real; p2->imag = p1->imag - temp.imag;
            p1->real += temp.real; p1->imag += temp.imag;
        }
        cplx<T>* w = (sign==1) ? m_w : m_wi;
        for (uint k = 1; k < m_log2s; k++)
        {
            const uint step = jump;
            jump <<= 1;
            u.real = 1; u.imag = 0;
            w++;
            uint pair;
            for (pair = 0; pair < step - 1; pair++) // following steps
            {
                for (p1 = cplxbuf + pair, p2 = p1 + step; p1 < endcplxbuf; p1 += jump, p2 += jump)
                {
                    temp.real = p2->real * u.real - p2->imag * u.imag; temp.imag = p2->real * u.imag + p2->imag * u.real;
                    p2->real = p1->real - temp.real; p2->imag = p1->imag - temp.imag;
                    p1->real += temp.real; p1->imag += temp.imag;
                }
                const T tr = u.real * w->real - u.imag * w->imag; u.imag = u.real * w->imag + u.imag * w->real; u.real = tr;
            }
            for (p1 = cplxbuf + pair, p2 = p1 + step; p1 < endcplxbuf; p1 += jump, p2 += jump) // last step
            {
                temp.real = p2->real * u.real - p2->imag * u.imag; temp.imag = p2->real * u.imag + p2->imag * u.real;
                p2->real = p1->real - temp.real; p2->imag = p1->imag - temp.imag;
                p1->real += temp.real; p1->imag += temp.imag;
            }
        }
    }
};

template <typename T>

class CFFTtwiddle
{
public:
    enum TTransFormType {FORWARD, INVERSE};
    CFFTtwiddle() : CFFTtwiddle(256) {}
    CFFTtwiddle(uint s) { setSize(s); }
    ~CFFTtwiddle()
    {
        if (BufferR)
        {
            delete [] InputR;
            delete [] InputI;
            delete [] BufferR;
            delete [] BufferI;
            delete [] TwiddleR;
            delete [] TwiddleI;
            delete [] iTwiddleR;
            delete [] iTwiddleI;
            delete [] RevBits;
        }
    }
    void setSize(uint s)
    {
        QMutexLocker locker(&mutex);
        N = s;

        LogTwoOfN = uint( log(double(N)) / M_LN2 + 0.5 );

        if (BufferR)
        {
            delete [] InputR;
            delete [] InputI;
            delete [] BufferR;
            delete [] BufferI;
            delete [] TwiddleR;
            delete [] TwiddleI;
            delete [] iTwiddleR;
            delete [] iTwiddleI;
            delete [] RevBits;
        }
        // Memory allocation for all the arrays.
        InputR  = new T[N];
        InputI  = new T[N];
        BufferR  = new T[N];
        BufferI  = new T[N];
        TwiddleR = new T[N/2];
        TwiddleI = new T[N/2];
        iTwiddleR = new T[N/2];
        iTwiddleI = new T[N/2];
        RevBits  = new uint[N];
        ResultR = InputR;
        ResultI = InputI;
        uint k, J, K;

        J = N/2;
        K = 1;
        RevBits[0] = 0;
        while(J >= 1)
        {
            for(k=0; k<K; k++)
            {
                RevBits[k+K] = RevBits[k] + J;
            }
            K *= 2;
            J /= 2;
        }
        FillTwiddleArray(TwiddleR, TwiddleI, N, FORWARD);
        FillTwiddleArray(iTwiddleR, iTwiddleI, N, INVERSE);
    }
    void Forward()
    {
        // Move the rearranged input values to Buffer.
        // Take note of the pointer swaps at the top of the transform algorithm.
        for(uint j=0; j<N; j++)
        {
            BufferR[j] = InputR[ RevBits[j]];
            BufferI[j] = 0;
        }
        Perform(InputR,InputI,BufferR,BufferI,TwiddleR,TwiddleI);
    }
    void Forward(float* b)
    {
        // Move the rearranged input values to Buffer.
        // Take note of the pointer swaps at the top of the transform algorithm.
        for(uint j=0; j<N; j++)
        {
            BufferR[j] = b[ RevBits[j]];
            BufferI[j] = 0;
        }
        Perform(InputR,InputI,BufferR,BufferI,TwiddleR,TwiddleI);
    }
    void Forward(float* b, float* win)
    {
        // Move the rearranged input values to Buffer.
        // Take note of the pointer swaps at the top of the transform algorithm.
        for(uint j=0; j<N; j++)
        {
            const uint rb = RevBits[j];
            BufferR[j] = b[rb] * win[rb];
            BufferI[j] = 0;
        }
        Perform(InputR,InputI,BufferR,BufferI,TwiddleR,TwiddleI);
    }
    void Inverse()
    {
        // Move the rearranged input values to Buffer.
        // Take note of the pointer swaps at the top of the transform algorithm.
        for(j=0; j<N; j++)
        {
            const uint rb = RevBits[j];
            BufferR[j] = InputR[rb];
            BufferI[j] = InputI[rb];
        }
        Perform(InputR,InputI,BufferR,BufferI,iTwiddleR,iTwiddleI);
    }
    void Inverse(float* b)
    {
        Inverse();
        for(j=0; j<N; j++) b[j] += ResultR[j];
    }
    void Inverse(float* b, float* win)
    {
        Inverse();
        for(j=0; j<N; j++) *b++ += ResultR[j] * *win++;
    }
    void Inverse(float* b, float* win, T f)
    {
        Inverse();
        for(j=0; j<N; j++) *b++ += ResultR[j] * *win++ * f;
    }
    void Hermitian()
    {
        T* p1r = InputR + (N >> 1) -1; // input
        T* p2r = p1r + 2; // output
        T* p1i = InputI + (N >> 1) -1; // input
        T* p2i = p1i + 2; // output
        while (p1r >= InputR)
        {
            *p2r++ = *p1r--;
            *p2i++ = -*p1i--;
        }
    }
    inline T magn(const uint k) const
    {
        const T* pr=ResultR+k;
        const T* pi=ResultI+k;
        return T(2.0) * std::sqrt(*pr * *pr + *pi * *pi);
    }
    inline T phase(const uint k) const
    {
        return std::atan2(ResultI[k],ResultR[k]);
    }
    inline void polar(const T magn, const T phase, const uint k)
    {
        InputR[k] = magn*std::cos(phase); InputI[k] = magn*std::sin(phase);
    }
    inline T real(const uint k)
    {
        return ResultR[k];
    }
    inline T imag(const uint k)
    {
        return ResultI[k];
    }
private:
    QMutex mutex;
    uint N, j, LogTwoOfN, *RevBits;
    T *InputR = nullptr, *InputI = nullptr;
    T *BufferR = nullptr, *BufferI = nullptr;
    T *TwiddleR = nullptr, *TwiddleI = nullptr;
    T *iTwiddleR = nullptr, *iTwiddleI = nullptr;
    T *ResultR = nullptr, *ResultI = nullptr;
    /*
     The Pentium takes a surprising amount of time to calculate the sine and cosine.
     You may want to make the twiddle arrays static if doing repeated FFTs of the same size.
     This uses 4 fold symmetry to calculate the twiddle factors. As a result, this function
     requires a minimum FFT size of 8.
    */
    #define M_SQRT_2 0.707106781186547524401
    void FillTwiddleArray(T *TwiddleR, T *TwiddleI, int N, TTransFormType Type)
    {
        int j;
        double Theta, TwoPiOverN;

        TwoPiOverN = 2.0 * M_PI / double(N);

        if(Type == FORWARD)
        {
            TwiddleR[0] = 1.0;
            TwiddleI[0] = 0.0;
            TwiddleR[N/4] = 0.0;
            TwiddleI[N/4] = -1.0;
            TwiddleR[N/8] = M_SQRT_2;
            TwiddleI[N/8] = -M_SQRT_2;
            TwiddleR[3*N/8] = -M_SQRT_2;
            TwiddleI[3*N/8] = -M_SQRT_2;
            for(j=1; j<N/8; j++)
            {
                Theta = double(j) * -TwoPiOverN;
                TwiddleR[j] = cos(Theta);
                TwiddleI[j] = sin(Theta);
                TwiddleR[N/4-j] = -TwiddleI[j];
                TwiddleI[N/4-j] = -TwiddleR[j];
                TwiddleR[N/4+j] = TwiddleI[j];
                TwiddleI[N/4+j] = -TwiddleR[j];
                TwiddleR[N/2-j] = -TwiddleR[j];
                TwiddleI[N/2-j] = TwiddleI[j];
            }
        }

        else
        {
            TwiddleR[0] = 1.0;
            TwiddleI[0] = 0.0;
            TwiddleR[N/4] = 0.0;
            TwiddleI[N/4] = 1.0;
            TwiddleR[N/8] = M_SQRT_2;
            TwiddleI[N/8] = M_SQRT_2;
            TwiddleR[3*N/8] = -M_SQRT_2;
            TwiddleI[3*N/8] = M_SQRT_2;
            for(j=1; j<N/8; j++)
            {
                Theta = double(j) * TwoPiOverN;
                TwiddleR[j] = cos(Theta);
                TwiddleI[j] = sin(Theta);
                TwiddleR[N/4-j] = TwiddleI[j];
                TwiddleI[N/4-j] = TwiddleR[j];
                TwiddleR[N/4+j] = -TwiddleI[j];
                TwiddleI[N/4+j] = TwiddleR[j];
                TwiddleR[N/2-j] = -TwiddleR[j];
                TwiddleI[N/2-j] = TwiddleI[j];
            }
        }

    }
    // The Fourier Transform.
    void Perform(T *InputR, T *InputI, T *BufferR, T *BufferI, T *TwiddleR, T *TwiddleI)
    {
        uint j, k, J, K, I, Twd;
        T *TempPointer;
        T TempR, TempI;

        J = N >> 1;     // J increments down to 1
        K = 1;       // K increments up to N/2
        while(J > 0) // Loops Log2(N) times.
        {
            // Swap pointers, instead doing this: for(j=0; j<N; j++) Input[j] = Buffer[j];
            // We start with a swap because of the swap in ReArrangeInput.
            TempPointer = InputR;
            InputR = BufferR;
            BufferR = TempPointer;
            TempPointer = InputI;
            InputI = BufferI;
            BufferI = TempPointer;
            I = 0;
            for(j=0; j<J; j++)
            {
                Twd = 0;
                for(k=0; k<K; k++) // Loops N/2 times for every value of J and K
                {
                    const uint KI = K+I;
                    TempR = InputR[KI] * TwiddleR[Twd] - InputI[KI] * TwiddleI[Twd];
                    TempI = InputR[KI] * TwiddleI[Twd] + InputI[KI] * TwiddleR[Twd];
                    BufferR[I] = InputR[I] + TempR;
                    BufferI[I] = InputI[I] + TempI;
                    BufferR[KI] = InputR[I] - TempR;
                    BufferI[KI] = InputI[I] - TempI;
                    I++;
                    Twd += J;
                }
                I += K;
            }
            K <<= 1;
            J >>= 1;
        }
        // The ReArrangeInput function swapped Input[] and Buffer[]. Then Transform()
        // swapped them again, LogTwoOfN times. Ultimately, these swaps must be done
        // an even number of times, or the pointer to Buffer gets returned.
        // So we must do one more swap here, for N = 16, 64, 256, 1024, ...
        //if(LogTwoOfN % 2 == 1)
        //{
        //    ResultR = InputR;
        //    ResultI = InputI;
        //}
        //else // if(LogTwoOfN % 2 == 0) // then the results are still in Buffer.
        //{
            ResultR = BufferR;
            ResultI = BufferI;
        //}
    }
};

template <typename T>

class CFFTtwiddleInterleaved
{
private:
    /*
    struct cplx
    {
        T real;
        T imag;
    };
    */
public:
    enum TTransFormType {FORWARD, INVERSE};
    CFFTtwiddleInterleaved() : CFFTtwiddleInterleaved(256) {}
    CFFTtwiddleInterleaved(uint s) { setSize(s); }
    ~CFFTtwiddleInterleaved()
    {
        QMutexLocker locker(&mutex);
        if (Buffer)
        {
            delete [] Input;
            delete [] Buffer;
            delete [] Twiddle;
            delete [] iTwiddle;
            delete [] RevBits;
            Buffer = nullptr;
        }
    }
    void setSize(uint s)
    {
        QMutexLocker locker(&mutex);
        N = s;

        LogTwoOfN = uint( log(double(N)) / M_LN2 + 0.5 );

        if (Buffer)
        {
            delete [] Input;
            delete [] Buffer;
            delete [] Twiddle;
            delete [] iTwiddle;
            delete [] RevBits;
        }
        // Memory allocation for all the arrays.
        Input  = new cplx<T>[N];
        Buffer  = new cplx<T>[N];
        Twiddle = new cplx<T>[N/2];
        iTwiddle = new cplx<T>[N/2];
        RevBits  = new uint[N];
        Result = Input;
        uint k, J, K;

        J = N/2;
        K = 1;
        RevBits[0] = 0;
        while(J >= 1)
        {
            for(k=0; k<K; k++)
            {
                RevBits[k+K] = RevBits[k] + J;
            }
            K *= 2;
            J /= 2;
        }
        FillTwiddleArray(Twiddle, N, FORWARD);
        FillTwiddleArray(iTwiddle, N, INVERSE);
    }
    void Forward()
    {
        QMutexLocker locker(&mutex);
        // Move the rearranged input values to Buffer.
        // Take note of the pointer swaps at the top of the transform algorithm.
        cplx<T>* c = Buffer;
        for(uint j=0; j<N; j++)
        {
            c->real = Input[ RevBits[j]].real;
            c->imag = 0;
            c++;
        }
        Result = Perform(Input,Buffer,Twiddle);
    }
    void Forward(float* b)
    {
        QMutexLocker locker(&mutex);
        // Move the rearranged input values to Buffer.
        // Take note of the pointer swaps at the top of the transform algorithm.
        cplx<T>* c = Buffer;
        for(uint j=0; j<N; j++)
        {
            c->real = b[ RevBits[j]];
            c->imag = 0;
            c++;
        }
        Result = Perform(Input,Buffer,Twiddle);
    }
    void Forward(float* b, float* win)
    {
        QMutexLocker locker(&mutex);
        // Move the rearranged input values to Buffer.
        // Take note of the pointer swaps at the top of the transform algorithm.
        cplx<T>* c = Buffer;
        for(uint j=0; j<N; j++)
        {
            const uint rb = RevBits[j];
            c->real = b[rb] * win[rb];
            c->imag = 0;
            c++;
        }
        Result = Perform(Input,Buffer,Twiddle);
    }
    void Inverse()
    {
        QMutexLocker locker(&mutex);
        // Move the rearranged input values to Buffer.
        // Take note of the pointer swaps at the top of the transform algorithm.
        cplx<T>* c = Buffer;
        cplx<T>* r;
        for(j=0; j<N; j++)
        {
            r = &Input[RevBits[j]];
            c->real = r->real;
            c->imag = r->imag;
            c++;
        }
        Result = Perform(Input,Buffer,iTwiddle);
    }
    void Inverse(float* b)
    {
        QMutexLocker locker(&mutex);
        // Move the rearranged input values to Buffer.
        // Take note of the pointer swaps at the top of the transform algorithm.
        cplx<T>* c = Buffer;
        cplx<T>* r;
        for(j=0; j<N; j++)
        {
            r = &Input[RevBits[j]];
            c->real = r->real;
            c->imag = r->imag;
            c++;
        }
        Result = Perform(Input,Buffer,iTwiddle);
        for(j=0; j<N; j++) b[j] += Result[j].real;
    }
    void Inverse(float* b, float* win)
    {
        QMutexLocker locker(&mutex);
        // Move the rearranged input values to Buffer.
        // Take note of the pointer swaps at the top of the transform algorithm.
        cplx<T>* c = Buffer;
        cplx<T>* r;
        for(j=0; j<N; j++)
        {
            r = &Input[RevBits[j]];
            c->real = r->real;
            c->imag = r->imag;
            c++;
        }
        Result = Perform(Input,Buffer,iTwiddle);
        for(j=0; j<N; j++) *b++ += Result[j].real * *win++;
    }
    void Inverse(float* b, float* win, T f)
    {
        QMutexLocker locker(&mutex);
        // Move the rearranged input values to Buffer.
        // Take note of the pointer swaps at the top of the transform algorithm.
        cplx<T>* c = Buffer;
        cplx<T>* r;
        for(j=0; j<N; j++)
        {
            r = &Input[RevBits[j]];
            c->real = r->real;
            c->imag = r->imag;
            c++;
        }
        Result = Perform(Input,Buffer,iTwiddle);
        for(j=0; j<N; j++) *b++ += Result[j].real * *win++ * f;
    }
    void Hermitian()
    {
        QMutexLocker locker(&mutex);
        cplx<T>* p1 = Input + (N >> 1) -1; // input
        cplx<T>* p2 = p1 + 2; // output
        while (p1 >= Input)
        {
            p2->real = p1->real;
            (p2++)->imag = -(p1--)->imag;
        }
    }
    inline T magn(const uint k) const
    {
        const cplx<T>* p=Result+k;
        return T(2.0) * std::sqrt(p->real * p->real + p->imag * p->imag);
    }
    inline T phase(const uint k) const
    {
        return std::atan2(Result[k].imag,Result[k].real);
    }
    inline void polar(const T magn, const T phase, const uint k)
    {
        Input[k].real = magn*std::cos(phase); Input[k].imag = magn*std::sin(phase);
    }
    inline T real(const uint k)
    {
        return Result[k].real;
    }
    inline T imag(const uint k)
    {
        return Result[k].imag;
    }
private:
    QRecursiveMutex mutex;
    uint N, j, LogTwoOfN, *RevBits;
    cplx<T> *Input = nullptr;
    cplx<T> *Buffer = nullptr;
    cplx<T> *Twiddle = nullptr;
    cplx<T> *iTwiddle = nullptr;
    cplx<T> *Result = nullptr;
    /*
     The Pentium takes a surprising amount of time to calculate the sine and cosine.
     You may want to make the twiddle arrays static if doing repeated FFTs of the same size.
     This uses 4 fold symmetry to calculate the twiddle factors. As a result, this function
     requires a minimum FFT size of 8.
    */
    #define M_SQRT_2 0.707106781186547524401
    void FillTwiddleArray(cplx<T> *Twiddle, int N, TTransFormType Type)
    {
        QMutexLocker locker(&mutex);
        int j;
        double Theta, TwoPiOverN;

        TwoPiOverN = 2.0 * M_PI / double(N);

        if(Type == FORWARD)
        {
            Twiddle[0].real = 1.0;
            Twiddle[0].imag = 0.0;
            Twiddle[N/4].real = 0.0;
            Twiddle[N/4].imag = -1.0;
            Twiddle[N/8].real = M_SQRT_2;
            Twiddle[N/8].imag = -M_SQRT_2;
            Twiddle[3*N/8].real = -M_SQRT_2;
            Twiddle[3*N/8].imag = -M_SQRT_2;
            for(j=1; j<N/8; j++)
            {
                Theta = double(j) * -TwoPiOverN;
                Twiddle[j].real = cos(Theta);
                Twiddle[j].imag = sin(Theta);
                Twiddle[N/4-j].real = -Twiddle[j].imag;
                Twiddle[N/4-j].imag = -Twiddle[j].real;
                Twiddle[N/4+j].real = Twiddle[j].imag;
                Twiddle[N/4+j].imag = -Twiddle[j].real;
                Twiddle[N/2-j].real = -Twiddle[j].real;
                Twiddle[N/2-j].imag = Twiddle[j].imag;
            }
        }

        else
        {
            Twiddle[0].real = 1.0;
            Twiddle[0].imag = 0.0;
            Twiddle[N/4].real = 0.0;
            Twiddle[N/4].imag = 1.0;
            Twiddle[N/8].real = M_SQRT_2;
            Twiddle[N/8].imag = M_SQRT_2;
            Twiddle[3*N/8].real = -M_SQRT_2;
            Twiddle[3*N/8].imag = M_SQRT_2;
            for(j=1; j<N/8; j++)
            {
                Theta = double(j) * TwoPiOverN;
                Twiddle[j].real = cos(Theta);
                Twiddle[j].imag = sin(Theta);
                Twiddle[N/4-j].real = Twiddle[j].imag;
                Twiddle[N/4-j].imag = Twiddle[j].real;
                Twiddle[N/4+j].real = -Twiddle[j].imag;
                Twiddle[N/4+j].imag = Twiddle[j].real;
                Twiddle[N/2-j].real = -Twiddle[j].real;
                Twiddle[N/2-j].imag = Twiddle[j].imag;
            }
        }

    }
    // The Fourier Transform.
    cplx<T>* Perform(cplx<T> *Input, cplx<T> *Buffer, cplx<T> *Twiddle)
    {
        uint j, k, J, K;
        cplx<T> *TempPointer;
        cplx<T>* InputI;
        cplx<T>* InputKI;
        cplx<T>* BufferI;
        cplx<T>* BufferKI;
        cplx<T>* twd;
        cplx<T> Temp;

        J = N >> 1;     // J increments down to 1
        K = 1;       // K increments up to N/2

        while(J > 0) // Loops Log2(N) times.
        {
            // Swap pointers, instead doing this: for(j=0; j<N; j++) Input[j] = Buffer[j];
            // We start with a swap because of the swap in ReArrangeInput.
            TempPointer = Input;
            Input = Buffer;
            Buffer = TempPointer;

            InputI = Input;
            BufferI = Buffer;

            for(j=0; j<J; j++)
            {
                InputKI = &InputI[K];
                BufferKI = &BufferI[K];

                Temp.real = InputKI->real; // twd[0] is 1.0 - 0.0
                Temp.imag = InputKI->imag;
                BufferI->real = InputI->real + Temp.real;
                BufferI->imag = InputI->imag + Temp.imag;
                BufferKI->real = InputI->real - Temp.real;
                BufferKI->imag = InputI->imag - Temp.imag;
                InputI++;
                BufferI++;
                InputKI++;
                BufferKI++;

                twd = &Twiddle[J];

                for(k = 1; k < K; k++) // Loops N/2 times for every value of J and K
                {
                    Temp.real = InputKI->real * twd->real - InputKI->imag * twd->imag;
                    Temp.imag = InputKI->real * twd->imag + InputKI->imag * twd->real;
                    BufferI->real = InputI->real + Temp.real;
                    BufferI->imag = InputI->imag + Temp.imag;
                    BufferKI->real = InputI->real - Temp.real;
                    BufferKI->imag = InputI->imag - Temp.imag;
                    InputI++;
                    BufferI++;
                    InputKI++;
                    BufferKI++;
                    twd += J;
                }
                InputI += K;
                BufferI += K;
            }
            K <<= 1;
            J >>= 1;
        }
        // The ReArrangeInput function swapped Input[] and Buffer[]. Then Transform()
        // swapped them again, LogTwoOfN times. Ultimately, these swaps must be done
        // an even number of times, or the pointer to Buffer gets returned.
        // So we must do one more swap here, for N = 16, 64, 256, 1024, ...
        //if(LogTwoOfN % 2 == 1)
        //{
        //    Result = Input;
        //}
        //else // if(LogTwoOfN % 2 == 0) // then the results are still in Buffer.
        //{
            return Buffer;
        //}
    }
};

#endif
