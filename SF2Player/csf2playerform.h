#ifndef CSF2PLAYERFORM_H
#define CSF2PLAYERFORM_H

#include <QDialog>
#include "softsynthsclasses.h"

namespace Ui {
    class CSF2PlayerForm;
}

class CSF2PlayerForm : public CSoftSynthsForm
{
    Q_OBJECT

public:
    explicit CSF2PlayerForm(IDevice* Device, QWidget *parent = 0);
    ~CSF2PlayerForm();

private:
    Ui::CSF2PlayerForm *ui;
    void FillPresetList(int Bank, int Preset);
    void FillPresetList2(int Preset);
public:		// User declarations
    const QString CustomSave();
    void CustomLoad(const QString& XML);
    void SetPatchResponse(bool value);
    void setVolume(int value);
    void SetProgram(const int Bank, const int Preset);
private slots:
    void OpenClick();
    void TestMouseDown();
    void TestMouseUp();
    void ChangeBank();
    void ChangePreset();
    void PatchToggled(bool value);
    void VolChanged(int value);
};

#endif // CSF2PLAYERFORM_H
