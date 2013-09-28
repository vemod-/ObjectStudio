#ifndef CAUDIOUNITFORM_H
#define CAUDIOUNITFORM_H

#include "softsynthsclasses.h"
#include "caudiounitclass.h"
#include <QComboBox>

namespace Ui {
class CAudioUnitForm;
}

class CAudioUnitForm : public CSoftSynthsForm
{
    Q_OBJECT
public:
    explicit CAudioUnitForm(IDevice* Device, QWidget *parent = 0);
    ~CAudioUnitForm();
    CAudioUnitClass* AU;
    void FillList(int CurrentProgram=-1);
    void SetProgram(const int index);
private slots:
    void UnitIndexChanged();
    void ChangeProgram(int programIndex);
private:
    Ui::CAudioUnitForm *ui;
    QComboBox* li;
    QWidget* listHolder;
protected:
    bool event(QEvent *event);
    void timerEvent(QTimerEvent* event);
};

#endif // CAUDIOUNITFORM_H
