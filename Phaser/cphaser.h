#ifndef CPHASER_H
#define CPHASER_H

#include "idevice.h"

class Phaser : protected IPresetRef
{
public:
    Phaser();
    void Range( float fMin, float fMax );
    void Rate( float rate );
    void Feedback( float fb );
    void Depth( float depth );
    float Update( float inSamp );
private:
    class AllpassDelay{
    public:
        AllpassDelay();
        void Delay( float delay );
        float Update( float inSamp );
    private:
        float _a1, _zm1;
    };
    AllpassDelay _alps[6];
    float _dmin, _dmax; //range
    float _fb; //feedback
    float _lfoPhase;
    float _lfoInc;
    float _depth;
    float _zm1;
    float _pi;
};

class CPhaser : public IDevice
{
public:
    CPhaser();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
private:
    enum JackNames
    {jnOut,jnIn};
    enum ParameterNames
    {pnRangeMin,pnRangeMax,pnRate,pnFeedback,pnDepth};
    Phaser P;
    void inline updateDeviceParameter(const CParameter* p = nullptr);
};

#endif // CPHASER_H
