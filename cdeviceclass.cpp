#include "cdeviceclass.h"

#ifndef BUILD_WITH_STATIC
IDevice* _createinstance(){return new deviceclass;}
#endif
