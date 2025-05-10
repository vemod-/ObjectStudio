#ifndef CSF2PLAYERFORM_H
#define CSF2PLAYERFORM_H

#include <QDialog>
//#include "softsynthsclasses.h"
//#include "csf2player.h"
#include "csoftsynthsform.h"

#define SF2CLASS DEVICEFUNC(CSF2Player)
#define SF2DEVICE (&(SF2CLASS->SF2Device))

namespace Ui {
    class CSF2PlayerForm;
}

class CSF2PlayerForm : public CSoftSynthsForm
{
    Q_OBJECT

public:
    explicit CSF2PlayerForm(IDevice* Device, QWidget *parent = nullptr);
    ~CSF2PlayerForm();

private:
    Ui::CSF2PlayerForm *ui;
    void FillBankList(int Bank, int Preset);
    void FillPresetList(int Bank, int Preset);
    //QRecursiveMutex mutex;
protected:
    void timerEvent(QTimerEvent *);
public:		// User declarations
    void serializeCustom(QDomLiteElement* xml) const;
    void unserializeCustom(const QDomLiteElement* xml);
    void SetPatchResponse();
    void SetProgram(const int Program);
public slots:
    void ChangeBank(int index);
    void ChangePreset(int index);
private slots:
    void OpenClick();
    void TestMouseDown();
    void TestMouseUp();
};

#endif // CSF2PLAYERFORM_H
