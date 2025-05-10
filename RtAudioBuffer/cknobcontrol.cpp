#include "cknobcontrol.h"
#include "ui_cknobcontrol.h"

CKnobControl::CKnobControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CKnobControl)
{
    ui->setupUi(this);
    popup=new QSignalMenu(this);
    QFont f=popup->font();
    f.setPointSize(10);
    popup->setFont(f);
    spinbox=new QDoubleSpinBox(this);
    spinboxAction=new QWidgetAction(this);
    spinboxAction->setDefaultWidget(spinbox);
    ui->verticalSlider->setVisible(false);
    ui->pushButton->setVisible(false);
    ui->pushButton->setZoom(0.8);
    m_KnobType = Knob;
    connect(ui->dial,&QAbstractSlider::valueChanged,this,&CKnobControl::valueChanged);
    connect(ui->verticalSlider,&QAbstractSlider::valueChanged,this,&CKnobControl::valueChanged);
    connect(ui->pushButton,&QSynthCheckbox::valueChanged,this,&CKnobControl::valueChanged);
    ui->dial->setNotchStyle(QSynthKnob::LEDNotch);
    ui->verticalSlider->setLEDTicks(true);
    connect(popup,qOverload<int>(&QSignalMenu::menuClicked),ui->dial,&QAbstractSlider::setValue);
    connect(popup,qOverload<int>(&QSignalMenu::menuClicked),ui->verticalSlider,&QSynthSwitch::setValue);
    connect(popup,qOverload<int>(&QSignalMenu::menuClicked),ui->pushButton,&QSynthCheckbox::setValue);
    ui->label->setEffect(EffectLabel::Raised);
    ui->label->setShadowColor(QColor(255,255,255,200));
    ui->label->setTextColor(QColor(0,0,0,200));
}

CKnobControl::~CKnobControl()
{
    delete ui;
}

int CKnobControl::value()
{
    if (m_KnobType==Switch) return  ui->verticalSlider->value();
    if (m_KnobType==Checkbox) return ui->pushButton->value();
    return ui->dial->value();
}

void CKnobControl::setValue(CParameter* p)
{
    //QMutexLocker locker(&mutex);
    m_Parameter=p;
    KnobType t = Knob;
    if (p->Type == CParameter::SelectBox)
    {
        if (p->Max < 4) t = Switch;
        if (p->Max < 2) t = Checkbox;
    }
    if (t != m_KnobType)
    {
        m_KnobType = t;
        ui->verticalSlider->setVisible(m_KnobType==Switch);
        ui->dial->setVisible(m_KnobType==Knob);
        ui->pushButton->setVisible(m_KnobType==Checkbox);
    }
    if (m_KnobType==Switch)
    {
        ui->verticalSlider->blockSignals(true);
        ui->verticalSlider->setStringList(p->stringList());
        ui->verticalSlider->setValue(p->Value);
        ui->verticalSlider->blockSignals(false);
    }
    else if (m_KnobType==Checkbox)
    {
        ui->pushButton->blockSignals(true);
        ui->pushButton->setStateList(p->stringList());
        ui->pushButton->setValue(p->Value);
        ui->pushButton->blockSignals(false);
    }
    else
    {
        ui->dial->blockSignals(true);
        ui->dial->setMinimum(p->Min);
        ui->dial->setMaximum(p->Max);
        ui->dial->setValue(p->Value);
        ui->dial->blockSignals(false);
    }
    setLabels(p);
}

void CKnobControl::setLabels(CParameter* p)
{
    //QMutexLocker locker(&mutex);
    ui->label->blockSignals(true);
    ui->label_2->blockSignals(true);
    ui->label->setText(p->Name);
    ui->label_2->setText(p->valueText());
    ui->label->blockSignals(false);
    ui->label_2->blockSignals(false);
    /*
    if (p->Type==CParameter::dB)
    {
        ui->label_2->setText(QString::number(p->dBValue(),'f',2)+" "+p->Unit);
    }
    else if (p->Type==CParameter::SelectBox)
    {
        const QStringList l=p->stringList();
        ui->label_2->setText(l[p->Value]);
    }
    else
    {
        ui->label_2->setText(QString::number(p->decimalValue(),'f',QString::number(p->DecimalFactor).length()-1)+" "+p->Unit);
    }
    */
}

void CKnobControl::mousePressEvent(QMouseEvent *event)
{
    if (event->button()==Qt::RightButton)
    {
        disconnect(spinbox);
        spinbox->setAttribute(Qt::WA_MacShowFocusRect, false);
        popup->removeAction(spinboxAction);
        popup->clear();
        QAction* automationAction = new QAction("Automation");
        connect(automationAction,&QAction::triggered,this,&CKnobControl::sendAutomationRequest);
        if (m_Parameter->Type==CParameter::SelectBox)
        {
            QStringList l=m_Parameter->stringList();
            for (int i=0;i<l.size();i++)
            {
                QAction* a=popup->addAction(l[i],i);
                if (value()==i) popup->setActiveAction(a);
            }
            popup->addSeparator();
            popup->addAction(automationAction);
            popup->popup(mapToGlobal(event->pos()));
        }
        else if (m_Parameter->Type==CParameter::dB)
        {
            spinbox->setMinimum(lin2dB(m_Parameter->Min*0.01));
            spinbox->setMaximum(lin2dB(m_Parameter->Max*0.01));
            spinbox->setDecimals(2);
            spinbox->setValue(lin2dB(value()*0.01));
            spinbox->selectAll();
            popup->addAction(spinboxAction);
            connect(spinbox,qOverload<double>(&QDoubleSpinBox::valueChanged),this,&CKnobControl::SetdBValue);
            popup->addSeparator();
            popup->addAction(automationAction);
            popup->popup(mapToGlobal(event->pos()));
            spinbox->setFocus();
        }
        else
        {
            spinbox->setMinimum(m_Parameter->Min/double(m_Parameter->DecimalFactor));
            spinbox->setMaximum(m_Parameter->Max/double(m_Parameter->DecimalFactor));
            spinbox->setDecimals(QString::number(m_Parameter->DecimalFactor).length()-1);
            spinbox->setValue(value()/double(m_Parameter->DecimalFactor));
            spinbox->selectAll();
            popup->addAction(spinboxAction);
            connect(spinbox,qOverload<double>(&QDoubleSpinBox::valueChanged),this,&CKnobControl::SetNumericValue);
            popup->addSeparator();
            popup->addAction(automationAction);
            popup->popup(mapToGlobal(event->pos()));
            spinbox->setFocus();
        }
    }
}

void CKnobControl::SetNumericValue(double Value)
{
    ui->dial->setValue(int(Value*m_Parameter->DecimalFactor));
    spinbox->selectAll();
    spinbox->setFocus();
}

void CKnobControl::SetdBValue(double Value)
{
    ui->dial->setValue(qRound(dB2lin(Value)*100.0));
    spinbox->selectAll();
    spinbox->setFocus();
}
