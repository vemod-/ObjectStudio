#ifndef CLANEEDIWIDGET_H
#define CLANEEDIWIDGET_H

#include <QWidget>
#include "ui_claneediwidget.h"

namespace Ui {
class CLaneEdiWidget;
}

class CLaneEdiWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CLaneEdiWidget(QWidget *parent = nullptr);
    ~CLaneEdiWidget();
    void init(CWaveLane* l, QList<int>* currentTrack, ulong64 currentSample,QDomLiteElement* timelineXML,QList<QAction*> al) {
        ui->LaneEdit->init(l, currentTrack, currentSample, timelineXML, al);
        connect(ui->LaneEdit,&CLaneEditControl::Changed,this,&CLaneEdiWidget::Changed,Qt::DirectConnection);
    }
signals:
    void Changed();
private:
    Ui::CLaneEdiWidget *ui;
};

#endif // CLANEEDIWIDGET_H
