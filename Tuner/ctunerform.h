#ifndef CTUNERFORM_H
#define CTUNERFORM_H

#include "cpitchdetect.h"
#include "YinPitchDetector.h"
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
    CYIN PD;
    void setPitchRecord();
    void setRate(int r);
    void setCalib(double c);
private:
    Ui::CTunerForm *ui;
};

#endif // CTUNERFORM_H
