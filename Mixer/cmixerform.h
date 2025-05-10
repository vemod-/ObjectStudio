#ifndef CMIXERFORM_H
#define CMIXERFORM_H

#include "idevice.h"
#include "cmixerframe.h"

#define MIXERCLASS DEVICEFUNC(CMixer)

namespace Ui {
    class CMixerForm;
}

class CMixerForm : public CSoftSynthsForm
{
    Q_OBJECT

public:
    explicit CMixerForm(IDevice* Device, QWidget *parent = 0);
    ~CMixerForm();
    void unserializeCustom(const QDomLiteElement* xml);
    void serializeCustom(QDomLiteElement* xml) const;
    void Reset();
    void setSender(const QString& s, int Index);
private:
    Ui::CMixerForm *ui;
    QList<CMixerFrame*> MF;
protected:
    void timerEvent(QTimerEvent *event);
    void showEvent(QShowEvent *);
private slots:
    void LeftChanged(int Value);
    void RightChanged(int Value);
    void SoloClicked(bool Pressed, int Index);
};

#endif // CMIXERFORM_H
