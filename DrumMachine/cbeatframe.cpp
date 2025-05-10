#include "cbeatframe.h"
#include "ui_cbeatframe.h"
#include <QDomLite>
#include "cpitchtextconvert.h"

CBeatFrame::CBeatFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CBeatFrame)
{
    ui->setupUi(this);
    m_TimerID=0;
    m_Beat=nullptr;
    m_SoundIndex=0;
    setAutoFillBackground(true);
    connect( ui->LenSlider,&QAbstractSlider::valueChanged,this,&CBeatFrame::LenChanged);
    connect(ui->PitchSlider,&QAbstractSlider::valueChanged,this,&CBeatFrame::PitchChanged);
    connect(ui->VolSlider,&QAbstractSlider::valueChanged,this,&CBeatFrame::VolChanged);
    connect(ui->PitchCombo,qOverload<int>(&QComboBox::currentIndexChanged),this,&CBeatFrame::PitchChanged);
    connect(this,&CBeatFrame::flashed,this,&CBeatFrame::timerStart);
}

CBeatFrame::~CBeatFrame()
{
    if (m_TimerID) killTimer(m_TimerID);
    m_TimerID=0;
    delete ui;
}

void CBeatFrame::LenChanged(int Value)
{
    m_Beat->Length[m_SoundIndex]=Value;
    UpdateBeat();
}
//---------------------------------------------------------------------------
void CBeatFrame::UpdateBeat()
{
    ui->LenProgress->setValue(m_Beat->Length[m_SoundIndex]);
}

void CBeatFrame::Init(BeatType* Beat,int Index,int SoundIndex,bool HideLength,bool HideVolume,bool HidePitch)
{
    m_SoundIndex=SoundIndex;
    m_Beat=Beat;
    ui->LenProgress->setVisible(!HideLength);
    ui->LenSlider->setVisible(!HideLength);
    ui->VolSlider->setVisible(!HideVolume);
    ui->VolProgress->setVisible(!HideVolume);
    ui->PitchCombo->setVisible(!HidePitch);
    ui->PitchSlider->setVisible(!HidePitch);
    ui->label->setText(QString::number(Index+1));
    ui->PitchCombo->blockSignals(true);
    ui->LenSlider->blockSignals(true);
    ui->VolSlider->blockSignals(true);
    ui->VolProgress->blockSignals(true);
    ui->LenProgress->blockSignals(true);
    ui->PitchCombo->setCurrentIndex(m_Beat->Pitch[m_SoundIndex]);
    ui->LenSlider->setValue(m_Beat->Length[m_SoundIndex]);
    ui->LenProgress->setValue(m_Beat->Length[m_SoundIndex]);
    ui->VolSlider->setValue(m_Beat->Volume[m_SoundIndex]);
    ui->PitchSlider->setValue(m_Beat->Pitch[m_SoundIndex]);
    ui->VolProgress->setValue(m_Beat->Volume[m_SoundIndex]);
    for (int i = 0; i < 128; i++) ui->PitchCombo->addItem(CPitchTextConvert::pitch2Text(i));
    ui->PitchCombo->blockSignals(false);
    ui->LenSlider->blockSignals(false);
    ui->VolSlider->blockSignals(false);
    ui->VolProgress->blockSignals(false);
    ui->LenProgress->blockSignals(false);
    //ui->PitchCombo->setStyleSheet("QComboBox::drop-down:!editable {border: 0px; subcontrol-origin: padding; subcontrol-position: center right; } ");
}

//---------------------------------------------------------------------------

void CBeatFrame::PitchChanged(int Value)
{
    m_Beat->Pitch[m_SoundIndex]=Value;
    ui->PitchCombo->blockSignals(true);
    ui->PitchCombo->setCurrentIndex(Value);
    ui->PitchCombo->blockSignals(false);
    ui->PitchSlider->blockSignals(true);
    ui->PitchSlider->setValue(Value);
    ui->PitchSlider->blockSignals(false);
}
//---------------------------------------------------------------------------


void CBeatFrame::VolChanged(int Value)
{
    m_Beat->Volume[m_SoundIndex]=Value;
    ui->VolProgress->setValue(Value);
}
//---------------------------------------------------------------------------
void CBeatFrame::Flash()
{
    QPalette p=ui->label->palette();
    p.setColor(QPalette::Window,Qt::yellow);
    ui->label->setPalette(p);
    update();
    emit flashed();
}

void CBeatFrame::timerStart()
{
    m_TimerID = startTimer(200);
}

void CBeatFrame::timerEvent(QTimerEvent *)
{
    if (!m_TimerID) return;
    QPalette p=ui->label->palette();
    p.setColor(QPalette::Window,Qt::transparent);
    ui->label->setPalette(p);
    update();
    killTimer(m_TimerID);
    m_TimerID=0;
}
