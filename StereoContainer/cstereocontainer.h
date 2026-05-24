#ifndef CSTEREOCONTAINER_H
#define CSTEREOCONTAINER_H

#include "cstereocontainerbase.h"

#define devicejacks stereoin,midiin,stereoout
#define devicecategory Container | Effect

class CStereoContainer : public CStereoContainerBase {
public:
    CStereoContainer();
};

#endif // CSTEREOCONTAINER_H
