#ifndef L3ALIAS_H
#define L3ALIAS_H

#include "math.h"

class CAlias
{
public:
    CAlias()
    {
        static const double Ci[8] = { -0.6, -0.535, -0.33, -0.185, -0.095, -0.041, -0.0142, -0.0037 };
        for (int i = 0; i < 8; i++)
        {
            csa[i][0] = 1.0 / sqrt(1.0 + Ci[i] * Ci[i]);
            csa[i][1] = Ci[i] / sqrt(1.0 + Ci[i] * Ci[i]);
        }
    }
    void antialias(float *x, const int n)
    {
        for (int k = 0; k < n; k++)
        {
            for (int i = 0; i < 8; i++)
            {
                const double a = x[17 - i];
                const double b = x[18 + i];
                x[17 - i] = a * csa[i][0] - b * csa[i][1];
                x[18 + i] = b * csa[i][0] + a * csa[i][1];
            }
            x += 18;
        }
    }
private:
    double csa[8][2];  /* antialias */
};

#endif // L3ALIAS_H
