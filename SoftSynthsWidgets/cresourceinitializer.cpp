#include "cresourceinitializer.h"
#include <QtGlobal>

// Initiera alla resurser här
void CResourceInitializer::initializeResources() {
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        Q_INIT_RESOURCE(desktopresources);
        Q_INIT_RESOURCE(QCheckboxResources);
        Q_INIT_RESOURCE(SynthKnobResources);
        Q_INIT_RESOURCE(SynthPanelResources);
        Q_INIT_RESOURCE(QSynthSliderResources);
        Q_INIT_RESOURCE(synthswitchresources);
        Q_INIT_RESOURCE(synthbuttonpanelresources);

    }
}
