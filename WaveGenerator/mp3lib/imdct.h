#ifndef IMDCT_H
#define IMDCT_H

#include "math.h"

#define IMDCTPOINTS_LONG 18
#define IMDCTPOINTS_SHORT 6

class CImdct
{
public:
    CImdct()
    {
        //--- 18 point //
        fillXForm(w,w2,IMDCTPOINTS_LONG);
        calcCoef(&coef[0][0],9,4);
        //--- 6 point //
        fillXForm(v,v2,IMDCTPOINTS_SHORT);
        calcCoef(&coef87,1,1);
        /* adjust scaling to save a few mults */
        for (int p = 0; p < IMDCTPOINTS_SHORT; p++) v[p] *= 0.5;
        coef87 = 2.0 * coef87;
    }
    void imdct18(float f[]) // 18 point //
    {
        double a[9], b[9];
        double ap[2], bp[2];
        for (int p = 0; p < 4; p++)
        {
            for (int i = 0; i < 2; i++)
            {
                const int p8 = (i == 0) ? p : 8 - p;
                const int p9 = (i == 0) ? 17 - p : 9 + p;
                const double g1 = w[p8] * f[p8];
                const double g2 = w[p9] * f[p9];
                ap[i] = g1 + g2;  // a[p]
                bp[i] = w2[p8] * (g1 - g2); // b[p]
            }
            a[p] = ap[0] + ap[1];
            a[5 + p] = ap[0] - ap[1];
            b[p] = bp[0] + bp[1];
            b[5 + p] = bp[0] - bp[1];
        }
        const double g1 = w[4] * f[4];
        const double g2 = w[17 - 4] * f[17 - 4];
        a[4] = g1 + g2;
        b[4] = w2[4] * (g1 - g2);

        f[0] = 0.5 * (a[0] + a[1] + a[2] + a[3] + a[4]);
        f[1] = 0.5 * (b[0] + b[1] + b[2] + b[3] + b[4]);

        f[2] = coef[1][0] * a[5] + coef[1][1] * a[6] + coef[1][2] * a[7] + coef[1][3] * a[8];
        f[3] = coef[1][0] * b[5] + coef[1][1] * b[6] + coef[1][2] * b[7] + coef[1][3] * b[8] - f[1];
        f[1] -= f[0];
        f[2] -= f[1];

        f[4] = coef[2][0] * a[0] + coef[2][1] * a[1] + coef[2][2] * a[2] + coef[2][3] * a[3] - a[4];
        f[5] = coef[2][0] * b[0] + coef[2][1] * b[1] + coef[2][2] * b[2] + coef[2][3] * b[3] - b[4] - f[3];
        f[3] -= f[2];
        f[4] -= f[3];

        f[6] = coef[3][0] * (a[5] - a[7] - a[8]);
        f[7] = coef[3][0] * (b[5] - b[7] - b[8]) - f[5];
        f[5] -= f[4];
        f[6] -= f[5];

        f[8] = coef[4][0] * a[0] + coef[4][1] * a[1] + coef[4][2] * a[2] + coef[4][3] * a[3] + a[4];
        f[9] = coef[4][0] * b[0] + coef[4][1] * b[1] + coef[4][2] * b[2]  + coef[4][3] * b[3] + b[4] - f[7];
        f[7] -= f[6];
        f[8] -= f[7];

        f[10] = coef[5][0] * a[5] + coef[5][1] * a[6] + coef[5][2] * a[7] + coef[5][3] * a[8];
        f[11] = coef[5][0] * b[5] + coef[5][1] * b[6] + coef[5][2] * b[7] + coef[5][3] * b[8] - f[9];
        f[9] -= f[8];
        f[10] -= f[9];

        f[12] = 0.5 * (a[0] + a[2] + a[3]) - a[1] - a[4];
        f[13] = 0.5 * (b[0] + b[2] + b[3]) - b[1] - b[4] - f[11];
        f[11] -= f[10];
        f[12] -= f[11];

        f[14] = coef[7][0] * a[5] + coef[7][1] * a[6] + coef[7][2] * a[7] + coef[7][3] * a[8];
        f[15] = coef[7][0] * b[5] + coef[7][1] * b[6] + coef[7][2] * b[7] + coef[7][3] * b[8] - f[13];
        f[13] -= f[12];
        f[14] -= f[13];

        f[16] = coef[8][0] * a[0] + coef[8][1] * a[1] + coef[8][2] * a[2] + coef[8][3] * a[3] + a[4];
        f[17] = coef[8][0] * b[0] + coef[8][1] * b[1] + coef[8][2] * b[2] + coef[8][3] * b[3] + b[4] - f[15];
        f[15] -= f[14];
        f[16] -= f[15];
        f[17] -= f[16];
    }
    void imdct6_3(float f[]) // 6 point //
    {
        /* does 3, 6 pt dct.  changes order from f[i][window] c[window][i] */
        double buf[IMDCTPOINTS_LONG];

        float* c = f;
        double* a = buf;
        for (int w = 0; w < 3; w++, a += 6, f++)
        {
            for (int i = 0; i < 3; i++)
            {
                const double g1 = v[i] * f[3 * i];
                const double g2 = v[5-i] * f[3 * (5-i)];
                a[i] = g1 + g2;
                a[3 + i] = v2[i] * (g1 - g2);
            }
        }

        a = buf;
        for (int w = 0; w < 3; w++, a += 6, c += 6)
        {
            const double a02 = (a[0] + a[2]);
            const double b02 = (a[3 + 0] + a[3 + 2]);
            c[0] = a02 + a[1];
            c[1] = b02 + a[3 + 1];
            c[2] = coef87 * (a[0] - a[2]);
            c[3] = coef87 * (a[3 + 0] - a[3 + 2]) - c[1];
            c[1] -= c[0];
            c[2] -= c[1];
            c[4] = a02 - a[1] - a[1];
            c[5] = b02 - a[3 + 1] - a[3 + 1] - c[3];
            c[3] -= c[2];
            c[4] -= c[3];
            c[5] -= c[4];
        }
    }
private:
    /*------ 18 point xform -------*/
    double w[IMDCTPOINTS_LONG];
    double w2[IMDCTPOINTS_LONG / 2];
    double coef[9][4];
    /*------ 6 point xform -------*/
    double v[IMDCTPOINTS_SHORT];
    double v2[IMDCTPOINTS_SHORT / 2];
    double coef87;
    void fillXForm(double* w,double* w2,const int n)
    {
        const double t = M_PI_4 / n;
        for (int p = 0; p < n; p++) w[p] = 2.0 * cos(t * (2 * p + 1));
        for (int p = 0; p < n/2; p++) w2[p] = 2.0 *cos(2 * t * (2 * p + 1));
    }
    void calcCoef(double* coef, const int n1, const int n2)
    {
        const double t = M_PI_2 / 18;
        for (int k = 0; k < n1; k++)
        {
            for (int p = 0; p < n2; p++) coef[(n2 * k) + p] = cos(t * (2 * k) * (2 * p + 1));
        }
    }
};

#endif // IMDCT_H
