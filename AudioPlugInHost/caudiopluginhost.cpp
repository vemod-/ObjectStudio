#include "caudiopluginhost.h"

CAudioPlugInHost::CAudioPlugInHost() : CDeviceContainer("AudioPlugInHost")
{
    menuWidget = new CMenuWidget(this);
}
