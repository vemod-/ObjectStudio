#ifndef CKNOBCONTROL_H
#define CKNOBCONTROL_H

#include <QWidget>
#include <QComboBox>
#include <QDoubleSpinBox>
#include "qsignalmenu.h"
#include <QMouseEvent>
#include <QWidgetAction>
#include "cparameter.h"

namespace Ui {
    class CKnobControl;
}

class CKnobControl : public QWidget
{
    Q_OBJECT
public:
    enum KnobType {
        Knob,
        Switch,
        Checkbox
    };
    explicit CKnobControl(QWidget *parent = 0);
    ~CKnobControl();
    void setValue(CParameter* p);
    void setLabels(CParameter* p);
    int value();
protected:
    void mousePressEvent(QMouseEvent *);
signals:
    void valueChanged(int Value);
    void requestAutomation(CParameter*);
private:
    CParameter* m_Parameter;
    Ui::CKnobControl *ui;
    QSignalMenu* popup;
    QWidgetAction* spinboxAction;
    QDoubleSpinBox* spinbox;
    KnobType m_KnobType;
    //QRecursiveMutex mutex;
private slots:
    void SetNumericValue(double Value);
    void SetdBValue(double Value);
    void sendAutomationRequest() {
        emit requestAutomation(m_Parameter);
    }
};

#endif // CKNOBCONTROL_H
