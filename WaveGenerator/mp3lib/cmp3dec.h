#ifndef CMP3DEC_H
#define CMP3DEC_H

#include "clayer1.h"
#include "clayer2.h"
#include "clayer3.h"

#pragma pack(push,1)

class CMP3dec
{
public:
    CMP3dec(byte* mpeg);
    ~CMP3dec();
    int mp3SetDecodeOption(MPEG_DECODE_OPTION* option);
    void mp3GetDecodeOption(MPEG_DECODE_OPTION* option);
    int mp3SetEqualizer(int* value);
    int mp3GetDecodeInfo(byte* mpeg)
    {
        return currentLayer->mp3GetDecodeInfo(mpeg);
    }
    int mp3DecodeFrame(byte* inbuf,short* outbuf)
    {
        return  currentLayer->mp3DecodeFrame(inbuf,outbuf);
    }
    void mp3MuteStart();
    void mp3MuteEnd();
    bool isInitialised() { return (currentLayer != nullptr); }
    uint mp3Frequency()
    {
        return uint(currentLayer->CurrentFrameInfo.frequency);
    }
    uint mp3Channels()
    {
        return uint(currentLayer->CurrentFrameInfo.channels);
    }
    static byte* findAudioFrame(byte* mpeg, const ulong64 sz)
    {
        const byte LameDescriptor[4]={0xFF, 0xFB, 0x90, 0x64};
        byte* pFrameData=mpeg;
        ulong64 FileSize=sz;
        if(descriptorMatch(&mpeg[0],"RIFF"))
        {
            if(descriptorMatch(&mpeg[8],"WAVEfmt",7))
            {
                if(descriptorMatch(&mpeg[50],"fact")) pFrameData = mpeg + 70;
                if(descriptorMatch(&mpeg[52],"fact")) pFrameData = mpeg + 72;
                if(descriptorMatch(&mpeg[50],"data")) pFrameData = mpeg + 58;
            }
            if(descriptorMatch(&mpeg[8],"RMP3"))
            {
                if(descriptorMatch(&mpeg[12],"data")) pFrameData = mpeg + 16;
            }
        }
        else if (descriptorMatch(&mpeg[0],"TAG+"))
        {
            pFrameData=mpeg+227;
        }
        else if (descriptorMatch(&mpeg[0],"TAG",3))
        {
            pFrameData=mpeg+128;
        }
        else if (descriptorMatch(&mpeg[0],"ID3",3))
        {
            ID3Header* ID3=reinterpret_cast<ID3Header*>(mpeg);
            pFrameData=mpeg + syncSafe(ID3->size) + sizeof(ID3Header);
        }
        else if (descriptorMatch(mpeg,LameDescriptor,4)) //Lame
        {
             pFrameData=mpeg;
             while ((!descriptorMatch(pFrameData,"LAME")) && (pFrameData < mpeg + FileSize)) pFrameData++;
             pFrameData+=4;
        }
        else
        {
            pFrameData = mpeg;
        }
        while (!audioFrame(pFrameData))
        {
            if (pFrameData >= mpeg + FileSize) return nullptr;
            pFrameData++;
        }
        return pFrameData;
    }

private:
    struct ID3Header
    {
        char ID[3];
        ushort version;
        byte flags;
        byte size[4];
    };
    struct ID3ExtendedHeader
    {
        byte size[4];
        byte flagCount;
        byte flag[1];
    };
    static inline bool audioFrame(const byte* p)
    {
        /* This should give a good guess if we have an MPEG audio header */
        return (p[0] == 0xff && ((p[1]>>5)&0x7) == 0x7 &&
           ((p[1]>>1)&0x3) != 0 && ((p[2]>>4)&0xf) != 0xf &&
           ((p[2]>>2)&0x3) != 0x3)
            /* got mpeg audio header */;
    }
    static inline uint syncSafe(const byte* p)
    {
        return uint((p[3] & 0x7F) | ((p[2] & 0x7F) << 7) | ((p[1] & 0x7F) << 14) | ((p[0] & 0x7F) << 21));
    }
    static bool inline descriptorMatch(const void *descriptor, const void *s, const ulong64 l=4)
    {
        return (memcmp(descriptor,s,l)==0);
    }
    IMP3Layer* currentLayer = nullptr;
    IMP3Layer* nullLayer = nullptr;
    IMP3Layer* tempLayer = nullptr;
};

#pragma pack(pop)

#endif // CMP3DEC_H
