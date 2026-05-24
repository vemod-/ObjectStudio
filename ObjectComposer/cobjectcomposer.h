#ifndef COBJECTCOMPOSER_H
#define COBJECTCOMPOSER_H

#include "idevice.h"

#define devicejacks stereoout
#define devicecategory Generator

class CObjectComposer : public IDevice
{
public:
    CObjectComposer();
    ~CObjectComposer();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
    void initWithFile(const QString& path);
    enum ParameterNames
    {pnVolume,pnHumanize};
private:
    enum JackNames
    {devicejacks};
    void inline updateDeviceParameter(const CParameter* p = nullptr);
};

#endif // COBJECTCOMPOSER_H
