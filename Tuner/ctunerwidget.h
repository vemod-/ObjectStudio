#ifndef CTUNERWIDGET_H
#define CTUNERWIDGET_H

#include <QWidget>
#include "YinPitchDetector.h"

namespace Ui {
class CTunerWidget;
}
/*
class CTunerWidget : public QCanvas
{
    Q_OBJECT

public:
    explicit CTunerWidget(QWidget *parent = 0);
    ~CTunerWidget();
    void setTune(CYIN::PitchRecord rec, double calib);
private:
    Ui::CTunerWidget *ui;
    CYIN::PitchRecord r;
    double c;
    double cent2X(int cent);
};
*/
class CTunerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CTunerWidget(QWidget *parent = nullptr);

    void setPitchRecord(const CYIN::PitchRecord& rec);
    void setCalib(double calib);
protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
private:
    CYIN::PitchRecord m_rec{};
    double m_calib = 440;

    CYIN::PitchRecord m_lastDrawnRec{};
    double m_lastDrawnCalib = 0;

    QString m_pitchText;
    QString m_noteText;
    QString m_calibText;

    QFont m_bigFont;
    QFont m_smallFont;

    QRect indRect;
    QRect freqRect;
    QRect noteRect;
    QRect calibRect;

    int cent2X(int cent) const;
    int xc;
};

#endif // CTUNERWIDGET_H
