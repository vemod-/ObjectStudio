#include "cequalizerframe.h"
#include "ui_cequalizerframe.h"

CEqualizerFrame::CEqualizerFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CEqualizerFrame)
{
    ui->setupUi(this);
    connect(ui->VolSlider,&QAbstractSlider::valueChanged,this,&CEqualizerFrame::VolChanged);
    connect(ui->dial,&QAbstractSlider::valueChanged,this,&CEqualizerFrame::FreqChanged);
    m_Device=nullptr;
    Index=0;
}

CEqualizerFrame::~CEqualizerFrame()
{
    delete ui;
}

void CEqualizerFrame::init(CEqualizer *EQ, int BandIndex, int FqMin, int FqMax, int FqDefault)
{
    m_Device=EQ;
    Index=BandIndex;
    m_Device->Freq[Index]=FqDefault;
    m_Device->Level[Index]=0;
    ui->FreqLabel->setText("Freq.");
    ui->dial->setMinimum(FqMin);
    ui->dial->setMaximum(FqMax);
    ui->dial->setValue(FqDefault);
    ui->VolSlider->setMaximum(200);
    ui->Peak->setMax(200);
    ui->dBScale->setMax(200);
    ui->VolSlider->setValue(100);
    VolChanged(100);
    ui->IndexLabel->setText("Band " + QString::number(Index+1));
}

void CEqualizerFrame::VolChanged(int Value)
{
    if (!m_Device) return;
    const float dB=lin2dBf(Value*0.01f);
    m_Device->SetLevel(Index,dB);
    ui->VolLabel->setText(QString::number(dB,'f',2)+"dB");
}

void CEqualizerFrame::FreqChanged(int Freq)
{
    if (!m_Device) return;
    m_Device->SetFreq(Index,Freq);
    ui->ValueLabel->setText(QString::number(Freq)+"Hz");
}

void CEqualizerFrame::serialize(QDomLiteElement* xml) const
{
    xml->setAttribute("Volume",ui->VolSlider->value());
    xml->setAttribute("Frequency",ui->dial->value());
}

void CEqualizerFrame::unserialize(const QDomLiteElement* xml)
{
    if (!xml) return;
    ui->VolSlider->setValue(xml->attributeValueInt("Volume"));
    ui->dial->setValue(xml->attributeValueInt("Frequency"));
    m_Device->SetLevel(Index,lin2db(ui->VolSlider->value()*0.01));
    m_Device->SetFreq(Index,ui->dial->value());
}

void CEqualizerFrame::showEvent(QShowEvent *)
{
    ui->dBScale->setMargin(ui->VolSlider->grooveMargin());
    ui->Peak->setMargin(ui->VolSlider->grooveMargin());
}

void CEqualizerFrame::reset()
{
    ui->Peak->reset();
}

void CEqualizerFrame::peak(const float val)
{
    ui->Peak->setValue(val);
}
