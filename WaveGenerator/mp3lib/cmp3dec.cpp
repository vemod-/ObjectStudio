#include "cmp3dec.h"
//#include "include/bstream.h"bstream.h"
//#include "math.h"
//#include "string.h"

CMP3dec::CMP3dec(byte* mpeg)
{
    MPEG_HEADER h(mpeg);
    nullLayer = new IMP3Layer(&h);
    switch (h.layer)
    {
    case MPEG_HEADER::invalidLayer:
        return;
    case MPEG_HEADER::layer_1:
        currentLayer=new CLayer1(&h);
        break;
    case MPEG_HEADER::layer_2:
        currentLayer= new CLayer2(&h);
        break;
    case MPEG_HEADER::layer_3:
        currentLayer= new CLayer3(&h);
        break;
    }
    currentLayer->mp3GetDecodeInfo(mpeg);
    if (!currentLayer->decode_start())
    {
        delete currentLayer;
        currentLayer = nullptr;
    }
}

CMP3dec::~CMP3dec()
{
    if (tempLayer)
    {
        currentLayer=tempLayer;
        tempLayer=nullptr;
    }
    if (currentLayer) delete currentLayer;
    delete nullLayer;
}
/*
CMP3dec::mp3Error CMP3dec::mp3GetLastError()
{
    return m_last_error;
}
*/
/*
int CMP3dec::mp3FindSync(byte* buf, int size, int* sync)
{
    int i;

    *sync = 0;
    size -= 3;
    if (size <= 0) {
        m_last_error = MP3_ERROR_OUT_OF_BUFFER;
        return 0;
    }
    for (i = 0; i < size; i ++) {
        if (buf[i] == 0xFF) {
            if ((buf[i + 1] & 0xF0) == 0xF0) {
                break;
            }
            else if ((buf[i + 1] & 0xF0) == 0xE0) {
                break;
            }
        }
    }
    if (i == size) {
        m_last_error = MP3_ERROR_OUT_OF_BUFFER;
        return 0;
    }
    *sync = i;
    return 1;
}
*/
void CMP3dec::mp3GetDecodeOption(MPEG_DECODE_OPTION* option)
{
    *option = currentLayer->m_option;
}

int CMP3dec::mp3SetDecodeOption(MPEG_DECODE_OPTION* option)
{
    currentLayer->m_option = *option;
    return 1;
}

int CMP3dec::mp3SetEqualizer(int* value)
{
    if (!value)
    {
        currentLayer->setEQEnabled(false);
        return 1;
    }
    currentLayer->setEQEnabled(true);
    currentLayer->setEQ(value);
    return 1;
}

void CMP3dec::mp3MuteStart()
{
    if (tempLayer) return;
    if (!currentLayer) return;
    tempLayer=currentLayer;
    currentLayer=nullLayer;
}

void CMP3dec::mp3MuteEnd()
{
    if (!tempLayer) return;
    if (!currentLayer) return;
    currentLayer=tempLayer;
    tempLayer=nullptr;
    currentLayer->decode_reset();
}


