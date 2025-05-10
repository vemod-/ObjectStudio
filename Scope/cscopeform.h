#ifndef CSCOPEFORM_H
#define CSCOPEFORM_H

#include "cscopecontrol.h"
#include <QDialog>
#include <QTabWidget>
#include "csoftsynthsform.h"

namespace Ui {
    class CScopeForm;
}

class CScopeForm : public CSoftSynthsForm
{
    Q_OBJECT

public:
    explicit CScopeForm(IDevice* Device, QWidget *parent = 0);
    ~CScopeForm();
    CScopeControl* Scope;
protected:
    void timerEvent(QTimerEvent *event);
private:
    Ui::CScopeForm *ui;
};

#endif // CSCOPEFORM_H
