#ifndef CEQUALIZERFORM_H
#define CEQUALIZERFORM_H

#include "cwavebank.h"
#include "qcanvas.h"
#include "cequalizerframe.h"

#define EQUALIZERCLASS DEVICEFUNC(CEqualizer)

namespace Ui {
    class CEqualizerForm;
}

class CEqualizerForm : public CSoftSynthsForm
{
    Q_OBJECT

public:
    explicit CEqualizerForm(IDevice* Device, QWidget *parent = nullptr);
    ~CEqualizerForm();
    void Init();
    void serializeCustom(QDomLiteElement* xml) const;
    void unserializeCustom(const QDomLiteElement* xml);
    void Reset();
    void Peak();
    float PeakVal[8]={0};
public slots:
    void Draw();
    void DrawGraph();
private:
    Ui::CEqualizerForm *ui;
    CWaveBank W;
    QCanvas* Canvas;
    void DrawBg();
    QList<CEqualizerFrame*> frames;
protected:
    void timerEvent(QTimerEvent *);
};

#endif // CEQUALIZERFORM_H
