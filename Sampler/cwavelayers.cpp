#include "cwavelayers.h"
#include "ui_cwavelayers.h"

CWaveLayers::CWaveLayers(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CWaveLayers)
{
    ui->setupUi(this);
    connect(ui->VolSpin,qOverload<int>(&QSpinBox::valueChanged),this,&CWaveLayers::UpdateGraph);
    connect(ui->TransposeSpin,qOverload<int>(&QSpinBox::valueChanged),this,&CWaveLayers::UpdateGraph);
    connect(ui->TuneSpin,qOverload<int>(&QSpinBox::valueChanged),this,&CWaveLayers::UpdateGraph);
    connect(ui->LowVelFullSpin,qOverload<int>(&QSpinBox::valueChanged),this,&CWaveLayers::UpdateGraph);
    connect(ui->HighVelFullSpin,qOverload<int>(&QSpinBox::valueChanged),this,&CWaveLayers::UpdateGraph);
    connect(ui->LowVelXSpin,qOverload<int>(&QSpinBox::valueChanged),this,&CWaveLayers::UpdateGraph);
    connect(ui->HighVelXSpin,qOverload<int>(&QSpinBox::valueChanged),this,&CWaveLayers::UpdateGraph);
    connect(ui->ADSRWidget,&CADSRWidget::Changed,this,&CWaveLayers::UpdateADSRs);
    connect(ui->LayersControl,&CKeyLayersControl::CurrentLayerChanged,this,&CWaveLayers::UpdateControls);
    connect(ui->LayersControl,&CKeyLayersControl::LayerIndexChanged,this,&CWaveLayers::SelectLayer);
    connect(ui->LayersControl,&CKeyLayersControl::AddLayerRequested,this,&CWaveLayers::AddLayer);
    connect(ui->FixLayerButton,&QAbstractButton::clicked,this,&CWaveLayers::FixLayer);
    connect(ui->FixAllButton,&QAbstractButton::clicked,this,&CWaveLayers::FixAll);
    connect(ui->PitchLayerButton,&QAbstractButton::clicked,this,&CWaveLayers::PitchLayer);
    connect(ui->PitchAllButton,&QAbstractButton::clicked,this,&CWaveLayers::PitchAll);

    connect(ui->DeleteLayerButton,&QAbstractButton::clicked,this,&CWaveLayers::DeleteLayer);

    //MD=false;
    //Working=false;
    //Update();
}

CWaveLayers::~CWaveLayers()
{
    qDebug() << "Delete TWaveLayersControl";
    delete ui;
}

void CWaveLayers::Init(CSamplerDevice *Device)
{
    m_Sampler=Device;
    ui->ADSRWidget->Update(m_Sampler->ADSRParams());
    ui->LayersControl->Init(m_Sampler);
    ui->KeyLayoutControl->Init(m_Sampler);
    SelectLayer(m_Sampler->currentLayerIndex);
}

void CWaveLayers::SelectLayer(int LayerIndex)
{
    ReleaseLoop();
    m_Sampler->currentLayerIndex=LayerIndex;
    if (m_Sampler->currentRangeIndex >= m_Sampler->rangeCount()) m_Sampler->currentRangeIndex=m_Sampler->rangeCount()-1;
    Update();
}

void CWaveLayers::DeleteLayer()
{
    ReleaseLoop();
    if (m_Sampler->layerCount() > 1)
    {
        m_Sampler->removeLayer();
        SelectLayer(qMax<int>(m_Sampler->currentLayerIndex-1,0));
    }
}

void CWaveLayers::UpdateGraph()
{
    CLayer::LayerParams LP;
    LP.Volume=ui->VolSpin->value();
    if (ui->HighVelFullSpin->value() > ui->HighVelXSpin->value()) ui->HighVelXSpin->setValue(ui->HighVelFullSpin->value());
    if (ui->LowVelFullSpin->value() < ui->LowVelXSpin->value()) ui->LowVelXSpin->setValue(ui->LowVelFullSpin->value());
    LP.LowerTop=ui->LowVelFullSpin->value();
    LP.UpperTop=ui->HighVelFullSpin->value();
    LP.LowerZero=ui->LowVelXSpin->value();
    LP.UpperZero=ui->HighVelXSpin->value();
    LP.Transpose=ui->TransposeSpin->value();
    LP.Tune=ui->TuneSpin->value();
    m_Sampler->setLayerParams(LP);
    setControlBounds(LP);
    ui->LayersControl->Draw();
}

void CWaveLayers::UpdateControls(CLayer::LayerParams LP)
{
    setControlBounds(LP);
    qDebug() << LP.Tune;
    for(QWidget* w : (const QList<QWidget*>)findChildren<QWidget*>()) w->blockSignals(true);
    ui->VolSpin->setValue(LP.Volume);
    ui->LowVelFullSpin->setValue(LP.LowerTop);
    ui->HighVelFullSpin->setValue(LP.UpperTop);
    ui->LowVelXSpin->setValue(LP.LowerZero);
    ui->HighVelXSpin->setValue(LP.UpperZero);
    ui->TransposeSpin->setValue(LP.Transpose);
    ui->TuneSpin->setValue(LP.Tune);
    for(QWidget* w : (const QList<QWidget*>)findChildren<QWidget*>()) w->blockSignals(false);
    m_Sampler->setLayerParams(LP);
}

void CWaveLayers::setControlBounds(CLayer::LayerParams LP)
{
    for(QWidget* w : (const QList<QWidget*>)findChildren<QWidget*>()) w->blockSignals(true);
    ui->LowVelFullSpin->setMaximum(LP.UpperTop);
    ui->HighVelFullSpin->setMinimum(LP.LowerTop);
    ui->LowVelXSpin->setMaximum(LP.LowerTop);
    ui->HighVelXSpin->setMinimum(LP.UpperTop);
    for(QWidget* w : (const QList<QWidget*>)findChildren<QWidget*>()) w->blockSignals(false);
}

void CWaveLayers::Update()
{
    ui->ADSRWidget->Update(m_Sampler->ADSRParams());
    ui->LayersControl->Draw();
    UpdateControls(m_Sampler->LayerParams());
    ui->KeyLayoutControl->Update();
    ui->DeleteLayerButton->setEnabled(m_Sampler->layerCount() > 1);
}

void CWaveLayers::UpdateADSRs(CADSR::ADSRParams ADSRParams)
{
    m_Sampler->setADSRParams(ADSRParams);
}

void CWaveLayers::AddLayer(int Upper, int Lower)
{
    m_Sampler->addLayer(Upper,Lower);
    m_Sampler->currentLayerIndex=m_Sampler->layerCount()-1;
    m_Sampler->currentRangeIndex=0;
    Update();
}

void CWaveLayers::ReleaseLoop()
{
    ui->KeyLayoutControl->ReleaseLoop();
}

void CWaveLayers::FixLayer()
{
    for (int i=0;i<m_Sampler->rangeCount();i++)
    {
        CSampleKeyRange* KR=m_Sampler->range(m_Sampler->currentLayerIndex,i);
        KR->autoFix(ui->KeyLayoutControl->LoopCycles(),ui->KeyLayoutControl->Tune_A440());
        m_Sampler->setLoopParams(KR->generator.LP,m_Sampler->currentLayerIndex,i);
    }
    Update();
}

void CWaveLayers::PitchLayer()
{
    for (int i=0;i<m_Sampler->rangeCount();i++)
    {
        CSampleKeyRange* KR=m_Sampler->range(m_Sampler->currentLayerIndex,i);
        KR->pitchDetect(ui->KeyLayoutControl->Tune_A440());
        m_Sampler->setLoopParams(KR->generator.LP,m_Sampler->currentLayerIndex,i);
    }
    Update();
}

void CWaveLayers::FixAll()
{
    for (int l=0;l<m_Sampler->layerCount();l++)
    {
        for (int i=0;i<m_Sampler->rangeCount(l);i++)
        {
            CSampleKeyRange* KR=m_Sampler->range(l,i);
            KR->autoFix(ui->KeyLayoutControl->LoopCycles(),ui->KeyLayoutControl->Tune_A440());
            m_Sampler->setLoopParams(KR->generator.LP,l,i);
        }
    }
    Update();
}

void CWaveLayers::PitchAll()
{
    for (int l=0;l<m_Sampler->layerCount();l++)
    {
        for (int i=0;i<m_Sampler->rangeCount(l);i++)
        {
            CSampleKeyRange* KR=m_Sampler->range(l,i);
            KR->pitchDetect(ui->KeyLayoutControl->Tune_A440());
            m_Sampler->setLoopParams(KR->generator.LP,l,i);
        }
    }
    Update();
}
