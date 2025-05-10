#ifndef L3HUFF_H
#define L3HUFF_H

#include "bstream.h"

#pragma pack(push,1)

class CHuff
{
public:
    CHuff(){}
    void huffman(int* xy, int n, const uint ntable, CBitget& bitget);
    int huffman_quad(int* vwxy, int n, int nbits, const uint ntable, CBitget& bitget);
private:
    typedef union
    {
        uint ptr;
        struct
        {
            byte signbits;
            byte x;
            byte y;
            byte purgebits; // 0 = esc
        } b;
    }
    HUFF_ELEMENT;
    /* max bits required for any lookup - change if htable changes */
    /* quad required 10 bit w/signs  must have (MAXBITS+2) >= 10   */
#define MAXBITS 9

    typedef struct
    {
        HUFF_ELEMENT* table;
        int linbits;
        int ncase;
    }
    HUFF_SETUP;
    typedef int array2[2];
    typedef int array4[4];
    inline void huffLoop(array2* xy, HUFF_ELEMENT* t, const int i, CBitget& bitget, const int linbits, const bool haveLinBits = false, const bool oneShot = false)
    {
        int code;
        while (true)
        {
            const int bits = t[0].b.signbits;
            bitget.bitget_check((MAXBITS + 2));
            code = bitget.mac_bitget2(bits);
            if ((t[1 + code].b.purgebits) || (oneShot)) break;
            t += t[1 + code].ptr; /* ptr include 1+code */
            bitget.bitget_purge(bits);
        }
        bitget.bitget_purge(t[1 + code].b.purgebits);
        xy[i][0] = t[1 + code].b.x;
        xy[i][1] = t[1 + code].b.y;
        if (haveLinBits)
        {
            for (int j = 0; j < 2; j++)
            {
                if (xy[i][j] == 15)
                {
                    bitget.bitget_check(linbits+2);
                    xy[i][j] += bitget.mac_bitget(linbits);
                }
                if (xy[i][j]) if (bitget.mac_bitget(1)) xy[i][j] = -xy[i][j];
            }
        }
        else
        {
            for (int j = 0; j < 2; j++)
            {
                if (xy[i][j]) if (bitget.mac_bitget(1)) xy[i][j] = -xy[i][j];
            }
        }
    }
    inline void quadLoop(array4* vwxy, const int i, const int tmp, int& i_non_zero, int& tmp_nz, int& nbits, CBitget& bitget)
    {
        if (tmp)
        {
            i_non_zero = i;
            tmp_nz = tmp;
        }
        for (int j = 0; j < 4; j++)
        {
            vwxy[i][j] = (tmp >> (3 - j)) & 1;
            if (vwxy[i][j])
            {
                if (bitget.mac_bitget(1)) vwxy[i][0] = -vwxy[i][0];
                nbits--;
            }
        }
    }
};

#pragma pack(pop)

#endif // L3HUFF_H
