#include "cdevicelist.h"

CDeviceListBase::~CDeviceListBase(){}

CDeviceList::~CDeviceList()
{
    setPolyphony(1);
    m_PolyDevices.clear();
}
/*
CPolyDeviceList::~CPolyDeviceList()
{
    setPolyphony(1);
    PolyDevices.clear();
}
*/
