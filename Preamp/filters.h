#ifndef FILTERS_H
#define FILTERS_H

#include <math.h>

/*
 * Presence and Shelve filters as given in
 *   James A. Moorer
 *   The manifold joys of conformal mapping:
 *   applications to digital filtering in the studio
 *   JAES, Vol. 31, No. 11, 1983 November
 */
//#define MINDOUBLE	4.94065645841246544e-324
//#define SPN MINDOUBLE
#define M_PI_F 3.14159265358979f

#define MINFLOAT 1.17549e-38f
#define SPN MINFLOAT

class filter
{
public:
    filter()
    {
        x1 = 0.f;
        x2 = 0.f;
        y1 = 0.f;
        y2 = 0.f;
        y = 0.f;
    }
    void setfilter_presence(float freq, float boost, float bw, float SR)
    {
        presence(freq/SR,boost,bw/SR, &cx,&cx1,&cx2,&cy1,&cy2);
        cy1 = -cy1;
        cy2 = -cy2;
    }

    void setfilter_shelve(float freq, float boost, float SR)
    {
        shelve(freq/SR,boost,&cx,&cx1,&cx2,&cy1,&cy2);
        cy1 = -cy1;
        cy2 = -cy2;
    }

    void setfilter_shelvelowpass(float freq, float boost, float SR)
    {
        float gain;

        gain = powf(10.f,boost/20.f);
        shelve(freq/SR,boost,&cx,&cx1,&cx2,&cy1,&cy2);
        cx /= gain;
        cx1 /= gain;
        cx2 /= gain;
        cy1 = -cy1;
        cy2 = -cy2;
    }

    /*
     * As in ''An introduction to digital filter theory'' by Julius O. Smith
     * and in Moore's book; I use the normalized version in Moore's book.
     */
    void setfilter_2polebp(float freq, float R, float SR)
    {
        float theta;

        theta = 2.f*M_PI_F*freq/SR;
        cx = 1.f-R;
        cx1 = 0.f;
        cx2 = -(1.f-R)*R;
        cy1 = 2.f*R*cosf(theta);
        cy2 = -R*R;
    }

    /*
     * As in
     *   Stanley A. White
     *   Design of a digital biquadratic peaking or notch filter
     *   for digital audio equalization
     *   JAES, Vol. 34, No. 6, 1986 June
     */
    void setfilter_peaknotch(float freq, float M, float bw, float SR)
    {
        float w0,p,om,ta,d;
        p = 0;

        w0 = 2.f*M_PI_F*freq;
        if ((1.f/sqrtf(2.f) < M) && (M < sqrtf(2.f))) {
            //fprintf(stderr,"peaknotch filter: 1/sqrtf(2) < M < sqrtf(2)\n");
            //exit(-1);
            return;
        }
        if (M <= 1.f/sqrtf(2.f)) p = sqrtf(1.f-2.f*M*M);
        if (sqrtf(2.f) <= M) p = sqrtf(M*M-2.f);
        om = 2.f*M_PI_F*bw;
        ta = tanf(om/(SR*2.f));
        d = p+ta;
        cx = (p+M*ta)/d;
        cx1 = -2.f*p*cosf(w0/SR)/d;
        cx2 = (p-M*ta)/d;
        cy1 = 2.f*p*cosf(w0/SR)/d;
        cy2 = -(p-ta)/d;
    }

    /*
     * Some JAES's article on ladder filter.
     * freq (Hz), gdb (dB), bw (Hz)
     */
    void setfilter_peaknotch2(float freq, float gdb, float bw, float SR)
    {
        float k,w,bwr,abw,gain;

        k = powf(10.f,gdb/20.f);
        w = 2.f*M_PI_F*freq/SR;
        bwr = 2.f*M_PI_F*bw/SR;
        abw = (1.f-tanf(bwr/2.f))/(1.f+tanf(bwr/2.f));
        gain = 0.5f*(1.f+k+abw-k*abw);
        cx = 1.f*gain;
        cx1 = gain*(-2.f*cosf(w)*(1.f+abw))/(1.f+k+abw-k*abw);
        cx2 = gain*(abw+k*abw+1.f-k)/(abw-k*abw+1.f+k);
        cy1 = 2.f*cosf(w)/(1.f+tanf(bwr/2.f));
        cy2 = -abw;
    }

    float applyfilter(float input)
    {
        x = input;
        y = cx * x + cx1 * x1 + cx2 * x2 + cy1 * y1 + cy2 * y2;
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;
        return y;
    }

private:
    float x;
    float x1;
    float x2;
    float y;
    float y1;
    float y2;
    float cx;
    float cx1;
    float cx2;
    //float cy;
    float cy1;
    float cy2;
    float bw2angle(float a, float bw)
    {
        float T,d,sn,cs,mag,delta,theta,tmp,a2,a4,asnd;

        T = tanf(2.f*M_PI_F*bw);
        a2 = a*a;
        a4 = a2*a2;
        d = 2.f*a2*T;
        sn = (1.f + a4)*T;
        cs = (1.f - a4);
        mag = sqrtf(sn*sn + cs*cs);
        d /= mag;
        delta = atan2f(sn,cs);
        asnd = asinf(d);
        theta = 0.5f*(M_PI_F - asnd - delta);
        tmp = 0.5f*(asnd-delta);
        if ((tmp > 0.f) && (tmp < theta)) theta = tmp;
        return(theta/(2.f*M_PI_F));
    }

    void presence(float cf,float boost,float bw,float* a0,float* a1,float* a2,float* b1,float* b2)
    {
        float a,A,F,xfmbw,C,tmp,alphan,alphad,b0,recipb0,asq,F2,a2plus1,ma2plus1;

        a = tanf(M_PI_F*(cf-0.25f));
        asq = a*a;
        A = powf(10.f,boost/20.f);
        if ((boost < 6.f) && (boost > -6.f)) F = sqrtf(A);
        else if (A > 1.f) F = A/sqrtf(2.f);
        else F = A*sqrtf(2.f);
        xfmbw = bw2angle(a,bw);

        C = 1.f/tanf(2.f*M_PI_F*xfmbw);
        F2 = F*F;
        tmp = A*A - F2;
        if (fabsf(tmp) <= SPN) alphad = C;
        else alphad = sqrtf(C*C*(F2-1.f)/tmp);
        alphan = A*alphad;

        a2plus1 = 1.f + asq;
        ma2plus1 = 1.f - asq;
        *a0 = a2plus1 + alphan*ma2plus1;
        *a1 = 4.f*a;
        *a2 = a2plus1 - alphan*ma2plus1;

        b0 = a2plus1 + alphad*ma2plus1;
        *b2 = a2plus1 - alphad*ma2plus1;

        recipb0 = 1.f/b0;
        *a0 *= recipb0;
        *a1 *= recipb0;
        *a2 *= recipb0;
        *b1 = *a1;
        *b2 *= recipb0;
    }

    void shelve(float cf, float boost, float* a0, float* a1, float* a2, float* b1, float* b2)
    {
        float a,A,F,tmp,b0,recipb0,asq,F2,gamma2,siggam2,gam2p1;
        float gamman,gammad,ta0,ta1,ta2,tb0,tb1,tb2,aa1,ab1;

        a = tanf(M_PI_F*(cf-0.25f));
        asq = a*a;
        A = powf(10.f,boost/20.f);
        if ((boost < 6.f) && (boost > -6.f)) F = sqrtf(A);
        else if (A > 1.f) F = A/sqrtf(2.f);
        else F = A*sqrtf(2.f);

        F2 = F*F;
        tmp = A*A - F2;
        if (fabsf(tmp) <= SPN) gammad = 1.f;
        else gammad = powf((F2-1.f)/tmp,0.25f);
        gamman = sqrtf(A)*gammad;

        gamma2 = gamman*gamman;
        gam2p1 = 1.f + gamma2;
        siggam2 = 2.f*sqrtf(2.f)/2.f*gamman;
        ta0 = gam2p1 + siggam2;
        ta1 = -2.f*(1.f - gamma2);
        ta2 = gam2p1 - siggam2;

        gamma2 = gammad*gammad;
        gam2p1 = 1.f + gamma2;
        siggam2 = 2.f*sqrtf(2.f)/2.f*gammad;
        tb0 = gam2p1 + siggam2;
        tb1 = -2.f*(1.f - gamma2);
        tb2 = gam2p1 - siggam2;

        aa1 = a*ta1;
        *a0 = ta0 + aa1 + asq*ta2;
        *a1 = 2.f*a*(ta0+ta2)+(1.f+asq)*ta1;
        *a2 = asq*ta0 + aa1 + ta2;

        ab1 = a*tb1;
        b0 = tb0 + ab1 + asq*tb2;
        *b1 = 2.f*a*(tb0+tb2)+(1.f+asq)*tb1;
        *b2 = asq*tb0 + ab1 + tb2;

        recipb0 = 1.f/b0;
        *a0 *= recipb0;
        *a1 *= recipb0;
        *a2 *= recipb0;
        *b1 *= recipb0;
        *b2 *= recipb0;
    }
};

#endif // FILTERS_H
