#ifndef CWAVEEDITWIDGET_H
#define CWAVEEDITWIDGET_H

#include <QWidget>
#include "cwavegenerator.h"

namespace Ui {
    class CWaveEditWidget;
}

class CWaveEditWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CWaveEditWidget(QWidget *parent = 0);
    ~CWaveEditWidget();
    void Init(CWaveGenerator* WG,CWaveGenerator::LoopParameters LP,bool LoopOn, bool Enabled=true);
private slots:
    void UpdateGraph();
    void UpdateControls(CWaveGenerator::LoopParameters LP);
    //void SetScrollMax();
    void UpdateStretch(bool v);
signals:
    void Changed(CWaveGenerator::LoopParameters LP);
private:
    Ui::CWaveEditWidget *ui;
    CWaveGenerator* m_WG;
    bool m_LoopOn;
};

#endif // CWAVEEDITWIDGET_H
