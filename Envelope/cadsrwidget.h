#ifndef CADSRWIDGET_H
#define CADSRWIDGET_H

#include "cadsr.h"
#include <QFrame>

namespace Ui {
    class CADSRWidget;
}

class CADSRWidget : public QFrame
{
    Q_OBJECT

public:
    explicit CADSRWidget(QWidget *parent = 0);
    ~CADSRWidget();
    void Update(CADSR::ADSRParams ADSRParams);
private slots:
    void UpdateControls(CADSR::ADSRParams ADSRParams);
    void UpdateGraph();
public slots:
    void setDelay(int v);
    void setAttack(int v);
    void setHold(int v);
    void setDecay(int v);
    void setSustain(int v);
    void setRelease(int v);
    void emitChanged();
signals:
    void Changed(CADSR::ADSRParams ADSRParams);
    void DelayChanged(int v);
    void AttackChanged(int v);
    void HoldChanged(int v);
    void DecayChanged(int v);
    void SustainChanged(int v);
    void ReleaseChanged(int v);
private:
    Ui::CADSRWidget *ui;
    CADSR::ADSRParams AP;
};

#endif // CADSRWIDGET_H
