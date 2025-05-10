#ifndef FDCT_H
#define FDCT_H

#include "math.h"
#include <string.h>
#include <vector>

class CFdct
{
public:
    CFdct(const int bits)
    {
        m_bits=bits;
        a.resize(bits);
        b.resize(bits);
        /* gen coef for N=32 (31 coefs) */
        int n = 16;
        int k = 0;
        for (int i = 0; i < 5; i++, n = n / 2)
        {
            for (int p = 0; p < n; p++, k++)
            {
                const double t = (M_PI / (4 * n)) * (2 * p + 1);
                coef32[k] = 0.5 / cos(t);
            }
        }
    }
    void setEQ(const int* value)
    {
        //60, 170, 310, 600, 1K, 3K
        for (int i = 0; i < 6; i ++) {
            m_equalizer[i] = pow(10,value[i]/200);
        }
        //6K
        m_equalizer[6] = pow(10,value[6]/200);
        m_equalizer[7] = m_equalizer[6];
        //12K
        m_equalizer[8] = pow(10,value[7]/200);
        m_equalizer[9] = m_equalizer[8];
        m_equalizer[10] = m_equalizer[8];
        m_equalizer[11] = m_equalizer[8];
        //14K
        m_equalizer[12] = pow(10,value[8]/200);
        m_equalizer[13] = m_equalizer[12];
        m_equalizer[14] = m_equalizer[12];
        m_equalizer[15] = m_equalizer[12];
        m_equalizer[16] = m_equalizer[12];
        m_equalizer[17] = m_equalizer[12];
        m_equalizer[18] = m_equalizer[12];
        m_equalizer[19] = m_equalizer[12];
        //16K
        m_equalizer[20] = pow(10,value[9]/200);
        m_equalizer[21] = m_equalizer[20];
        m_equalizer[22] = m_equalizer[20];
        m_equalizer[23] = m_equalizer[20];
        m_equalizer[24] = m_equalizer[20];
        m_equalizer[25] = m_equalizer[20];
        m_equalizer[26] = m_equalizer[20];
        m_equalizer[27] = m_equalizer[20];
        m_equalizer[28] = m_equalizer[20];
        m_equalizer[29] = m_equalizer[20];
        m_equalizer[30] = m_equalizer[20];
        m_equalizer[31] = m_equalizer[20];
    }
    double getEQ(const int band) { return m_equalizer[band]; }
    void setEQEnabled(const bool value) { m_enableEQ = value; }
    bool getEQEnabled() { return m_enableEQ; }
    void fdct(float* x, double* c) { do_fdct(x,c,1); }
    void fdct_dual(float* x, double* c) { do_fdct(x,c,2); }
private:
    void do_fdct(float* x, double* c, const int channels)
    {
        if (m_bits == 32)
        {
            if (m_enableEQ) for (int p = 0; p < 32; p++) x[p] *= m_equalizer[p];
            /* special first stage for dual chan (interleaved x) */
            int pp = 0;
            int qq = channels * 31;
            for (int p = 0; p < 16; p++, pp += channels, qq -= channels)
            {
                a[p] = x[pp] + x[qq];
                a[16 + p] = coef32[p] * (x[pp] - x[qq]);
            }
        }
        else if (m_bits == 16)
        {
            /* special first stage for interleaved input */
            a[0] = x[0];
            a[8] = coef32[16] * x[0];
            int pp = channels;
            int qq = channels * 14;
            for (int p = 1; p < 8; p++, pp += channels, qq -= channels)
            {
                a[p] = x[pp] + x[qq];
                a[8 + p] = coef32[16 + p] * (x[pp] - x[qq]);
            }
        }
        else if (m_bits == 8)
        {
            /* special first stage for interleaved input */
            //b[0] = x[0] + x[7 * channels];
            //b[4] = coef32[16 + 8] * (x[0] - x[7 * channels]);
            int pp = 0;
            int qq = channels * 7;
            for (int p = 0; p < 4; p++, pp += channels, qq -= channels)
            {
                a[p] = x[pp] + x[qq];
                a[4 + p] = coef32[16 + 8 + p] * (x[pp] - x[qq]);
            }
        }
        fdct_2(c);
    }
    double coef32[32]; /* 32 pt dct coefs */
    int m_enableEQ = false;
    double m_equalizer[32] = {0};
    int m_bits;
    std::vector<double> a; /* ping pong buffers */
    std::vector<double> b;
    inline void forward_bf(const int m, const int n, const double* x, double* f, const double* coef)
    {
        int p0 = 0;
        const int n2 = n >> 1;
        for (int i = 0; i < m; i++, p0 += n)
        {
            int k = 0;
            int p = p0;
            int q = p + n - 1;
            for (int j = 0; j < n2; j++, p++, q--, k++)
            {
                f[p] = x[p] + x[q];
                f[n2 + p] = coef[k] * (x[p] - x[q]);
            }
        }
    }
    inline void back_bf(const int m, const int n, const double* x, double* f)
    {
        int p0 = 0;
        const int n2 = n >> 1;
        const int n21 = n2 - 1;
        for (int i = 0; i < m; i++, p0 += n)
        {
            int p = p0;
            int q = p0;
            for (int j = 0; j < n2; j++, p += 2, q++) f[p] = x[q];
            p = p0 + 1;
            for (int j = 0; j < n21; j++, p += 2, q++) f[p] = x[q] + x[q + 1];
            f[p] = x[q];
        }
    }
    void fdct_2(double* c)
    {
        double* ax = a.data();
        double* bx = b.data();
        if (m_bits == 32)
        {
            forward_bf(2, 16, ax, bx, coef32 + 16);
            forward_bf(4, 8, bx, ax, coef32 + 16 + 8);
            forward_bf(8, 4, ax, bx, coef32 + 16 + 8 + 4);
            forward_bf(16, 2, bx, ax, coef32 + 16 + 8 + 4 + 2);
            back_bf(8, 4, ax, bx);
            back_bf(4, 8, bx, ax);
            back_bf(2, 16, ax, bx);
            back_bf(1, 32, bx, c);
        }
        else if (m_bits == 16)
        {
            forward_bf(2, 8, ax, bx, coef32 + 16 + 8);
            forward_bf(4, 4, bx, ax, coef32 + 16 + 8 + 4);
            forward_bf(8, 2, ax, bx, coef32 + 16 + 8 + 4 + 2);
            back_bf(4, 4, bx, ax);
            back_bf(2, 8, ax, bx);
            back_bf(1, 16, bx, c);
        }
        else if (m_bits == 8)
        {
            forward_bf(2, 4, ax, bx, coef32 + 16 + 8 + 4);
            forward_bf(4, 2, bx, ax, coef32 + 16 + 8 + 4 + 2);
            back_bf(2, 4, ax, bx);
            back_bf(1, 8, bx, c);
        }
    }
};


#endif // FDCT_H
