#ifndef CDEVICECLASS_H
#define CDEVICECLASS_H

#include "idevice.h"
#ifdef BUILD_WITH_STATIC
#include "../PluginLoader/caddins.h"
#endif
#include headerfile

#ifdef BUILD_WITH_STATIC
    #define expandCreateFunction(x) createinstance_##x
    #define createfunctionname(devnm) expandCreateFunction(devnm)
    #define expandCreateFunction1(x) _createinstance_##x
    #define createfunctionname1(devnm) expandCreateFunction1(devnm)
    IDevice* createfunctionname1(devicelib)() {return new deviceclass;};
    instancefunc Q_DECL_EXPORT createfunctionname(devicelib)=createfunctionname1(devicelib);//_createinstance;
#else
    IDevice* _createinstance();
    instancefunc Q_DECL_EXPORT createinstance=_createinstance;
#endif
    const char* Q_DECL_EXPORT name=devicename;

#define STRINGIFY(...) #__VA_ARGS__
#define TOSTRING(...) STRINGIFY(__VA_ARGS__)
#ifdef devicejacks
    const char* Q_DECL_EXPORT jacks = TOSTRING(devicejacks);
#else
    const char* Q_DECL_EXPORT jacks="";
#endif
#ifndef devicecategory
#define devicecategory 0
#endif

    extern "C" const int Q_DECL_EXPORT category = static_cast<int>(devicecategory);

#ifdef BUILD_WITH_STATIC
    #define expandPluginFunction(x) registerPlugin_##x
    #define pluginfunctionname(devnm) expandPluginFunction(devnm)

    extern "C" void pluginfunctionname(devicelib)() __attribute__((used));
    extern "C" void pluginfunctionname(devicelib)() {
        qDebug() << "register " << devicename;
#ifdef devicejacks
        CAddIns::registerAddIn((voidinstancefunc)createfunctionname(devicelib), devicename, static_cast<int>(devicecategory), TOSTRING(devicejacks));
#else
        CAddIns::registerAddIn((voidinstancefunc)createfunctionname(devicelib), devicename, static_cast<int>(devicecategory));
#endif
    }

#endif

#endif // CDEVICECLASS_H
