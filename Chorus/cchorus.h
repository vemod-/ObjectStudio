#ifndef CCHORUS_H
#define CCHORUS_H

#include "idevice.h"
#include "biquad.h"
#include "cringbuffer.h"

#define DEPTH_BUFLEN 450
#define DELAY_BUFLEN 19200

/* Max. frequency setting */
#define MAX_FREQ 5.0f

/* bandwidth of highpass filters (in octaves) */
#define HP_BW 1

/* cosine table for fast computations */
#define COS_TABLE_SIZE 1024

class CChorus : public IDevice
{
public:
    CChorus();
    void init(const int Index, QWidget* MainWindow);
    void process();
private:
    enum JackNames
    {jnOut,jnIn};
    enum ParameterNames
    {pnFrequency,pnPhase,pnDepth,pnDelay,pnContour,pnEffect};
    CRingBuffer ring_L;
    CRingBuffer ring_R;
    uint pos_L;
    uint pos_R;
    float Frequency;
    int Phase;
    float Depth;
    int Delay;
    int Contour;
    float DryLevel;
    float WetLevel;
    float d_pos;
    float cm_phase;
    CBiquad highpass_L;
    CBiquad highpass_R;
    float cos_table[COS_TABLE_SIZE];
    void inline updateDeviceParameter(const CParameter* p = nullptr);
};

#endif // CCHORUS_H
