#ifndef CSAMPLERFORM_H
#define CSAMPLERFORM_H

#include "csamplerdevice.h"
//#include <QDialog>
#include <QMenu>
#include "csoftsynthsform.h"

namespace Ui {
    class CSamplerForm;
}

class CSamplerForm : public CSoftSynthsForm
{
    Q_OBJECT

public:
    explicit CSamplerForm(IDevice* Device, QWidget *parent = 0);
    ~CSamplerForm();
    void Init(CSamplerDevice* Device);
    void unserializeCustom(const QDomLiteElement* xml);
    void serializeCustom(QDomLiteElement* xml) const;
    void ReleaseLoop();
    void ConvertSfz(const QString& Path);
protected:
    void mousePressEvent(QMouseEvent* e);
private slots:
    void loadSfz();
private:
    Ui::CSamplerForm *ui;
    CSamplerDevice* m_Sampler;
    QMenu* menu;
};

#endif // CSAMPLERFORM_H
