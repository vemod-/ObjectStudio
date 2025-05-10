#ifndef CTUNERFORM_H
#define CTUNERFORM_H

#include "cpitchdetect.h"
#include "csoftsynthsform.h"

namespace Ui {
class CTunerForm;
}

class CTunerForm : public CSoftSynthsForm
{
    Q_OBJECT

public:
    explicit CTunerForm(IDevice* Device, QWidget *parent = 0);
    ~CTunerForm();
    CPitchDetect PD;
protected:
    void timerEvent(QTimerEvent* e);
private:
    Ui::CTunerForm *ui;
    int m_TimerID;
};

#endif // CTUNERFORM_H
