#ifndef CVSTFORM_H
#define CVSTFORM_H

#include "softsynthsclasses.h"
#include "IAudioPlugInHost.h"
#include <QComboBox>

namespace Ui {
class CVSTForm;
}

class CVSTForm : public CSoftSynthsForm
{
    Q_OBJECT

public:
    explicit CVSTForm(IAudioPlugInHost* plug, IDevice* Device, QWidget *parent = 0);
    ~CVSTForm();
    IAudioPlugInHost* PlugIn;
    void FillList(int CurrentProgram=-1);
    void SetProgram(const int programIndex);
private slots:
    void PlugInIndexChanged();
    void ChangeProgram(int programIndex);
private:
    Ui::CVSTForm *ui;
    QComboBox* li;
    QWidget* listHolder;
protected:
    bool event(QEvent *event);
    void timerEvent(QTimerEvent* event);

};

#endif // CVSTFORM_H
