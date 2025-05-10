#ifndef CPROGRAMBOX_H
#define CPROGRAMBOX_H

#include "idevice.h"
#include "cdesktopcomponent.h"
#include "cdesktopcontainer.h"
#include "cmacroboxform.h"
#include <QStackedLayout>
#include "qsynthbuttonpanel.h"

#define MaxPrograms 16

class CProgramBox : public IDevice
{
public:
    CProgramBox();
    ~CProgramBox();
    void init(const int Index, QWidget* MainWindow);
    CAudioBuffer* getNextA(const int ProcIndex);
    CMIDIBuffer* getNextP(const int ProcIndex);
    float getNext(const int ProcIndex);
    void serializeCustom(QDomLiteElement* xml) const;
    void unserializeCustom(const QDomLiteElement* xml);
    void tick();
private:
    //enum JackNames
    //{jnOut,jnIn,jnMIDIIn};
    enum ParameterNames
    {pnMIDIChannel,pnProgram};
    void inline updateDeviceParameter(const CParameter* p = nullptr);
    QList<IJack*> JacksCreated;
    QList<CInJack*> InsideJacks;
    CMIDIBuffer MIDIBuffer;
    QSynthButtonPanel* buttonPanel;
    QList<CDesktopContainer*> Desktops;
    //QRecursiveMutex mutex;
    QStackedLayout* layout;
    int currentIndex;
    inline int program() const { return m_Parameters[pnProgram]->Value-1; }
    inline CDesktopContainer* desktopContainer() const { return Desktops[program()]; }
    inline CDesktopComponent* desktopComponent() const { return desktopContainer()->Desktop; }
    inline CDeviceList* deviceList() const { return desktopComponent()->deviceList(); }
    inline CMacroBoxForm* form() const { return static_cast<CMacroBoxForm*>(m_Form); }
};

#endif // CPROGRAMBOX_H
