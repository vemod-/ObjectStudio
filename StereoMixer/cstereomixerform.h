#ifndef CSTEREOMIXERFORM_H
#define CSTEREOMIXERFORM_H

#include "cdevicelist.h"
#include "cmixerwidget.h"

namespace Ui {
class CStereoMixerForm;
}

class CStereoMixerForm : public CSoftSynthsForm
{
    Q_OBJECT
    
public:
    explicit CStereoMixerForm(IDevice* Device, QWidget *parent = 0);
    ~CStereoMixerForm();
    void serializeCustom(QDomLiteElement* xml) const;
    void unserializeCustom(const QDomLiteElement* xml);
    void setSender(const QString& s, const int index);
    CMixerWidget* m_Mx;
    CDeviceList deviceList;
    QList<CDeviceContainer*> Effects;
private:
    Ui::CStereoMixerForm *ui;
};

#endif // CSTEREOMIXERFORM_H
