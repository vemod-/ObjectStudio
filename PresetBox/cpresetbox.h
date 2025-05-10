#ifndef CPRESETBOX_H
#define CPRESETBOX_H

#include "idevice.h"
#include "cdesktopcomponent.h"
#include "cdesktopcontainer.h"
#include "cmacroboxform.h"
#include "qsynthbuttonpanel.h"
//#include "array"

#define MaxPresets 16

class CPresetBox : public IDevice
{

public:
    CPresetBox();
    ~CPresetBox();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
    CMIDIBuffer* getNextP(const int ProcIndex);
    float getNext(const int ProcIndex);
    void serializeCustom(QDomLiteElement* xml) const;
    void unserializeCustom(const QDomLiteElement* xml);
private:
    //enum JackNames
    //{jnOut,jnIn,jnMIDIIn};
    enum ParameterNames
    {pnMIDIChannel,pnPreset};
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    QList<IJack*> JacksCreated;
    QList<CInJack*> InsideJacks;
    //QDomLiteElementList presetList;
    //std::array<QDomLiteElement*,MaxPresets> presetList={nullptr};
    QDomLiteElement presetList[MaxPresets];
    CMIDIBuffer MIDIBuffer;
    QSynthButtonPanel* buttonPanel;
    //QRecursiveMutex mutex;
    int currentIndex;
    inline int preset() const { return m_Parameters[pnPreset]->Value-1; }
    inline CDesktopContainer* desktopContainer() const { return form()->DesktopContainer; }
    inline CDesktopComponent* desktopComponent() const { return desktopContainer()->Desktop; }
    inline CDeviceList* deviceList() const { return desktopComponent()->deviceList(); }
    inline CMacroBoxForm* form() const { return static_cast<CMacroBoxForm*>(m_Form); }
    void savePreset(const int index);
    void loadPreset(const int index);
};

#endif // CPRESETBOX_H
