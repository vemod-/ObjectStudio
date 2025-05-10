#include "cchorus.h"

CChorus::CChorus()
{
    pos_L=0;
    pos_R=0;
    Frequency=0;
    Phase=0;
    Depth=0;
    Delay=0;
    Contour=0;
    DryLevel=0;
    WetLevel=0;
    d_pos=0;
    cm_phase=0;
    for (int i = 0; i < COS_TABLE_SIZE; i++) cos_table[i] = cosf(i * 2.0f * PI_F / COS_TABLE_SIZE);
}

void CChorus::init(const int Index, QWidget* MainWindow) {
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackStereoOut(jnOut);
    addJackStereoIn();
    addParameterRate("Rate",400);
    addParameter(CParameter::Numeric,"Phase","Centigrades",0,180,0,"",100);
    addParameterPercent("Depth",50);
    addParameterPercent("Delay",50);
    addParameterCutOff("Contour",19200);
    addParameterPercent("Effect",50);
    ring_L.reserve((DEPTH_BUFLEN + DELAY_BUFLEN) * presets.SampleRate / 192000);
    ring_R.reserve((DEPTH_BUFLEN + DELAY_BUFLEN) * presets.SampleRate / 192000);
    updateDeviceParameter();
}

float inline chorus_run(const CRingBuffer* ring,uint pos,float fpos)
{
    const float n = floorf(fpos);
    const float rem = fpos - n;
    return ((1 - rem) * ring->read_buffer(pos, uint(n)) + rem * ring->read_buffer(pos, uint(n) + 1));
}

void CChorus::process() {
    const CStereoBuffer* InBuffer = FetchAStereo(jnIn);
    CStereoBuffer* OutBuffer=StereoBuffer(jnOut);
    float in_L=0;
    float in_R=0;
    for (uint sample_index = 0; sample_index < m_BufferSize; sample_index++)
    {
        if (InBuffer->isValid())
        {
            in_L=InBuffer->at(sample_index);//input_L[sample_index];
            in_R=InBuffer->atR(sample_index);
        }
        ring_L.push_buffer(in_L, pos_L);
        ring_R.push_buffer(in_R, pos_R);
        cm_phase += Frequency / presets.SampleRate * COS_TABLE_SIZE;
        while (cm_phase >= COS_TABLE_SIZE) cm_phase -= COS_TABLE_SIZE;
        float phase_R = cm_phase + (Phase * COS_TABLE_SIZE / 2.0f);
        while (phase_R >= COS_TABLE_SIZE) phase_R -= COS_TABLE_SIZE;
        const float fpos_L = d_pos + Depth * (0.5f + 0.5f * cos_table[uint(cm_phase)]);
        const float fpos_R = d_pos + Depth * (0.5f + 0.5f * cos_table[uint(phase_R)]);
        OutBuffer->setAt(sample_index,
            (DryLevel * in_L) + (WetLevel * highpass_L.run(chorus_run(&ring_L,pos_L,fpos_L))),
            (DryLevel * in_R) + (WetLevel * highpass_R.run(chorus_run(&ring_R,pos_R,fpos_R)))
            );
    }
}

void CChorus::updateDeviceParameter(const CParameter* /*p*/) {
    Frequency=m_Parameters[pnFrequency]->PercentValue;
    Phase=m_Parameters[pnPhase]->Value;
    Depth=presets.SampleRate / 44.1f * m_Parameters[pnDepth]->PercentValue;
    Delay=100 - qMax<int>(m_Parameters[pnDelay]->Value,1);
    d_pos=float(presets.mSecsToSamples(Delay));
    Contour=m_Parameters[pnContour]->Value;
    DryLevel=m_Parameters[pnEffect]->DryValue;
    WetLevel=m_Parameters[pnEffect]->PercentValue;
    highpass_L.hpSetParams(Contour, HP_BW, presets.SampleRate);
    highpass_R.hpSetParams(Contour, HP_BW, presets.SampleRate);
}
