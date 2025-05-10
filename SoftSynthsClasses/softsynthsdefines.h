#ifndef SOFTSYNTHSDEFINES_H
#define SOFTSYNTHSDEFINES_H

#include <qmath.h>
#include <limits.h>
#include <QString>
#include <QRandomGenerator>

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef long long long64;
typedef unsigned long long ulong64;
typedef long double ldouble;

#define MAXCHARMULTIPLY 0.00787401574//1.0/MAXCHAR

#define MAXSHORTMULTIPLY 0.0000305185094759972//MAXSHORTMULTIPLY 1.0/MAXSHORT

#define MAXINTMULTIPLY 4.6566129e-10

#define MAXCHARMULTIPLY_F 0.00787401574f//1.0/MAXCHAR

#define MAXSHORTMULTIPLY_F 0.0000305185094759972f//MAXSHORTMULTIPLY 1.0/MAXSHORT

#define MAXINTMULTIPLY_F 4.6566129e-10f

#define MAXLONGMULTIPLY_F 1.0842022e-19f

#define FreqResolution 100000

#define FreqResolutionMultiply 0.00001//1/FreqResolution

#define M_LOG2E_F 1.44269504089f

#define M_LOG10E_F 0.4342944819f

#define M_SQRT1_2_F 0.70710678118655f

#define pitchBendFactor 0.0244140625//(1.0 / (0x2000/200.0))

#define EPSILON_FLOAT 1.0e-5f
#define EPSILON_DOUBLE 1.0e-9

#define PI_F 3.14159265358979f

#define setFontSizeScr(w,s) {QFont f(w->font()); f.setPointSizeF(s); w->setFont(f);}

template <typename T>

bool inline closeEnough(const T x, const T y)
{
    return (x == y) ? true : qAbs<T>(x - y) <= T(EPSILON_DOUBLE);
}

template <typename T>

bool inline isZero(const T x)
{
    return (x == 0) ? true :  qAbs<T>(x) <= T(EPSILON_DOUBLE);
}

template <typename T>

bool inline isOne(const T x)
{
    return (x == 1) ? true : qAbs<T>(x - 1) <= T(EPSILON_DOUBLE);
}


#define voltage2Factor(voltage) pow(2.0, voltage)

#define voltage2Factorf(voltage) powf(2.f, voltage)

#define factor2voltage(factor) log2(factor)

#define factor2voltagef(factor) log2f(factor)

double inline cent2Freq(const long cent ,const double A440=440.0)
{
    return (cent==0) ? 0 : A440 * voltage2Factor(((cent * 0.01) - 69.0) / 12.0);
}

float inline cent2Freqf(const long cent ,const float A440=440.f)
{
    return (cent==0) ? 0 : A440 * voltage2Factorf(((cent * 0.01f) - 69.f) / 12.f);
}

long inline freq2Cent(const double freq, const double A440=440.0)
{
    return (isZero(freq)) ? 0 : long((((12 * factor2voltage(freq / A440)) + 69) * 100.0) + 0.5);
}

long inline freq2Centf(const float freq, const float A440=440.f)
{
    return (isZero(freq)) ? 0 : long((((12 * factor2voltagef(freq / A440)) + 69) * 100.f) + 0.5f);
}

double inline voltage2Freq(const double voltage ,const double A440=440.0)
{
    return cent2Freq(long(voltage * 1200.0), A440);
}

float inline voltage2Freqf(const float voltage ,const float A440=440.f)
{
    return cent2Freqf(long(voltage * 1200.f), A440);
}

double inline freq2voltage(const double freq, const double A440=440.0)
{
    return freq2Cent(freq, A440) / 1200.0;
}

float inline freq2voltagef(const float freq, const float A440=440.f)
{
    return freq2Centf(freq, A440) / 1200.f;
}

double inline MIDIkey2Freq(const int keynum, const double A440=440.0, const int cents=0)
{
    return cent2Freq((keynum * 100)+cents, A440);
}

float inline MIDIkey2Freqf(const int keynum, const float A440=440.f, const int cents=0)
{
    return cent2Freqf((keynum * 100)+cents, A440);
}

int inline freq2MIDIkey(const double freq, const double A440=440.0)
{
    return int((freq2Cent(freq, A440) * 0.01) + 0.5);
}

int inline freq2MIDIkeyf(const float freq, const float A440=440.f)
{
    return int((freq2Centf(freq, A440) * 0.01f) + 0.5f);
}

double inline cent2Factor(const int Cent)
{
    return (Cent==0) ? 1 : voltage2Factor(Cent / 1200.0);
}

float inline cent2Factorf(const int Cent)
{
    return (Cent==0) ? 1 : voltage2Factorf(Cent / 1200.f);
}

int inline factor2Cent(const double factor)
{
    return int((factor2voltage(factor) * 1200.0) + 0.5);
    //return int((1200/log10(2))*log(factor));
}

int inline factor2Centf(const float factor)
{
    return int((factor2voltagef(factor) * 1200.f) + 0.5f);
    //return int((1200/log10(2))*log(factor));
}

double inline tune2voltage(double A440)
{
    return factor2voltage(A440/440.0);
}

float inline tune2voltagef(float A440)
{
    return factor2voltagef(A440/440.f);
}

double inline tune2Cent(double A440)
{
    return factor2Cent(A440/440.0);
}

float inline tune2Centf(float A440)
{
    return factor2Centf(A440/440.f);
}

double inline MIDIkey2voltage(const int keynum, const double A440=440.0, const int cents=0)
{
    return ((keynum*100) + cents + tune2Cent(A440)) / 1200.0;
}

float inline MIDIkey2voltagef(const int keynum, const float A440=440.0, const int cents=0)
{
    return ((keynum*100) + cents + tune2Centf(A440)) / 1200.f;
}


void inline zeroMemory(void* mem, const ulong64 size)
{
    memset(mem,0,size);
}

void inline copyMemory(void* dest, const void* source, const ulong64 size)
{
    memcpy(dest,source,size);
}

inline ushort from14bit(const byte lsb, const byte msb)
{
   return (ushort(msb) << 7) | lsb ;
}

inline byte to14bitMSB(const ushort i)
{
    return (i >> 7) & 0x7F;
}

inline byte to14bitLSB(const ushort i)
{
    return i & 0x7F;
}


/*
 * convert timecents to msec
 */
int inline timecent2msec(const long timecent)
{
    return int(1000.0 * voltage2Factor(timecent / 1200.0));
}
/*
 * convert msec to timecents
 */
long inline msec2timecent(double msec)
{
    return long(factor2voltage(qMax<double>(msec,1) / 1000.0) * 1200.0);
}

double inline cB2Percent(const double cB)
{
    if (cB>=960) return 0;
    if (isZero(cB)) return 1;
    return pow(10.0,cB/-200.0);
}

float inline cB2Percentf(const float cB)
{
    if (cB>=960) return 0;
    if (isZero(cB)) return 1;
    return powf(10.f,cB/-200.f);
}

double inline lin2exp(const double lin)
{
    return cB2Percent((1.0-lin)*960.0);
}

float inline lin2expf(const float lin)
{
    return cB2Percentf((1.f-lin)*960.f);
}

double inline filterQadip(const int gain_cB)
{
    return qBound<double>(0, gain_cB / 15.0, 15) / 15.0;
}

bool inline descriptorMatch(const void *descriptor, const void *s, const ulong64 l=4)
{
    return (memcmp(descriptor,s,l)==0);
}

void inline setDescriptor(void* descriptor, const void* s)
{
    copyMemory(descriptor,s,4);
}

double inline dB2lin(const double x)
{
    return (x > -96.0) ? pow(10.0, x * 0.05) : 0;
}

float inline dB2linf(const float x)
{
    return (x > -96.f) ? powf(10.f, x * 0.05f) : 0;
}

double inline lin2dB(const double x)
{
    return  (x > 0) ? 20.0*(log10(x)) : -96;
}

float inline lin2dBf(const float x)
{
    return (x > 0) ? 20.f*(log10f(x)) : -96;
}

QString inline percent2dBText(const int percent)
{
    return QString::number(lin2dB(percent*0.01),'f',2)+" dB";
}

QString inline mSecsToText(const ulong64 mSecs)
{
    int minutes = mSecs/60000;
    int seconds = int(mSecs/1000)%60;
    int milliseconds = mSecs%1000;
    return QString::number(minutes) + ":" + QString::number(seconds) + "." + QString::number(milliseconds);
}

double inline mixFactor(const int connectCount)
{
    if (connectCount<2) return 1;
    if (connectCount==2) return M_SQRT1_2;
    return 1.0/sqrt(connectCount);
}

float inline mixFactorf(const int connectCount)
{
    if (connectCount<2) return 1;
    if (connectCount==2) return M_SQRT1_2_F;
    return 1.f/sqrtf(connectCount);
}

double inline samplesPerTick(const double uSPQ, const double Ticks, const double uSPerSample)
{
    return (uSPQ/Ticks) / uSPerSample;
}

double inline humanizeFactor(int amount) {
    if (amount) return 1 + ((QRandomGenerator::global()->generateDouble() - 0.5) * amount * 0.01);
    return 1;
}

#define FORMFUNC(FormClassName) dynamic_cast<FormClassName*>(m_Form)

#define DEVICEFUNC(DeviceClassName) dynamic_cast<DeviceClassName*>(m_Device)

#endif // SOFTSYNTHSDEFINES_H
