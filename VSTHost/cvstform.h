#ifndef CVSTFORM_H
#define CVSTFORM_H

#include "iaudiopluginhost.h"
#include <QComboBox>
#include "csoftsynthsform.h"

namespace Ui {
class CVSTForm;
}

class CVSTForm : public CSoftSynthsForm
{
    Q_OBJECT

public:
    explicit CVSTForm(IAudioPlugInHost* plug, IDevice* Device, QWidget *parent = 0);
    ~CVSTForm();
    IAudioPlugInHost* plugIn;
    void fillList(int CurrentProgram=-1);
    void setBankPreset(const int programIndex);
private slots:
    void PlugInIndexChanged();
    void ChangeBankPreset(int programIndex);
    //void updateHost();
private:
    Ui::CVSTForm *ui;
    QComboBox* li;
    QWidget* listHolder;
    QTimer updateTimer;
protected:
    bool event(QEvent *event);
    void timerEvent(QTimerEvent* event);

};

#endif // CVSTFORM_H
