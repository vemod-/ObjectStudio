#ifndef CSPECTRUMFORM_H
#define CSPECTRUMFORM_H

#include "cspectrumcontrol.h"
#include <QDialog>
#include "csoftsynthsform.h"

namespace Ui {
class CSpectrumForm;
}

class CSpectrumForm : public CSoftSynthsForm
{
    Q_OBJECT

public:
    explicit CSpectrumForm(IDevice* Device, QWidget *parent = 0);
    ~CSpectrumForm();
    CSpectrumControl* Spectrum;
protected:
    void timerEvent(QTimerEvent *event);
private:
    Ui::CSpectrumForm *ui;
};

#endif // CSPECTRUMFORM_H
