//
// MPEG1 Audio Layer-3 Class
// Copyright(C) 2000 by TOx2RO / O2 Software All rights reserved.
//
// MP3Play.cpp
//

#include "mp3play.h"
#include <cwavefile.h>

bool CMP3File::assign(const QByteArray& b)
{
    QMutexLocker locker(&mutex);
    if (MP3decoder) delete MP3decoder;
    MP3decoder = nullptr;

    auto pSrcFile=reinterpret_cast<byte*>(const_cast<char*>(b.constData()));
    FileSize=ulong64(b.size());
    pFrameData = CMP3dec::findAudioFrame(pSrcFile,FileSize);
    if (!pFrameData) return false;

    FileSize -= ulong64(pFrameData - pSrcFile);
    while (descriptorMatch(&pSrcFile[FileSize - 128],"TAG",3)) FileSize -= 128;

    MP3decoder = new CMP3dec(pFrameData);
    if (!MP3decoder->isInitialised()) return false;
    m_Frequency=MP3decoder->mp3Frequency();
    m_Channels=MP3decoder->mp3Channels();
    return true;
}

void CMP3File::decode(std::vector<short>& PCMBuffer)
{
    QMutexLocker locker(&mutex);
    if (!MP3decoder->isInitialised()) return;
    PCMBuffer.resize(0xFFFFFF);
    ulong64 inputPos = 0;
    ulong64 outputPos = 0;

    while (inputPos < FileSize)
    {
        int outSize = MP3decoder->mp3GetDecodeInfo(pFrameData + inputPos);
        if (outSize == 0) break;
        const ulong64 byteSize=PCMBuffer.size()*sizeof(short);
        ulong64 cp=byteSize;
        while (ulong64(outSize)+outputPos  > cp) cp += 0xFFFFFF;
        if (cp > byteSize) PCMBuffer.resize(cp/sizeof(short));

        int inSize = MP3decoder->mp3DecodeFrame(pFrameData + inputPos,PCMBuffer.data() + (outputPos / sizeof(short)));
        if (inSize == 0) break;
        inputPos += ulong64(inSize);
        outputPos  += ulong64(outSize);
    }
    PCMBuffer.resize(outputPos /sizeof(short));
    qDebug() << outputPos  << 0xFFFFFF;
}

void CMP3File::createFloatBuffer(CChannelBuffer& OutBuffer, const uint Samplerate)
{
    QMutexLocker locker(&mutex);
    std::vector<short> PCMBuffer;
    decode(PCMBuffer);
    m_WaveStart=reinterpret_cast<byte*>(PCMBuffer.data());
    m_AuEncoding=AUDIO_FILE_ENCODING_LINEAR_16;
    m_SampleBitSize=16;
    m_ChunkSize=PCMBuffer.size()*sizeof(short);
    IWaveFile::createFloatBuffer(OutBuffer,Samplerate);
}

void CMP3File::Equalize(EQ eq)
{
    int tEQ[10];

    if(!MP3decoder->isInitialised()) return;

    tEQ[0] = eq._60;
    tEQ[1] = eq._170;
    tEQ[2] = eq._310;
    tEQ[3] = eq._600;
    tEQ[4] = eq._1k;
    tEQ[5] = eq._3k;
    tEQ[6] = eq._6k;
    tEQ[7] = eq._12k;
    tEQ[8] = eq._14k;
    tEQ[9] = eq._16k;

    MP3decoder->mp3SetEqualizer(&tEQ[0]);
}
