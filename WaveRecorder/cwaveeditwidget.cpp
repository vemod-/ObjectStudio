#include "cwaveeditwidget.h"
#include "ui_cwaveeditwidget.h"

CWaveEditWidget::CWaveEditWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CWaveEditWidget)
{
    ui->setupUi(this);
    ui->ScrollBar->setVisible(false);
    connect(ui->StartSpin,qOverload<int>(&QSpinBox::valueChanged),this,&CWaveEditWidget::UpdateGraph);
    connect(ui->EndSpin,qOverload<int>(&QSpinBox::valueChanged),this,&CWaveEditWidget::UpdateGraph);
    connect(ui->FadeInSpin,qOverload<int>(&QSpinBox::valueChanged),this,&CWaveEditWidget::UpdateGraph);
    connect(ui->FadeOutSpin,qOverload<int>(&QSpinBox::valueChanged),this,&CWaveEditWidget::UpdateGraph);
    connect(ui->VolSpin,qOverload<int>(&QSpinBox::valueChanged),this,&CWaveEditWidget::UpdateGraph);
    connect(ui->SpeedSpin,qOverload<double>(&QDoubleSpinBox::valueChanged),this,&CWaveEditWidget::UpdateGraph);
    connect(ui->PitchSpin,qOverload<double>(&QDoubleSpinBox::valueChanged),this,&CWaveEditWidget::UpdateGraph);
    connect(ui->StretchCheck,&QAbstractButton::toggled,this,&CWaveEditWidget::UpdateStretch);

    //connect(ui->ScrollBar,&QAbstractSlider::valueChanged,ui->WaveEdit,&CWaveEditControl::setViewportLeft);

    connect(ui->ZoomInButton,&QAbstractButton::clicked,ui->WaveEdit,&CWaveEditControl::ZoomIn);
    connect(ui->ZoomOutButton,&QAbstractButton::clicked,ui->WaveEdit,&CWaveEditControl::ZoomOut);
    connect(ui->ZoomMinButton,&QAbstractButton::clicked,ui->WaveEdit,&CWaveEditControl::ZoomMin);
    connect(ui->ZoomMaxButton,&QAbstractButton::clicked,ui->WaveEdit,&CWaveEditControl::ZoomMax);
    //connect(ui->WaveEdit,&QViewportCanvas::paintRequested,this,&CWaveEditWidget::SetScrollMax);

    connect(ui->WaveEdit,&CWaveEditControl::ParameterChanged,this,&CWaveEditWidget::UpdateControls);

    ui->zoomLayout->replaceWidget(ui->ScrollBar,ui->WaveEdit->horizontalScrollBar());
}

CWaveEditWidget::~CWaveEditWidget()
{
    delete ui;
}

void CWaveEditWidget::Init(CWaveGenerator *WG, CWaveGenerator::LoopParameters LP, bool LoopOn, bool Enabled)
{
    m_WG=WG;
    m_LoopOn=LoopOn;
    ui->WaveEdit->Enabled=Enabled;
    for(QWidget* w : (const QList<QWidget*>)findChildren<QWidget*>()) w->setEnabled(Enabled);
    if (!Enabled) return;
    if (LP.End>WG->size())
    {
        LP.End=WG->size();
    }
    if (LP.Start>WG->size())
    {
        LP.Start=WG->size();
    }
    if (LP.LoopEnd>WG->size())
    {
        LP.LoopEnd=WG->size();
    }
    if (LP.LoopStart>WG->size())
    {
        LP.LoopStart=WG->size();
    }
    for(QWidget* w : (const QList<QWidget*>)findChildren<QWidget*>()) w->blockSignals(true);
    ui->VolSpin->setValue(LP.Volume);
    ui->SpeedSpin->setValue(LP.Speed*100.0);
    ui->PitchSpin->setValue(LP.PitchShift);
    ui->StartSpin->setMaximum(WG->size());
    ui->EndSpin->setMaximum(WG->size());
    ui->FadeInSpin->setMaximum(WG->size());
    ui->FadeOutSpin->setMaximum(WG->size());
    ui->StartSpin->setValue(LP.Start);
    ui->EndSpin->setValue(LP.End);
    if (LoopOn)
    {
        ui->FadeInSpin->setValue(LP.LoopStart);
        ui->FadeOutSpin->setValue(LP.LoopEnd);
        ui->FadeInLabel->setText("Loop Start");
        ui->FadeOutLabel->setText("Loop End");
        ui->VolSpin->setVisible(false);
        ui->PitchSpin->setVisible(false);
        ui->SpeedSpin->setVisible(false);
        ui->StretchCheck->setVisible(false);
        ui->VolLabel->setVisible(false);
        ui->PitchShiftLabel->setVisible(false);
    }
    else
    {
        ui->FadeInSpin->setValue(LP.FadeIn);
        ui->FadeOutSpin->setValue(LP.FadeOut);
        ui->VolSpin->setVisible(true);
        ui->VolLabel->setText("Volume");
    }
    ui->WaveEdit->Init(WG,LP,LoopOn);
    for(QWidget* w : (const QList<QWidget*>)findChildren<QWidget*>()) w->blockSignals(false);
}
/*
void CWaveEditWidget::SetScrollMax()
{
    ui->ScrollBar->blockSignals(true);
    if (ui->WaveEdit->sceneRect().width()>ui->WaveEdit->viewPortGeometry().width())
    {
        ui->ScrollBar->setMaximum(ui->WaveEdit->sceneRect().width()-ui->WaveEdit->viewPortGeometry().width());
    }
    else
    {
        ui->ScrollBar->setMaximum(0);
    }
    ui->ScrollBar->setSingleStep(1);
    ui->ScrollBar->setPageStep(ui->WaveEdit->width());
    ui->ScrollBar->setSliderPosition(ui->WaveEdit->viewPortGeometry().left());
    ui->ScrollBar->blockSignals(false);
}
*/
void CWaveEditWidget::UpdateGraph()
{
    CWaveGenerator::LoopParameters LP=m_WG->LP;
    LP.Volume=ui->VolSpin->value();
    LP.PitchShift=ui->PitchSpin->value();
    LP.Speed=ui->SpeedSpin->value()*0.01;
    if (ui->StretchCheck->isChecked())
    {
        LP.PitchShift = -factor2Cent(LP.Speed)*0.01;
        ui->PitchSpin->setValue(LP.PitchShift);
    }
    if (m_LoopOn)
    {
        LP.Start=ui->StartSpin->value();
        LP.End=ui->EndSpin->value();
        LP.LoopStart=ui->FadeInSpin->value();
        LP.LoopEnd=ui->FadeOutSpin->value();
    }
    else
    {
        ui->StartSpin->setMaximum(ui->EndSpin->value()-(ui->FadeInSpin->value()+ui->FadeOutSpin->value()));
        LP.Start=ui->StartSpin->value();
        ui->EndSpin->setMinimum(ui->StartSpin->value()+ui->FadeInSpin->value()+ui->FadeOutSpin->value());
        LP.End=ui->EndSpin->value();
        ui->FadeInSpin->setMaximum((ui->EndSpin->value()-ui->StartSpin->value())-ui->FadeOutSpin->value());
        LP.FadeIn=ui->FadeInSpin->value();
        ui->FadeOutSpin->setMaximum((ui->EndSpin->value()-ui->StartSpin->value())-ui->FadeInSpin->value());
        LP.FadeOut=ui->FadeOutSpin->value();
    }
    ui->WaveEdit->Draw(LP);
    emit Changed(LP);
}

void CWaveEditWidget::UpdateControls(CWaveGenerator::LoopParameters LP)
{
    for(QWidget* w : (const QList<QWidget*>)findChildren<QWidget*>()) w->blockSignals(true);
    ui->VolSpin->setValue(LP.Volume);
    //ui->SpeedSpin->setValue(LP.Speed*100.0);
    //ui->PitchSpin->setValue(LP.PitchShift);
    ui->StartSpin->setValue(LP.Start);
    ui->EndSpin->setValue(LP.End);
    if (m_LoopOn)
    {
        ui->FadeInSpin->setValue(LP.LoopStart);
        ui->FadeOutSpin->setValue(LP.LoopEnd);
    }
    else
    {
        ui->FadeInSpin->setValue(LP.FadeIn);
        ui->FadeOutSpin->setValue(LP.FadeOut);
    }
    for(QWidget* w : (const QList<QWidget*>)findChildren<QWidget*>()) w->blockSignals(false);
    emit Changed(LP);
}

void CWaveEditWidget::UpdateStretch(bool v)
{
    if (v) ui->PitchSpin->setValue(-factor2Cent(ui->SpeedSpin->value()*0.01)*0.01);
}
