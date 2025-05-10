#ifndef CCOMPRESSOR_H
#define CCOMPRESSOR_H

#include "idevice.h"
#include <cmath>

class compressor
{

    private:
        float   threshold;
        float   attack, release, envelope_decay;
        float   output;
        float   transfer_A, transfer_B;
        float   env, gain;

    public:
    compressor()
    {
        threshold = 1.f;
        attack = release = envelope_decay = 0.f;
        output = 1.f;

        transfer_A = 0.f;
        transfer_B = 1.f;

        env = 0.f;
        gain = 1.f;
    }

    void set_threshold(float value)
    {
        threshold = value;
        transfer_B = output * powf(threshold,-transfer_A);
    }


    void set_ratio(float value)
    {
        transfer_A = value-1.f;
        transfer_B = output * powf(threshold,-transfer_A);
    }


    void set_attack(float value)
    {
        attack = expf(-1.f/value);
    }


    void set_release(float value)
    {
        release = expf(-1.f/value);
        envelope_decay = expf(-4.f/value); /* = exp(-1/(0.25*value)) */
    }


    void set_output(float value)
    {
        output = value;
        transfer_B = output * powf(threshold,-transfer_A);
    }


    void reset()
    {
        env = 0.f; gain = 1.f;
    }


    void process(float *input_left, float *input_right,float *output_left, float *output_right, int frames)
    {
        //float det, transfer_gain;
        for (int i=0; i<frames; i++)
        {
            float det = fmaxf(fabsf(input_left[i]),fabsf(input_right[i]));
            det += 10e-30f; /* add tiny DC offset (-600dB) to prevent denormals */

            env = det >= env ? det : det+envelope_decay*(env-det);

            const float transfer_gain = env > threshold ? powf(env,transfer_A)*transfer_B:output;

            gain = transfer_gain < gain ?
                            transfer_gain+attack *(gain-transfer_gain):
                            transfer_gain+release*(gain-transfer_gain);

            output_left[i] = input_left[i] * gain;
            output_right[i] = input_right[i] * gain;
        }
    }
};

class CCompressor  : public IDevice
{
public:
    CCompressor();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
private:
    enum JackNames
    {jnIn,jnOut};
    enum ParameterNames
    {pnThreshold,pnRatio,pnAttack,pnRelease,pnOutput};
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    compressor compr;
};

#endif // CCOMPRESSOR_H
