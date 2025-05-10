#ifndef COBJECTCOMPOSER_H
#define COBJECTCOMPOSER_H

#include "idevice.h"

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
    {jnOut};
    void inline updateDeviceParameter(const CParameter* p = nullptr);
};

#endif // COBJECTCOMPOSER_H
