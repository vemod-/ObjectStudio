#ifndef IHOST_H
#define IHOST_H

#include <QString>
#include <QPoint>
#include "softsynthsdefines.h"

class IDevice;
class CParameter;

class IHost
{
public:
    IHost(){}
    virtual ~IHost();
    virtual void parameterChange(IDevice* /*device*/, const CParameter* /*parameter*/ = nullptr){}
    //virtual void bankPresetChange(IDevice* /*device*/, const int /*program*/){}
    virtual void activate(IDevice* /*Device*/){}
    virtual void takeString(IDevice* /*Device*/, const int /*type*/, const QString& /*s*/) {}
};

class ITicker
{
public:
    ITicker(){}
    virtual ~ITicker();
    virtual void tick(){ }
    virtual void skip(const ulong64 /*samples*/){}
    virtual void play(const bool /*FromStart*/){}
    virtual void pause(){}
    virtual ulong ticks() const { return 0; }
    virtual ulong milliSeconds() const { return 0; }
    virtual ulong64 samples() const { return 0; }
};

class IMainPlayer
{
public:
    virtual void skip(const ulong64 /*samples*/){}
    virtual void play(const bool /*FromStart*/){}
    virtual void pause(){}
    virtual bool isPlaying() const { return false; }
    virtual ulong ticks() const { return 0; }
    virtual ulong milliSeconds() const { return 0; }
    virtual ulong64 samples() const { return 0; }
    virtual ulong64 currentSample() const { return 0; }
    virtual ulong currentMilliSecond() const { return 0; }
    virtual void renderWaveFile(const QString /*path*/) {}
};

class IDeviceParent
{
public:
    IDeviceParent(){}
    virtual ~IDeviceParent();
    virtual void hideForms() {}
    virtual void cascadeForms(QPoint& /*p*/){}
    virtual int deviceCount() const { return 0; }
    virtual IDevice* device(const int /*index*/) const { return nullptr; }
};

#endif // IHOST_H
