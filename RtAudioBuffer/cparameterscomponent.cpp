#include "cparameterscomponent.h"
#include "ui_cparameterscomponent.h"
#include "mouseevents.h"

CParametersComponent::CParametersComponent(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CParametersComponent)
{
    ui->setupUi(this);
    mapper=new QSignalMapper(this);
    connect(mapper,SIGNAL(mapped(int)),this,SLOT(ValueChanged(int)));
    Spacer=new QWidget(this);
    ImgLabel=new QLabel(this);
    ImgLabel->setFixedSize(120,90);
    ImgLabel->setAlignment(Qt::AlignCenter);
    Spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->horizontalLayout_2->addWidget(ImgLabel);
    ui->horizontalLayout_2->addWidget(Spacer);
    ui->label->setEffect(EffectLabel::Raised);
    ui->label->setShadowColor(Qt::white);
    MouseEvents* e=new MouseEvents();
    ImgLabel->installEventFilter(e);
    connect(e,SIGNAL(MousePress(QMouseEvent*)),this,SLOT(showUI()));
}

CParametersComponent::~CParametersComponent()
{
    disconnect(mapper);
    delete ui;
}

void CParametersComponent::ShowParameters(IDevice *Device,QString Title)
{
    setUpdatesEnabled(false);
    ui->DialsFrame->hide();
    foreach (CKnobControl* w,Dials) w->hide();
    //ui->DialsFrame->show();
    Parameters.clear();
    ui->horizontalLayout_2->removeWidget(Spacer);
    m_D=Device;
    ImgLabel->hide();
    if (Device != NULL)
    {
        for (int i=0;i<Device->ParameterCount();i++)
        {
            const ParameterType p=Device->Parameter(i);
            Parameters.append(p);
            CKnobControl* d;
            if (i>Dials.count()-1)
            {
                d=new CKnobControl(this);
                ui->horizontalLayout_2->addWidget(d);
                connect(d,SIGNAL(valueChanged(int)),mapper,SLOT(map()));
                mapper->setMapping(d,i);
                Dials.append(d);
            }
            else
            {
                d=Dials.at(i);
            }
            d->setValue(Device->GetParameterValue(i),p);
            d->show();
        }
        ui->horizontalLayout_2->addWidget(Spacer);
        //ui->DialsFrame->show();
        if (Device->HasUI())
        {
            QPixmap* px=(QPixmap*)Device->Picture();
            if (px)
            {
                ImgLabel->setPixmap(px->scaled(ImgLabel->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
                ImgLabel->show();
                delete px;
            }
        }
    }
    ui->DialsFrame->show();
    setUpdatesEnabled(true);
    //ui->DialsFrame->repaint();
    ui->label->setText(Title);
}

void CParametersComponent::ValueChanged(int i)
{
        const ParameterType p=Parameters.at(i);
        int v=Dials.at(i)->value();
        Dials.at(i)->setLabels(v,p);
        m_D->SetParameterValue(i,v);
        emit ParameterChanged(m_D,i,v);
}

void CParametersComponent::mousePressEvent(QMouseEvent *event)
{
    if (event->button()==Qt::RightButton)
    {
        emit Popup(mapToGlobal(event->pos()));
    }
}

void CParametersComponent::showUI()
{
    if (m_D) m_D->Execute(true);
}
