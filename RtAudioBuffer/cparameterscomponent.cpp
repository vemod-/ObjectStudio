#include "cparameterscomponent.h"
#include "ui_cparameterscomponent.h"
#include <QInputDialog>
//#include <QMessageBox>
#include <QClipboard>
#include "cparametersmenu.h"

CParametersComponent::CParametersComponent(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CParametersComponent)
{
    ui->setupUi(this);
    m_Device=nullptr;
    Spacer=new QWidget(this);
    Spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->NameLabel->setEffect(EffectLabel::Raised);
    ui->NameLabel->setTextColor(QColor(0,0,0,200));
    ui->NameLabel->setShadowColor(QColor(255,255,255,200));
    //auto e=new MouseEvents();
    //ui->UILabel->installEventFilter(e);
    //connect(e,&MouseEvents::MousePressed,this,&CParametersComponent::showUI);
    ui->DialsFrame->installEventFilter(this);
    m_Width=0;
    /*
    ParameterPresetsMenu=new QSignalMenu("Load",this);
    connect(ParameterPresetsMenu,qOverload<QString>(&QSignalMenu::menuClicked),this,&CParametersComponent::OpenPreset);
    ParametersMenu=new QMenu("Parameters",this);
    ParametersMenu->addMenu(ParameterPresetsMenu);
    ParametersMenu->addAction("Save as Preset",this,&CParametersComponent::SavePresetAs);
    ParametersMenu->addAction("Copy Parameters",this,&CParametersComponent::CopyParameters);
    actionPasteParameters = ParametersMenu->addAction("Paste Parameters",this,&CParametersComponent::PasteParameters);
    AutomationAction = ParametersMenu->addAction("Automation",this,&CParametersComponent::Automation);
*/
}

CParametersComponent::~CParametersComponent()
{
    delete ui;
}

QString CParametersComponent::deviceID()
{
    if (m_Device) return m_Device->deviceID();
    return QString();
}

void CParametersComponent::init(IDevice* Device)
{
    m_Device=Device;
    setUpdatesEnabled(false);
    ui->DialsFrame->hide();
    ui->LCDWidget->setVisible(false);
    m_Width=160;
    if (Device != nullptr)
    {
        for (int i=0;i<Device->parameterCount();i++)
        {
            Parameters.append(Device->parameter(i));
            auto d=new CKnobControl(this);
            ui->horizontalLayout_2->addWidget(d);
            connect(d, &CKnobControl::valueChanged, [=] { updateParameterValue(i); });
            connect(d,&CKnobControl::requestAutomation,this,&CParametersComponent::showAutomation);
            Dials.append(d);
            d->show();
            m_Width+=d->width();
        }
        ui->horizontalLayout_2->addWidget(Spacer);
        ui->NameLabel->setText(m_Device->name());
        ui->IDLabel->setText(m_Device->deviceID());

        if (Device->hasUI()) {
            ui->LCDWidget->setVisible(true);
            m_Width += ui->LCDWidget->width();
        }
    }
    ui->DialsFrame->setFixedWidth(qMax<int>(width(),m_Width));
    ui->DialsFrame->show();
    setUpdatesEnabled(true);
}

void CParametersComponent::updateControls()
{
    //QMutexLocker locker(&mutex);
    if (m_Device)
    {
        for (int i=0;i<m_Device->parameterCount();i++) Dials.at(i)->setValue(m_Device->parameter(i));
        ui->PresetLabel->setText(m_Device->currentProgramMatches());
    }
}

void CParametersComponent::updateControl(const CParameter* Parameter)
{
    //QMutexLocker locker(&mutex);
    if (m_Device)
    {
        for (int i=0;i<m_Device->parameterCount();i++) {
            if (m_Device->parameter(i) == Parameter) Dials.at(i)->setValue(m_Device->parameter(i));
        }
        ui->PresetLabel->setText(m_Device->currentProgramMatches());
    }
}

void CParametersComponent::showParameters()
{
    qDebug() << "CParametersComåponent showParameters";
    ui->UILabel->clear();
    //ui->LCDWidget->setVisible(false);
    ui->PresetLabel->clear();
    if (m_Device)
    {        
        for (int i=0;i<m_Device->parameterCount();i++) Dials.at(i)->setValue(m_Device->parameter(i));
        if (m_Device->hasUI())
        {
            const QPixmap* px=m_Device->picture();
            if (px)
            {
                //qDebug() << px->size() << px->scaled(ui->UILabel->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation).size();
                QPixmap pm(*px);//->scaled(ui->UILabel->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
                pm.setDevicePixelRatio(1);
                ui->UILabel->setPixmap(pm);
                delete px;
                //repaint();
            }
        }
        ui->PresetLabel->setText(m_Device->currentProgramMatches());
    }
}

void CParametersComponent::updateParameterValue(int i)
{
    const int v=Dials[i]->value();
    Parameters[i]->setValue(v);
    Dials.at(i)->setLabels(Parameters[i]);
    ui->PresetLabel->setText(m_Device->currentProgramMatches());
}

void CParametersComponent::wheelEvent(QWheelEvent* event)
{
    const int move = event->pixelDelta().rx();
    if (move != 0)
    {
        if (m_Width > width())
        {
            int l = ui->DialsFrame->geometry().left()+move;
            if (l > 0) l = 0;
            if (l < width()-m_Width) l = width()-m_Width;
            ui->DialsFrame->move(l,0);
            event->accept();
            return;
        }
    }
    event->ignore();
}

void CParametersComponent::mousePressEvent(QMouseEvent *event)
{
    if (event->button()==Qt::RightButton) {
        parametersMenu()->popup(mapToGlobal(event->pos())); //emit popupTriggered(m_Device, mapToGlobal(event->pos()));
        return;
    }
    if ((ui->NameLabel->geometry().contains(event->pos())) || (ui->UILabel->geometry().contains(ui->UILabel->mapFrom(this,event->pos()))))
    {
        if (m_Device) m_Device->toggleUI();
        return;
    }
    if (event->button()==Qt::LeftButton) emit mousePress(m_Device, mapToGlobal(event->pos()));
}

QMenu* CParametersComponent::parametersMenu()
{
    CParametersMenu* m = new CParametersMenu(m_Device,this);
    m->setAttribute(Qt::WA_DeleteOnClose,true);
    connect(m,&CParametersMenu::showAutomationRequested,this,&CParametersComponent::showDefaultAutomation);
    connect(m,&CParametersMenu::aboutToChange,this,&CParametersComponent::aboutToChange,Qt::DirectConnection);
    connect(m,&CParametersMenu::parametersChanged,this,&CParametersComponent::parametersChanged);
    connect(m,&CParametersMenu::updateControls,this,&CParametersComponent::updateControls);
    return m;
/*
    actionPasteParameters->setEnabled(QApplication::clipboard()->text().startsWith("<Parameters"));
    ParametersMenu->setEnabled((m_Device->parameterCount() > 0) || (m_Device->hasUI()));
    AutomationAction->setEnabled(m_Device->parameterCount() > 0);
    fillParameterPresetsMenu();
    return ParametersMenu;
*/
}
/*
QAction* CParametersComponent::pasteParameters()
{
    CPasteParametersAction* a = new CPasteParametersAction(m_Device,this);
    connect(a,&CPasteParametersAction::updateControls,this,&CParametersComponent::updateControls);
    connect(a,&CPasteParametersAction::parametersChanged,this,&CParametersComponent::parametersChanged);
    return a;
}
*/
/*
void CParametersComponent::fillParameterPresetsMenu()
{
    ParameterPresetsMenu->clear();
    QString PresetName;
    if (m_Device != nullptr)
    {
        PresetName=m_Device->currentProgramMatches();
        const QStringList& l=m_Device->programNames();
        for (const QString& s : l)
        {
            QAction* a=ParameterPresetsMenu->addAction(s,s);
            a->setCheckable(true);
            if (s==PresetName) a->setChecked(true);
        }
    }
    if (ParameterPresetsMenu->isEmpty())
    {
        QAction* a=ParameterPresetsMenu->addAction("No Presets");
        a->setEnabled(false);
    }
}
*/
void CParametersComponent::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    ui->DialsFrame->setFixedWidth(qMax<int>(width(),m_Width));
    ui->DialsFrame->move(0,0);
}

bool CParametersComponent::eventFilter(QObject* obj, QEvent* event)
{
    bool retval = obj->eventFilter(obj,event);
    if (event->type()==QEvent::Paint)
    {
        QPainter p(ui->DialsFrame);
        p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        QFont f(p.font()); f.setPointSizeF(10); p.setFont(f);
        QFontMetricsF fm(p.font());
        QPixmap screwPix=QPixmap::fromImage(QImage(":/screwhead.png"), Qt::AutoColor | Qt::DiffuseDither | Qt::DiffuseAlphaDither).scaled(QSize(12,12),Qt::KeepAspectRatio,Qt::SmoothTransformation);
        p.drawPixmap(QRect(3,ui->DialsFrame->geometry().top() + 3,12,12),screwPix);
        p.drawPixmap(QRect(3,ui->DialsFrame->geometry().top() + 97,12,12),screwPix);
        if (!Dials.empty())
        {
            for (int i = 0; i < m_Device->parameterGroupCount(); i++)
            {
                const CParameterGroup* g = m_Device->parameterGroup(i);
                QColor c = g->color;
                c.setAlphaF(0.2);
                p.setBrush(c);
                p.setPen(QColor(0,0,0,40));
                QRect r1 = Dials.at(g->startIndex)->geometry().adjusted(1,-6,-2,6);
                QRect r2 = (g->endIndex > -1) ? Dials.at(g->endIndex)->geometry().adjusted(1,-6,-2,6) : Dials.last()->geometry().adjusted(1,-6,-2,6);
                QRect r = r1.united(r2);
                p.drawRoundedRect(r,5,5);
                if (!g->Name.isEmpty())
                {
                    p.setBrush(QColor(255,255,255,200));
                    p.setPen(QColor(255,255,255,200));
                    p.drawText(r.center().x()-(fm.boundingRect(g->Name).width()/2)-1,r.top()+(fm.height()/2)+1,g->Name);
                    p.setBrush(QColor(0,0,0,200));
                    p.setPen(QColor(0,0,0,200));
                    p.drawText(r.center().x()-(fm.boundingRect(g->Name).width()/2),r.top()+(fm.height()/2)+2,g->Name);
                }
            }
        }
        int x = 0;
        for (int i = 100; i > 0; i = i - 10) {
            p.setPen(QColor(0,0,0,i));
            if (this->geometry().top() == 0) {
                p.drawLine(x,x,x,ui->DialsFrame->geometry().height());
                p.drawLine(x,x,ui->DialsFrame->geometry().width(),x);
            }
            else {
                p.drawLine(x,0,x,ui->DialsFrame->geometry().height());
            }
            x++;
        }
    }
    return retval;
}

