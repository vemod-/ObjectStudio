//
// Copyright(C) 2000 by TOx2RO / O2 Software All rights reserved.
//
// MP3Play.h
//

#ifndef MP3PLAY_H
#define MP3PLAY_H

#include "iwavefile.h"
#include <QtCore>
#include "cmp3dec.h"

#pragma pack(push,1)

class CMP3File : public IWaveFile
{
public:
    CMP3File()
    {
        m_Channels = 0;
        m_Frequency = 0;
    }
    ~CMP3File()
    {
        if (MP3decoder) delete MP3decoder;
    }
    bool assign(const QByteArray& b);
    void createFloatBuffer(CChannelBuffer& OutBuffer, const uint Samplerate);
private:
    // 10-Bands Equalizer
    struct EQ
    {
        signed char _60;       // 60Hz
        signed char _170;       // 170Hz
        signed char _310;       // 310Hz
        signed char _600;       // 600Hz
        signed char _1k;       // 1kHz
        signed char _3k;       // 3kHz
        signed char _6k;       // 6kHz
        signed char _12k;       // 12kHz
        signed char _14k;       // 14kHz
        signed char _16k;       // 16kHz
    };
    void Equalize(EQ eq);
    void decode(std::vector<short>& PCMBuffer);
    byte* pFrameData = nullptr;
    ulong64 FileSize = 0;
    CMP3dec* MP3decoder = nullptr;
};

#pragma pack(pop)

#endif
