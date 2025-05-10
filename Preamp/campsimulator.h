#ifndef CAMPSIMULATOR_H
#define CAMPSIMULATOR_H

#include "math.h"

#define clip_top 0.5f
#define clip_bottom -0.45f

class CAmpSimulator
{
public:
    CAmpSimulator()
    {
        fParam3 = 0.5f;
        clip = 0.5;
        drive = 1.5f; //1.0 + 0.5
        iterations = 4; //((3 / 15) * 15) + 1
    }
    float inline process(const float input)
    {
        float output;
        prevTemp = prevTemp * fParam3 + input * (1.0f - fParam3);
        output = input * 0.7f + prevTemp * 0.3f;
        for (int j = 0; j < iterations; ++j)
        {
            output = output * drive;
            if (output > clip_top)
            {
                float a = output - clip_top;
                a *= 1.0f - clip_top;
                a *= clip;
                output = clip_top + fabsf(a);
            }
            if (output < clip_bottom)
            {
                float a = output - clip_bottom;
                a *= 1.0f + clip_bottom;
                a *= clip;
                output = clip_bottom - fabsf(a);
            }
        }
        return (output * simulation) + (input * clean);
    }
    void setAmount(const float amount)
    {
        simulation = amount * 0.25f;
        clean = 1.f - amount;
    }
    float simulation;
    float clean;
    float prevTemp;
    float fParam3;
    float clip;
    float drive;
    int iterations;
};

#endif // CAMPSIMULATOR_H
