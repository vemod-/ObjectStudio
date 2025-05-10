#include "cmasterwidget.h"
#include "ui_cmasterwidget.h"
#include "effectlabel.h"

CMasterWidget::CMasterWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CMasterWidget)
{
    ui->setupUi(this);
    //ui->label->setEffect(EffectLabel::Raised);
    //ui->label->setShadowColor(Qt::white);
    connect(ui->MasterVol,&CMasterVol::leftVolChanged,this,&CMasterWidget::setVolL);
    connect(ui->MasterVol,&CMasterVol::rightVolChanged,this,&CMasterWidget::setVolR);

    m_Active = false;
    effectMenu=new QSignalMenu(this);
    showUIAction=effectMenu->addAction("Show UI","Show UI");
    for (const QString& s:CDeviceContainer::effectList()) effectMenu->addAction(s,s);
    unloadAction=effectMenu->addAction("Unload","Unload");
    connect(effectMenu,qOverload<QString>(&QSignalMenu::menuClicked),this,&CMasterWidget::selectEffect);
}

CMasterWidget::~CMasterWidget()
{
    delete ui;
}

void CMasterWidget::clear()
{
    m_Active = false;
}

void CMasterWidget::init(CStereoMixer *mx, QList<CDeviceContainer*>* effects)
{
    m_Mx=mx;
    m_Fx=effects;
    for (int i = dials.size(); i < int(m_Mx->sendCount()); i++)
    {
        auto d=new QSynthKnob(this);
        d->setFixedSize(40,40);
        //d->setKnobStyle(QSynthKnob::SimpleStyle);
        d->setMaximum(200);
        d->setValue(100);
        d->setNotchesVisible(true);
        d->setNotchStyle(QSynthKnob::dBNotch);
        dials.append(d);
        connect(d, &QSynthKnob::valueChanged, [=]() {effectVol(i);});
        ui->EffectLayout->addWidget(d,0,Qt::AlignHCenter);
        if (m_Fx==nullptr)
        {
            auto e=new EffectLabel(this);
            e->setEffect(EffectLabel::Raised);
            e->setShadowColor(QColor(255,255,255,200));
            e->setTextColor(QColor(0,0,0,200));
            e->setText("Effect "+QString::number(i+1));
            e->setMaximumHeight(13);
            setFontSizeScr(e,9);
            ui->EffectLayout->addWidget(e,0,Qt::AlignHCenter);
        }
        else
        {
            auto b=new QLCDLabel(this);
            m_Buttons.append(b);
            m_Names.append(m_Fx->at(i)->caption());
            b->setText("Effect "+QString::number(i+1));
            b->setFixedHeight(13);
            b->setAlignment(Qt::AlignHCenter);
            b->setMinimumWidth(this->width()-6);
            setFontSizeScr(b,9);
            if (i < m_Fx->size())
            {
                b->setText(m_Fx->at(i)->caption());
            }
            if (b->text().isEmpty()) b->setText("Effect "+QString::number(i+1));
            connect(b, &QLCDLabel::rightClicked, [=]() {showEffectMenu(i);});
            connect(b, &QLCDLabel::leftClicked, [=]() {showEffect(i);});
            ui->EffectLayout->addWidget(b,0,Qt::AlignHCenter);
        }
    }
    ui->MasterVol->setLock(true);
    ui->MasterVol->setLeftVol(100);
    for(QSynthKnob* d : std::as_const(dials)) d->setValue(100);
    m_Active = true;
}

void CMasterWidget::showEffect(int eff)
{
    currentEffect=m_Fx->at(eff);
    selectEffect("Show UI");
}

void CMasterWidget::showEffectMenu(int eff)
{
    showUIAction->setVisible(!m_Fx->at(eff)->deviceType().isEmpty());
    unloadAction->setVisible(!m_Fx->at(eff)->deviceType().isEmpty());
    currentEffect=m_Fx->at(eff);
    effectMenu->popup(ui->verticalFrame->mapToGlobal(m_Buttons.at(eff)->geometry().bottomLeft()));
}

void CMasterWidget::selectEffect(QString DeviceType)
{
    if (DeviceType=="Show UI")
    {
        currentEffect->execute(true);
        currentEffect->raiseForm();
        return;
    }
    if (currentEffect->deviceType()!=DeviceType)
    {
        currentEffect->setDeviceType(DeviceType);
    }
    currentEffect->execute(true);
}

void CMasterWidget::effectVol(int eff)
{
    m_Mx->Sends[eff]=dials[eff]->value()*0.01f;
}

void CMasterWidget::checkPeak()
{
    ui->MasterVol->peak(m_Mx->PeakL,m_Mx->PeakR);
    m_Mx->PeakL=0;
    m_Mx->PeakR=0;
}

void CMasterWidget::resetPeak()
{
    ui->MasterVol->resetPeak();
    m_Mx->PeakL=0;
    m_Mx->PeakR=0;
}

void CMasterWidget::setVolL(int vol)
{
    m_Mx->MasterLeft=vol*0.01f;
    ui->MasterVol->setLeftVol(vol);
    if (ui->MasterVol->lock()) m_Mx->MasterRight=vol*0.01f;
}

void CMasterWidget::setVolR(int vol)
{
    m_Mx->MasterRight=vol*0.01f;
    ui->MasterVol->setRightVol(vol);
    if (ui->MasterVol->lock()) m_Mx->MasterLeft=vol*0.01f;
}

void CMasterWidget::checkEffects()
{
    if (m_Active)
    {
        for (int i=0;i<m_Fx->size();i++)
        {
            QString s=m_Fx->at(i)->caption();
            if (s != m_Names[i])
            {
                m_Names[i]=s;
                m_Buttons[i]->setText(s);
                if (m_Buttons[i]->text().isEmpty()) m_Buttons[i]->setText("Effect "+QString::number(i+1));
            }
        }
    }
}

void CMasterWidget::setSoloChannel(int channel)
{
    m_Mx->SoloChannel=channel;
}

void CMasterWidget::serialize(QDomLiteElement* xml) const
{
    xml->setAttribute("Lock",ui->MasterVol->lock());
    xml->setAttribute("Volume Left",ui->MasterVol->leftVol());
    xml->setAttribute("Volume Right",ui->MasterVol->rightVol());
    for (int i=0;i<dials.size();i++) xml->setAttribute("Effect "+QString::number(i+1),dials[i]->value());
}

void CMasterWidget::unserialize(const QDomLiteElement* xml)
{
    if (!xml) return;
    ui->MasterVol->setLock(xml->attributeValueBool("Lock"));
    ui->MasterVol->setLeftVol(xml->attributeValueInt("Volume Left"));
    ui->MasterVol->setRightVol(xml->attributeValueInt("Volume Right"));
    for (int i=0;i<dials.size();i++)
    {
        dials[i]->setValue(xml->attributeValueInt("Effect "+QString::number(i+1),100));
    }
}

