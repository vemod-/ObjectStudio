#include "clayer3.h"

CLayer3::~CLayer3()
{
    if (xform) delete xform;
    if (l3process) delete l3process;
}





