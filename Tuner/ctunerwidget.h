#ifndef CTUNERWIDGET_H
#define CTUNERWIDGET_H

#include <QWidget>
#include <qcanvas.h>
#include "cpitchdetect.h"

namespace Ui {
class CTunerWidget;
}

class CTunerWidget : public QCanvas
{
    Q_OBJECT

public:
    explicit CTunerWidget(QWidget *parent = 0);
    ~CTunerWidget();
    void setTune(CPitchDetect::PitchRecord rec, double calib);
private:
    Ui::CTunerWidget *ui;
    CPitchDetect::PitchRecord r;
    double c;
    double cent2X(int cent);
};

#endif // CTUNERWIDGET_H
