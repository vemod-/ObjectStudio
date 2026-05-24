#ifndef CDUALMONOCONTAINER_H
#define CDUALMONOCONTAINER_H

#include "cstereocontainerbase.h"

#define devicejacks stereoin,midiin,stereoout
#define devicecategory Container | Effect | Generator | Instrument

class CDualMonoContainer : public CStereoContainerBase {
public:
    CDualMonoContainer();
};

#endif // CDUALMONOCONTAINER_H
