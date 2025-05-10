#ifndef CKEYBOARDFORM_H
#define CKEYBOARDFORM_H

#include "csoftsynthsform.h"

namespace Ui {
class CKeyboardForm;
}

class CKeyboardForm  : public CSoftSynthsForm
{
    Q_OBJECT

public:
    explicit CKeyboardForm(IDevice* Device, QWidget *parent = nullptr);
    ~CKeyboardForm();
    int pitchWheel();
    int modWheel1();
    int modWheel2();
private:
    Ui::CKeyboardForm *ui;
private slots:
    void NoteOn(int pitch);
    void NoteOff(int pitch);
    void PitchWheelRelease();
    void ModWheel1Release();
    void ModWheel2Release();
};

#endif // CKEYBOARDFORM_H
