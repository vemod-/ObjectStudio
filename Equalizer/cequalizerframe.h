#ifndef CEQUALIZERFRAME_H
#define CEQUALIZERFRAME_H

#include <QFrame>
#include "cequalizer.h"

namespace Ui {
    class CEqualizerFrame;
}

class CEqualizerFrame : public QFrame
{
    Q_OBJECT

public:
    explicit CEqualizerFrame(QWidget *parent = nullptr);
    ~CEqualizerFrame();
    void init(CEqualizer* EQ, int BandIndex, int FqMin, int FqMax, int FqDefault);
    void serialize(QDomLiteElement* xml) const;
    void unserialize(const QDomLiteElement* xml);
    void reset();
    void peak(const float val);
private:
    Ui::CEqualizerFrame *ui;
    CEqualizer* m_Device;
    int Index;
private slots:
    void VolChanged(int Value);
    void FreqChanged(int Freq);
protected:
    void showEvent(QShowEvent *);
};

#endif // CEQUALIZERFRAME_H
