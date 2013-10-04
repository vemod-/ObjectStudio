#ifndef CKNOBCONTROL_H
#define CKNOBCONTROL_H

#include <QWidget>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QMenu>
#include <QMouseEvent>
#include <QWidgetAction>
#include <QSignalMapper>
#include "softsynthsdefines.h"
#include "softsynthsclasses.h"

namespace Ui {
    class CKnobControl;
}

class CKnobControl : public QWidget
{
    Q_OBJECT
public:
    explicit CKnobControl(QWidget *parent = 0);
    ~CKnobControl();
    void setValue(int Value, const ParameterType& p);
    void setLabels(int Value, const ParameterType &p);
    int value();
protected:
    void mousePressEvent(QMouseEvent *);
signals:
    void valueChanged(int Value);
private:
    ParameterType Parameter;
    Ui::CKnobControl *ui;
    QMenu* popup;
    QWidgetAction* spinboxAction;
    QDoubleSpinBox* spinbox;
    QSignalMapper* mapper;

private slots:
    void SetNumericValue(double Value);
    void SetdBValue(double Value);
};

#endif // CKNOBCONTROL_H
