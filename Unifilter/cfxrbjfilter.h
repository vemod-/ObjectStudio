#ifndef CFXRBJFILTER_H
#define CFXRBJFILTER_H

#include "softsynthsdefines.h"

class CFxRbjFilter
{
public:
    CFxRbjFilter() {
        // reset filter coeffs
        b0a0=b1a0=b2a0=a1a0=a2a0=0.0;
        // reset in/out history
        ou1=ou2=in1=in2=0.0f;
    }
    float filter(float in0) {
        // filter
        float const yn = b0a0*in0 + b1a0*in1 + b2a0*in2 - a1a0*ou1 - a2a0*ou2;
        // push in/out buffers
        in2=in1;
        in1=in0;
        ou2=ou1;
        ou1=yn;
        // return output
        return yn;
    }
    void calc_filter_coeffs(int const type,double const frequency,double const sample_rate,double const q,double const db_gain,bool q_is_bandwidth) {
        //double const temp_pi=3.1415926535897932384626433832795;
        // temp coef vars
        double alpha,a0 = 1, a1 = 0, a2 = 0, b0 = 0, b1 = 0, b2 = 0;
        // peaking, lowshelf and hishelf
        if(type>=6)
        {
            double const A		=	db_gain;//pow(10.0,(db_gain/40.0));
            double const omega	=	2.0*M_PI*frequency/sample_rate;
            double const tsin	=	sin(omega);
            double const tcos	=	cos(omega);
            if(q_is_bandwidth)
                alpha=tsin*sinh(log(2.0)/2.0*q*omega/tsin);
            else
                alpha=tsin/(2.0*q);
            double const beta	=	sqrt(A)/q;
            // peaking
            if(type==6)
            {
                b0=1.0+alpha*A;
                b1=-2.0*tcos;
                b2=1.0-alpha*A;
                a0=1.0+alpha/A;
                a1=-2.0*tcos;
                a2=1.0-alpha/A;
            }
            // lowshelf
            if(type==7)
            {
                b0=A*((A+1.0)-(A-1.0)*tcos+beta*tsin);
                b1=2.0*A*((A-1.0)-(A+1.0)*tcos);
                b2=A*((A+1.0)-(A-1.0)*tcos-beta*tsin);
                a0=(A+1.0)+(A-1.0)*tcos+beta*tsin;
                a1=-2.0*((A-1.0)+(A+1.0)*tcos);
                a2=(A+1.0)+(A-1.0)*tcos-beta*tsin;
            }
            // hishelf
            if(type==8)
            {
                b0=A*((A+1.0)+(A-1.0)*tcos+beta*tsin);
                b1=-2.0*A*((A-1.0)+(A+1.0)*tcos);
                b2=A*((A+1.0)+(A-1.0)*tcos-beta*tsin);
                a0=(A+1.0)-(A-1.0)*tcos+beta*tsin;
                a1=2.0*((A-1.0)-(A+1.0)*tcos);
                a2=(A+1.0)-(A-1.0)*tcos-beta*tsin;
            }
        }
        else
        {
            // other filters
            double const omega	=	2.0*M_PI*frequency/sample_rate;
            double const tsin	=	sin(omega);
            double const tcos	=	cos(omega);
            if(q_is_bandwidth)
                alpha=tsin*sinh(log(2.0)/2.0*q*omega/tsin);
            else
                alpha=tsin/(2.0*q);
            // lowpass
            if(type==0)
            {
                b0=(1.0-tcos)/2.0;
                b1=1.0-tcos;
                b2=(1.0-tcos)/2.0;
                a0=1.0+alpha;
                a1=-2.0*tcos;
                a2=1.0-alpha;
            }
            // hipass
            if(type==1)
            {
                b0=(1.0+tcos)/2.0;
                b1=-(1.0+tcos);
                b2=(1.0+tcos)/2.0;
                a0=1.0+ alpha;
                a1=-2.0*tcos;
                a2=1.0-alpha;
            }
            // bandpass csg
            if(type==2)
            {
                b0=tsin/2.0;
                b1=0.0;
                b2=-tsin/2;
                a0=1.0+alpha;
                a1=-2.0*tcos;
                a2=1.0-alpha;
            }
            // bandpass czpg
            if(type==3)
            {
                b0=alpha;
                b1=0.0;
                b2=-alpha;
                a0=1.0+alpha;
                a1=-2.0*tcos;
                a2=1.0-alpha;
            }
            // notch
            if(type==4)
            {
                b0=1.0;
                b1=-2.0*tcos;
                b2=1.0;
                a0=1.0+alpha;
                a1=-2.0*tcos;
                a2=1.0-alpha;
            }

            // allpass
            if(type==5)
            {
                b0=1.0-alpha;
                b1=-2.0*tcos;
                b2=1.0+alpha;
                a0=1.0+alpha;
                a1=-2.0*tcos;
                a2=1.0-alpha;
            }
        }
        // set filter coeffs
        b0a0=b0/a0;
        b1a0=b1/a0;
        b2a0=b2/a0;
        a1a0=a1/a0;
        a2a0=a2/a0;
    }
private:
    // filter coeffs
    float b0a0,b1a0,b2a0,a1a0,a2a0;
    // in/out history
    float ou1,ou2,in1,in2;
};

#endif // CFXRBJFILTER_H
