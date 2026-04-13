#ifndef CHARMONIZER_H
#define CHARMONIZER_H

#include "idevice.h"
#include "smbpitchshifter.h"
#include "YinPitchDetector.h"

class CHarmonizer : public IDevice
{

public:
    CHarmonizer();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
    void serializeCustom(QDomLiteElement* xml) const;
    void unserializeCustom(const QDomLiteElement* xml);
private:
    struct shiftMatrix
    {
        int shift[8];
    };
    enum JackNames
    {jnOut,jnIn};
    enum ParameterNames
    {pnNote,pnNote1,pnNote2,pnNote3,pnTune,pnAutotune,pnGlide,pnSlack,pnThreshold,pnOversampling,pnEffect};
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    CYIN PD;
    smbPitchShifter PS;
    shiftMatrix m_Matrix[13]={{0}};
    double s[8]={0};
    float vol[8]={1,1,1,0,0,0,0,0};
    int m_lastKey = 0;
    int m_oldValue=0;
};

#endif // CHARMONIZER_H
