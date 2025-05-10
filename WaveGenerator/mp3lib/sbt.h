#ifndef SBT_H
#define SBT_H

#include "fdct.h"
#include "cwindowing.h"
#include "array"
class CSbt
{
public:
    CSbt(const int layer, const int reduction, const int convert, const int channels)
    {
        m_layer = layer;
        m_bits = 32 >> reduction;
        m_bytes = (convert & 8) ? 1 : 2;
        m_channels = channels;
        m_bitmask = ((m_bits*16)-1);
        window = new CWindowing(m_bits,m_bytes,m_channels,m_bitmask);
        fdct = new CFdct(m_bits);
    }
    ~CSbt()
    {
        delete fdct;
        delete window;
    }
    void setEQ(const int* value) { fdct->setEQ(value); }
    double getEQ(const int band) { return fdct->getEQ(band); }
    void setEQEnabled(const bool value) { fdct->setEQEnabled(value); }
    bool getEQEnabled() { return fdct->getEQEnabled(); }
    void sbt(float* sample, void* pcm, const int n, const int ch = 0)
    {
        (m_channels == 2) ? sbt_dual(sample,pcm,n,ch) : sbt_mono(sample,pcm,n);
    }
private:
    CFdct* fdct;
    CWindowing* window;
    int vb_ptr = 0;
    int vb2_ptr = 0;
    int m_layer;
    int m_bits;
    int m_bytes;
    int m_channels;
    int m_bitmask;
    double vbuf[512] = {0};
    double vbuf2[512] = {0};
    inline void* pcmAdd(void* pcm, const int n) { return static_cast<byte*>(pcm) + (n * m_bytes); }
    inline void sbt_mono(float* sample, void* pcm, const int n)
    {
        const int inc = (m_layer == 3) ? 32 : 64;
        for (int i = 0; i < n; i++)
        {
            fdct->fdct(sample,vbuf +vb_ptr);
            window->doWin(vbuf ,vb_ptr,pcm);
            sample += inc;
            vb_ptr = (vb_ptr - m_bits) & m_bitmask;
            pcm = pcmAdd(pcm, m_bits);
        }
    }
    inline void sbt_dual(float* sample, void* pcm, const int n, const int ch)
    {
        if (m_layer == 3)
        {
            if (ch == 0)
            {
                for (int i = 0; i < n; i++)
                {
                    fdct->fdct(sample,vbuf +vb_ptr);
                    window->doWin(vbuf ,vb_ptr,pcm);
                    sample += 32;
                    vb_ptr = (vb_ptr - m_bits) & m_bitmask;
                    pcm = pcmAdd(pcm,m_bits*2);
                }
            }
            else
            {
                for (int i = 0; i < n; i++)
                {
                    fdct->fdct(sample,vbuf2 +vb2_ptr);
                    window->doWin(vbuf2 ,vb2_ptr,pcmAdd(pcm,1));
                    sample += 32;
                    vb2_ptr = (vb2_ptr - m_bits) & m_bitmask;
                    pcm = pcmAdd(pcm,m_bits*2);
                }
            }
        }
        else
        {
            for (int i = 0; i < n; i++)
            {
                fdct->fdct_dual(sample,vbuf +vb_ptr);
                fdct->fdct_dual(sample+1,vbuf2 +vb_ptr);
                window->doWin(vbuf ,vb_ptr,pcm);
                window->doWin(vbuf2 ,vb_ptr,pcmAdd(pcm,1));
                sample += 64;
                vb_ptr = (vb_ptr - m_bits) & m_bitmask;
                pcm = pcmAdd(pcm,m_bits*2);
            }
        }
    }
};

#endif // SBT_H
