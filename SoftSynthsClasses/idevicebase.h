#ifndef IDEVICEBASE_H
#define IDEVICEBASE_H

#include <QMutex>

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
class QRecursiveMutex : public QMutex
{
public:
    explicit QRecursiveMutex() : QMutex(QMutex::Recursive) {}
};
#endif

class CParameter;
class CMIDIBuffer;
class CAudioBuffer;

class IParameterHost
{
public:
    virtual ~IParameterHost();
    virtual void updateParameter(const CParameter* /*p*/ = nullptr) {}
};

class IDeviceBase : public IParameterHost
{
public:
    virtual ~IDeviceBase();
    virtual float getNext(const int /*ProcIndex*/)
    {
        return 0;
    }
    virtual CMIDIBuffer* getNextP(const int /*ProcIndex*/)
    {
        return nullptr;
    }
    virtual CAudioBuffer* getNextA(const int /*ProcIndex*/)
    {
        return nullptr;
    }
    virtual void connectionChanged(){}
};

#endif // IDEVICEBASE_H
