#include "ckeylayoutcontrol.h"
#include "ui_ckeylayoutcontrol.h"
//#include "cwavefile.h"
#include <QFileDialog>

CKeyLayoutControl::CKeyLayoutControl(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CKeyLayoutControl)
{
    ui->setupUi(this);

    ui->LoopTypeCombo->addItems(QStringList() << "Forward" << "Alternate" << "X-fade");

    connect(ui->VolSpin,qOverload<int>(&QSpinBox::valueChanged),this,&CKeyLayoutControl::UpdateRangeGraph);
    connect(ui->TuneSpin,qOverload<int>(&QSpinBox::valueChanged),this,&CKeyLayoutControl::UpdateWaveGraph);
    connect(ui->XFadeCyclesSpin,qOverload<int>(&QSpinBox::valueChanged),this,&CKeyLayoutControl::UpdateWaveGraph);
    connect(ui->LoopTypeCombo,qOverload<int>(&QComboBox::currentIndexChanged),this,&CKeyLayoutControl::UpdateWaveGraph);

    connect(ui->LowKeyFullEdit,&CMIDINoteEdit::Changed,this,&CKeyLayoutControl::UpdateRangeGraph);
    connect(ui->HighKeyFullEdit,&CMIDINoteEdit::Changed,this,&CKeyLayoutControl::UpdateRangeGraph);
    connect(ui->LowKeyXEdit,&CMIDINoteEdit::Changed,this,&CKeyLayoutControl::UpdateRangeGraph);
    connect(ui->HighKeyXEdit,&CMIDINoteEdit::Changed,this,&CKeyLayoutControl::UpdateRangeGraph);
    connect(ui->MIDINoteEdit,&CMIDINoteEdit::Changed,this,&CKeyLayoutControl::UpdateWaveGraph);

    connect(ui->WaveEditWidget,&CWaveEditWidget::Changed,this,&CKeyLayoutControl::UpdateWaveControls);
    connect(ui->KeyLayout,&CKeyRangesControl::CurrentRangeChanged,this,&CKeyLayoutControl::UpdateRangeControls);
    connect(ui->KeyLayout,&CKeyRangesControl::RangeIndexChanged,this,&CKeyLayoutControl::SelectRange);
    connect(ui->KeyLayout,&CKeyRangesControl::WaveFileRequested,this,&CKeyLayoutControl::OpenFile);
    connect(ui->KeyLayout,&CKeyRangesControl::AddRangeRequested,this,&CKeyLayoutControl::AddRange);
    connect(ui->OpenButton,&QAbstractButton::clicked,this,&CKeyLayoutControl::OpenFile);
    connect(ui->DeleteButton,&QAbstractButton::clicked,this,&CKeyLayoutControl::DeleteRange);

    connect(ui->LoopTestButton,&QAbstractButton::toggled,this,&CKeyLayoutControl::ToggleLoopTest);
    connect(ui->TuneTestButton,&QAbstractButton::toggled,this,&CKeyLayoutControl::ToggleTuneTest);

    connect(ui->AutoLoopButton,&QAbstractButton::clicked,this,&CKeyLayoutControl::Autoloop);
    connect(ui->TuneLoopButton,&QAbstractButton::clicked,this,&CKeyLayoutControl::Autotune);
    connect(ui->PitchDetectButton,&QAbstractButton::clicked,this,&CKeyLayoutControl::Pitchdetect);
    connect(ui->FixRangeButton,&QAbstractButton::clicked,this,&CKeyLayoutControl::FixRange);


}

void CKeyLayoutControl::Init(CSamplerDevice* D)
{
    m_Sampler=D;
    ui->KeyLayout->Init(D);
    SelectRange(m_Sampler->currentRangeIndex);
}

CKeyLayoutControl::~CKeyLayoutControl()
{
    delete ui;
}

void CKeyLayoutControl::AddRange(int Upper, int Lower)
{
    m_Sampler->addRange(QString(),Upper,Lower);
    SelectRange(m_Sampler->rangeCount()-1);
    Update();
}

void CKeyLayoutControl::DoUpdateHost()
{
    emit Changed(false);
}

void CKeyLayoutControl::UpdateRangeGraph()
{
    CSampleKeyRange::RangeParams RP;
    RP.Volume=ui->VolSpin->value();
    if (ui->HighKeyFullEdit->value() > ui->HighKeyXEdit->value()) ui->HighKeyXEdit->setValue(ui->HighKeyFullEdit->value());
    if (ui->LowKeyFullEdit->value() < ui->LowKeyXEdit->value()) ui->LowKeyXEdit->setValue(ui->LowKeyFullEdit->value());
    RP.LowerTop=ui->LowKeyFullEdit->value()-1;
    RP.UpperTop=ui->HighKeyFullEdit->value();
    RP.LowerZero=ui->LowKeyXEdit->value()-1;
    RP.UpperZero=ui->HighKeyXEdit->value();

    m_Sampler->setRangeParams(RP);

    UpdateRangeControls(RP);

    ui->KeyLayout->Draw();
}

void CKeyLayoutControl::UpdateWaveGraph()
{
    CWaveGenerator::LoopParameters LP=m_Sampler->LoopParams();
    LP.MIDICents=ui->TuneSpin->value();
    LP.LoopType=(CWaveGenerator::LoopTypeEnum)ui->LoopTypeCombo->currentIndex();
    LP.XFade=ui->XFadeCyclesSpin->value();
    LP.MIDIKey=ui->MIDINoteEdit->value();
    m_Sampler->setLoopParams(LP);
}

void CKeyLayoutControl::UpdateWaveControls(CWaveGenerator::LoopParameters LP)
{
    for(QWidget* w : (const QList<QWidget*>)findChildren<QWidget*>()) w->blockSignals(true);
    ui->MIDINoteEdit->setValue(LP.MIDIKey);
    ui->TuneSpin->setValue(LP.MIDICents);
    ui->LoopTypeCombo->setCurrentIndex((int)LP.LoopType);
    ui->XFadeCyclesSpin->setValue(LP.XFade);
    m_Sampler->setLoopParams(LP);
    for(QWidget* w : (const QList<QWidget*>)findChildren<QWidget*>()) w->blockSignals(false);
}

void CKeyLayoutControl::UpdateRangeControls(CSampleKeyRange::RangeParams RP)
{
    for(QWidget* w : (const QList<QWidget*>)findChildren<QWidget*>()) w->blockSignals(true);
    ui->VolSpin->setValue(RP.Volume);

    //LowerTop
    ui->LowKeyFullEdit->setMaximum(RP.UpperTop);
    ui->LowKeyFullEdit->setValue(RP.LowerTop+1);
    //UpperTop
    ui->HighKeyFullEdit->setMinimum(RP.LowerTop+1);
    ui->HighKeyFullEdit->setValue(RP.UpperTop);
    //LowerZero
    ui->LowKeyXEdit->setMaximum(RP.LowerTop+1);
    ui->LowKeyXEdit->setValue(RP.LowerZero+1);
    //UpperZero
    ui->HighKeyXEdit->setMinimum(RP.UpperTop);
    ui->HighKeyXEdit->setValue(RP.UpperZero);

    m_Sampler->setRangeParams(RP);
    for(QWidget* w : (const QList<QWidget*>)findChildren<QWidget*>()) w->blockSignals(false);
}

void CKeyLayoutControl::Update()
{
    /*
    if (!SelectedRange)
    {
        InitSelected();
    }
    if (!SelectedRange)
    {
        ui->groupBox->setTitle("No Range");
        EnableEdit(false);
        WEInit(nullptr);
        return;
    }
    EnableEdit(true);
    */
    if (m_Sampler->rangeCount())
    {
        UpdateWaveControls(m_Sampler->LoopParams());
        UpdateRangeControls(m_Sampler->RangeParams());
        WEInit();
        UpdateRangeGraph();
    }
    ui->DeleteButton->setEnabled(m_Sampler->rangeCount() > 1);
}

void inline CKeyLayoutControl::WEInit()
{
    CSampleKeyRange* KR=m_Sampler->currentRange();
    ui->WaveEditWidget->Init(&KR->generator,KR->generator.LP,true);
    ui->FilenameEdit->blockSignals(true);
    ui->FilenameEdit->setText(KR->fileName);
    ui->FilenameEdit->blockSignals(false);
}

void CKeyLayoutControl::OpenFile()
{
    QString FN=QFileDialog::getOpenFileName(this,"Open",QStandardPaths::writableLocation(QStandardPaths::MusicLocation),WaveFile::WaveFilter);
    if (FN.isEmpty()) return;
    ReleaseLoop();
    if (!m_Sampler->rangeCount())
    {
        m_Sampler->addRange(FN,127,1);
        SelectRange(m_Sampler->rangeCount()-1);
        return;
    }
    m_Sampler->changePath(FN);
    Update();
}

void CKeyLayoutControl::SelectRange(int RangeIndex)
{
    ReleaseLoop();
    if (m_Sampler->rangeCount())
    {
        m_Sampler->currentRangeIndex=RangeIndex;
    }
    Update();
}

void CKeyLayoutControl::DeleteRange()
{
    ReleaseLoop();
    if (m_Sampler->rangeCount() > 1)
    {
        m_Sampler->removeRange();
        int R=m_Sampler->currentRangeIndex-1;
        if (R < 0) R=0;
        SelectRange(R);
    }
}

void CKeyLayoutControl::ReleaseLoop()
{
    ui->LoopTestButton->setChecked(false);
    ui->TuneTestButton->setChecked(false);
}

void CKeyLayoutControl::ToggleTuneTest(bool ButtonDown)
{
    if (ButtonDown)
    {
        m_Sampler->testMode=CSamplerDevice::st_TuneTest;
        m_Sampler->looping=false;
        ui->LoopTestButton->setEnabled(false);
    }
    else
    {
        m_Sampler->testMode=CSamplerDevice::st_NoTest;
        m_Sampler->looping=false;
        ui->LoopTestButton->setEnabled(true);
    }
}

void CKeyLayoutControl::ToggleLoopTest(bool ButtonDown)
{
    if (ButtonDown)
    {
        m_Sampler->testMode=CSamplerDevice::st_LoopTest;
        m_Sampler->looping=false;
        ui->TuneTestButton->setEnabled(false);
    }
    else
    {
        m_Sampler->testMode=CSamplerDevice::st_NoTest;
        m_Sampler->looping=false;
        ui->TuneTestButton->setEnabled(true);
    }
}

void CKeyLayoutControl::Autoloop()
{
    CSampleKeyRange* KR=m_Sampler->currentRange();
    KR->autoLoop(ui->LoopCyclesSpin->value());
    m_Sampler->setLoopParams(KR->generator.LP);
    Update();
}

void CKeyLayoutControl::Autotune()
{
    CSampleKeyRange* KR=m_Sampler->currentRange();
    KR->autoTune();
    m_Sampler->setLoopParams(KR->generator.LP);
    Update();
}

void CKeyLayoutControl::Pitchdetect()
{
    CSampleKeyRange* KR=m_Sampler->currentRange();
    KR->pitchDetect(ui->PitchDetectSpin->value());
    m_Sampler->setLoopParams(KR->generator.LP);
    Update();
}

void CKeyLayoutControl::FixRange()
{
    CSampleKeyRange* KR=m_Sampler->currentRange();
    KR->autoFix(ui->LoopCyclesSpin->value(),ui->PitchDetectSpin->value());
    m_Sampler->setLoopParams(KR->generator.LP);
    Update();
}

int CKeyLayoutControl::Tune_A440()
{
    return ui->PitchDetectSpin->value();
}

int CKeyLayoutControl::LoopCycles()
{
    return ui->LoopCyclesSpin->value();
}
