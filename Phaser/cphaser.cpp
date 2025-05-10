#include "cphaser.h"

CPhaser::CPhaser()
{
}

Phaser::Phaser() : _fb( .7f )
  , _lfoPhase( 0.f )
  , _depth( 1.f )
  , _zm1( 0.f )
  , _pi(PI_F) {
    Range( 440.f, 1600.f );
    Rate( .5f );
}

void Phaser::Range(float fMin, float fMax) { // Hz
    _dmin = fMin / presets.HalfRate;
    _dmax = fMax / presets.HalfRate;
}

void Phaser::Rate(float rate) { // cps
    _lfoInc = 2.f * _pi * (rate / presets.SampleRate);
}

void Phaser::Feedback(float fb) { // 0 -> <1.
    _fb = fb;
}

void Phaser::Depth(float depth) {  // 0 -> 1.
    _depth = depth;
}

float Phaser::Update(float inSamp) {
    //calculate and update phaser sweep lfo...
    const float d  = _dmin + (_dmax-_dmin) * ((sinf( _lfoPhase ) + 1.f)/2.f);
    _lfoPhase += _lfoInc;
    if( _lfoPhase >= _pi * 2.f )
    {
        _lfoPhase -= _pi * 2.f;
    }

    //update filter coeffs
    for( int i=0; i<6; i++ )
    {
        _alps[i].Delay( d );
    }

    //calculate output
    const float y = 	_alps[0].Update(
                _alps[1].Update(
                _alps[2].Update(
                _alps[3].Update(
                _alps[4].Update(
                _alps[5].Update( inSamp + _zm1 * _fb ))))));
    _zm1 = y;

    return inSamp + y * _depth;
}

Phaser::AllpassDelay::AllpassDelay() : _a1( 0.f )
  , _zm1( 0.f ) {}

void Phaser::AllpassDelay::Delay(float delay) { //sample delay time
    _a1 = (1.f - delay) / (1.f + delay);
}

float Phaser::AllpassDelay::Update(float inSamp) {
    const float y = inSamp * -_a1 + _zm1;
    _zm1 = y * _a1 + inSamp;

    return y;
}

void CPhaser::init(const int Index, QWidget* MainWindow) {
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveOut(jnOut);
    addJackWaveIn();
    startParameterGroup();
    addParameterCutOff("Range Min",440);
    addParameterCutOff("Range Max",1600);
    endParameterGroup();
    addParameterRate("Rate",50);
    addParameterPercent("Feedback",70);
    addParameterPercent("Depth",100);
    updateDeviceParameter();
}

CAudioBuffer *CPhaser::getNextA(const int ProcIndex) {
    const CMonoBuffer* InBuffer = FetchAMono(jnIn);
    if (!InBuffer->isValid()) return nullptr;
    CMonoBuffer* OutBuffer=MonoBuffer(ProcIndex);
    for (uint i=0;i<m_BufferSize;i++)
    {
        OutBuffer->setAt(i,P.Update(InBuffer->at(i)));
    }
    return OutBuffer;
}

void CPhaser::updateDeviceParameter(const CParameter* /*p*/) {
    P.Range(m_Parameters[pnRangeMin]->Value,m_Parameters[pnRangeMax]->Value);
    P.Rate(m_Parameters[pnRate]->PercentValue);
    P.Feedback(m_Parameters[pnFeedback]->scaleValue(0.0098f));
    P.Depth(m_Parameters[pnDepth]->PercentValue);
}
