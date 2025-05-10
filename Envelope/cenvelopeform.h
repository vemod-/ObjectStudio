#ifndef CENVELOPEFORM_H
#define CENVELOPEFORM_H

#include "cadsrwidget.h"
#include "csoftsynthsform.h"

namespace Ui {
    class CEnvelopeForm;
}

class CEnvelopeForm : public CSoftSynthsForm
{
    Q_OBJECT

public:
    explicit CEnvelopeForm(IDevice* Device, QWidget *parent = 0);
    ~CEnvelopeForm();
    CADSRWidget* ADSRWidget;
private slots:
    //void UpdateDevice(CADSR::ADSRParams ADSRParams);
private:
    Ui::CEnvelopeForm *ui;
};

#endif // CENVELOPEFORM_H
