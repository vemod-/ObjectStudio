#ifndef L3HYBRID_H
#define L3HYBRID_H

#include "bstream.h"
#include "imdct.h"
#include <string.h>
#include "l3sf.h"

class CHybrid
{
public:
    CHybrid()
    {
        /* type 0 */
        for (int i = 0; i < 36; i++) win[0][i] = sin(M_PI / 36 * (i + 0.5));

        /* type 1 */
        for (int i = 0; i < 18; i++) win[1][i] = sin(M_PI / 36 * (i + 0.5));
        for (int i = 18; i < 24; i++) win[1][i] = 1.0;
        for (int i = 24; i < 30; i++) win[1][i] = sin(M_PI / 12 * (i + 0.5 - 18));
        for (int i = 30; i < 36; i++) win[1][i] = 0.0;

        /* type 3 */
        for (int i = 0; i < 6; i++) win[3][i] = 0.0;
        for (int i = 6; i < 12; i++) win[3][i] = sin(M_PI / 12 * (i + 0.5 - 6));
        for (int i = 12; i < 18; i++) win[3][i] = 1.0;
        for (int i = 18; i < 36; i++) win[3][i] = sin(M_PI / 36 * (i + 0.5));

        /* type 2 */
        for (int i = 0; i < 12; i++) win[2][i] = sin(M_PI / 12 * (i + 0.5));
        for (int i = 12; i < 36; i++) win[2][i] = 0.0;

        /*--- invert signs by region to match mdct 18pt --> 36pt mapping */
        for (int j = 0; j < 4; j++) if (j != 2) for (int i = 9; i < 36; i++) win[j][i] = -win[j][i];

        /*-- invert signs for short blocks --*/
        for (int i = 3; i < 12; i++) win[2][i] = -win[2][i];
    }
    int hybrid(float *x, float *x0, GR_INFO::blockTypes btype, const int nlong, const int ntot, const int nprev, const int band_limit)
    {
        if (btype == GR_INFO::block_type_2) btype = GR_INFO::block_type_0;
        /*-- do long blocks (if any) --*/
        const int n = bandCountRoundUp(nlong); /* number of dct's to do */
        for (int i = 0; i < n; i++)
        {
            imdct.imdct18(x);
            for (int j = 0; j < 9; j++)
            {
                hybridBuffer[j][i] = x0[j] + win[btype][j] * x[9 + j];
                hybridBuffer[9 + j][i] = x0[9 + j] + win[btype][9 + j] * x[17 - j];
            }
            /* window x for next time x0 */
            for (int j = 0; j < 4; j++)
            {
                const double xa = x[j];
                const double xb = x[8 - j];
                x[j] = win[btype][IMDCTPOINTS_LONG + j] * xb;
                x[8 - j] = win[btype][(IMDCTPOINTS_LONG + 8) - j] * xa;
                x[9 + j] = win[btype][(IMDCTPOINTS_LONG + 9) + j] * xa;
                x[17 - j] = win[btype][(IMDCTPOINTS_LONG + 17) - j] * xb;
            }
            const double xa = x[4];
            x[4] = win[btype][IMDCTPOINTS_LONG + 4] * xa;
            x[9 + 4] = win[btype][(IMDCTPOINTS_LONG + 9) + 4] * xa;

            x += IMDCTPOINTS_LONG;
            x0 += IMDCTPOINTS_LONG;
        }
        /*-- do short blocks (if any) --*/
        const int n1 = bandCountRoundUp(ntot); /* number of 6 pt dct's triples to do */
        for (int i = n; i < n1; i++)
        {
            imdct.imdct6_3(x);
            for (int j = 0; j < 3; j++)
            {
                hybridBuffer[j][i] = x0[j];
                hybridBuffer[3 + j][i] = x0[3 + j];

                hybridBuffer[6 + j][i] = x0[6 + j] + win[2][j] * x[3 + j];
                hybridBuffer[9 + j][i] = x0[9 + j] + win[2][3 + j] * x[5 - j];

                hybridBuffer[12 + j][i] = x0[12 + j] + win[2][6 + j] * x[2 - j] + win[2][j] * x[(6 + 3) + j];
                hybridBuffer[15 + j][i] = x0[15 + j] + win[2][9 + j] * x[j] + win[2][3 + j] * x[(6 + 5) - j];
            }
            /* window x for next time x0 */
            for (int j = 0; j < 3; j++)
            {
                x[j] = win[2][6 + j] * x[(6 + 2) - j] + win[2][j] * x[(12 + 3) + j];
                x[3 + j] = win[2][9 + j] * x[6 + j] + win[2][3 + j] * x[(12 + 5) - j];
            }
            for (int j = 0; j < 3; j++)
            {
                x[6 + j] = win[2][6 + j] * x[(12 + 2) - j];
                x[9 + j] = win[2][9 + j] * x[12 + j];
            }
            for (int j = 0; j < 3; j++)
            {
                x[12 + j] = 0.0f;
                x[15 + j] = 0.0f;
            }
            x += IMDCTPOINTS_LONG;
            x0 += IMDCTPOINTS_LONG;
        }
        /*--- overlap prev if prev longer that current --*/
        const int n2 = bandCountRoundUp(nprev);
        for (int i = n1; i < n2; i++)
        {
            for (int j = 0; j < IMDCTPOINTS_LONG; j++) hybridBuffer[j][i] = x0[j];
            x0 += IMDCTPOINTS_LONG;
        }
        /*--- clear remaining only to band limit --*/
        const int band_limit_nsb = bandCountRoundUp(band_limit); /* limit nsb's rounded up */
        for (int i = n2; i < band_limit_nsb; i++)
        {
            for (int j = 0; j < IMDCTPOINTS_LONG; j++) hybridBuffer[j][i] = 0.0f;
        }
        return IMDCTPOINTS_LONG * band_limit_nsb;
    }
    int hybrid_sum(float *x_r, float *x_l, uint btype, const int nlong, const int ntot)
    {
        /*-- convert to mono, add curr result to y,
        window and add next time to current left */
        if (btype == 2) btype = 0;
        /*-- do long blocks (if any) --*/
        const int n = bandCountRoundUp(nlong); /* number of dct's to do */
        for (int i = 0; i < n; i++)
        {
            imdct.imdct18(x_r);
            for (int j = 0; j < 9; j++)
            {
                hybridBuffer[j][i] += win[btype][j] * x_r[9 + j];
                hybridBuffer[9 + j][i] += win[btype][9 + j] * x_r[17 - j];
            }
            /* window x for next time x0 */
            for (int j = 0; j < 4; j++)
            {
                const double xa = x_r[j];
                const double xb = x_r[8 - j];
                x_l[j] += win[btype][IMDCTPOINTS_LONG + j] * xb;
                x_l[8 - j] += win[btype][(IMDCTPOINTS_LONG + 8) - j] * xa;
                x_l[9 + j] += win[btype][(IMDCTPOINTS_LONG + 9) + j] * xa;
                x_l[17 - j] += win[btype][(IMDCTPOINTS_LONG + 17) - j] * xb;
            }
            const double xa = x_r[4];
            x_l[4] += win[btype][IMDCTPOINTS_LONG + 4] * xa;
            x_l[9 + 4] += win[btype][(IMDCTPOINTS_LONG + 9) + 4] * xa;

            x_r += IMDCTPOINTS_LONG;
            x_l += IMDCTPOINTS_LONG;
        }
        /*-- do short blocks (if any) --*/
        const int n1 = bandCountRoundUp(ntot); /* number of 6 pt dct's triples to do */
        for (int i = n; i < n1; i++)
        {
            imdct.imdct6_3(x_r);
            for (int j = 0; j < 3; j++)
            {
                hybridBuffer[6 + j][i] += win[2][j] * x_r[3 + j];
                hybridBuffer[9 + j][i] += win[2][3 + j] * x_r[5 - j];

                hybridBuffer[12 + j][i] += win[2][6 + j] * x_r[2 - j] + win[2][j] * x_r[(6 + 3) + j];
                hybridBuffer[15 + j][i] += win[2][9 + j] * x_r[j] + win[2][3 + j] * x_r[(6 + 5) - j];
            }
            /* window x for next time */
            for (int j = 0; j < 3; j++)
            {
                x_l[j] += win[2][6 + j] * x_r[(6 + 2) - j] + win[2][j] * x_r[(12 + 3) + j];
                x_l[3 + j] += win[2][9 + j] * x_r[6 + j] + win[2][3 + j] * x_r[(12 + 5) - j];
            }
            for (int j = 0; j < 3; j++)
            {
                x_l[6 + j] += win[2][6 + j] * x_r[(12 + 2) - j];
                x_l[9 + j] += win[2][9 + j] * x_r[12 + j];
            }
            x_r += IMDCTPOINTS_LONG;
            x_l += IMDCTPOINTS_LONG;
        }
        return  IMDCTPOINTS_LONG * n1;
    }
    void sum_f_bands(float *x_l, float *x_r, const int n)
    {
        for (int i = 0; i < n; i++) x_l[i] += x_r[i];
    }
    void freq_invert(int n)
    {
        n = bandCountRoundUp(n);
        for (int j = 0; j < IMDCTPOINTS_LONG; j += 2)
        {
            for (int i = 0; i < n; i += 2)
            {
                hybridBuffer[1 + j][1 + i] = -hybridBuffer[1 + j][1 + i];
            }
        }
    } /* xform, */
    static inline int bandCountRoundUp(const int n) { return (n + (IMDCTPOINTS_LONG - 1)) / IMDCTPOINTS_LONG; }
    float hybridBuffer[IMDCTPOINTS_LONG][ISMAX] = {{0}};  // hybrid out, sbt in
private:
    /*-- windows by block type --*/
    double win[4][36] = {{0}};
    CImdct imdct;
};



#endif // L3HYBRID_H
