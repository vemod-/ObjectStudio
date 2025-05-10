#ifndef BSTREAM_H
#define BSTREAM_H

#include "softsynthsdefines.h"

class CBitget
{
public:
    CBitget() {}
    CBitget(byte* buf) { bitget_init(buf); }
    void bitget_init(byte *buf)
    {
        bs_ptr_start = bs_ptr = buf;
        bitBufSize = 0;
        bitBuf = 0;
    }
    void bitget_init_end(byte *buf_end) { bs_ptr_end = buf_end; }
    inline bool bitget_overrun() { return (bs_ptr > bs_ptr_end); } /*------------- check overrun -------------*/
    inline int bitget_bits_used() { return ((bs_ptr - bs_ptr_start) << 3) - bitBufSize; } /* compute bits used from last init call */
    inline int bitget(const int n)
    {
        bitget_check(n);
        return mac_bitget(n);
    }
    inline void bitget_skip(int n)
    {
        if (bitBufSize < n)
        {
            n -= bitBufSize;
            const uint k = n >> 3; //--- bytes = n/8 --//
            bs_ptr += k;
            n -= k << 3;
            bitBuf = *bs_ptr++;
            bitBufSize = 8;
        }
        bitget_purge(n);
    }
    inline void bitget_check(const int n) /*------------- check for n bits in bitbuf -------------*/
    {
        if (bitBufSize < n)
        {
            while (bitBufSize <= 24)
            {
                bitBuf = (bitBuf << 8) | *bs_ptr++;
                bitBufSize += 8;
            }
        }
    }
    inline int mac_bitget(const int n)
    {
        bitBufSize -= n;
        const uint code = bitBuf >> bitBufSize;
        bitBuf -= code << bitBufSize;
        return code;
    }
    inline int mac_bitget2(const int n) { return (bitBuf >> (bitBufSize-n)); } //------------- get n bits but DO NOT remove from bitstream --//
    inline void bitget_purge(const int n) //------------- remove n bits from bitstream ---------//
    {
        bitBufSize -= n;
        bitBuf -= (bitBuf >> bitBufSize) << bitBufSize;
    }
private:
    uint bitBuf;
    int bitBufSize;
    byte *bs_ptr;
    byte *bs_ptr_start;
    byte *bs_ptr_end; // optional for overrun test
};

#endif //BSTREAM_H
