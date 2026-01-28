#include "cparameterscomponent.h"
#include "ui_cparameterscomponent.h"
#include <QInputDialog>
//#include <QMessageBox>
#include <QClipboard>
#include "cparametersmenu.h"
#include "qdprpixmap.h"

CParametersComponent::CParametersComponent(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CParametersComponent)
{
    ui->setupUi(this);
    m_Device = nullptr;
    Spacer=new QWidget(this);
    Spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->NameLabel->setEffect(EffectLabel::Raised);
    ui->NameLabel->setTextColor(QColor(0,0,0,200));
    ui->NameLabel->setShadowColor(QColor(255,255,255,200));
    ui->DialsFrame->m_Device = &m_Device;
    ui->DialsFrame->Dials = &Dials;
    m_Width=0;
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
        if (m_Device->alias().isEmpty()) {
            ui->NameLabel->setText(m_Device->name());
        }
        else {
            ui->NameLabel->setText(m_Device->alias() + "\n" + m_Device->name());
        }
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
    if (m_Device)
    {
        for (int i=0;i<m_Device->parameterCount();i++) Dials.at(i)->setValue(m_Device->parameter(i));
        ui->PresetLabel->setText(m_Device->currentProgramMatches());
    }
}

void CParametersComponent::updateControl(const CParameter* Parameter)
{
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
    ui->PresetLabel->clear();
    if (m_Device)
    {
        if (m_Device->alias().isEmpty()) {
            ui->NameLabel->setText(m_Device->name());
        }
        else {
            ui->NameLabel->setText(m_Device->alias() + "\n" + m_Device->name());
        }
        if (m_Device->parameterCount() != Dials.size()) {
            for (int i = Dials.size(); i >= 0; i--) {
                delete ui->horizontalLayout_2->takeAt(ui->horizontalLayout_2->count()-1);
            }
            Parameters.clear();
            qDeleteAll(Dials);
            Dials.clear();
            init(m_Device);
        }
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
            }
        }
        ui->PresetLabel->setText(m_Device->currentProgramMatches());
    }
}

QPixmap CParametersComponent::grabPanel() {
    return QDPRPixmap(ui->DialsFrame->grab());
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
}

void CParametersComponent::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    ui->DialsFrame->setFixedWidth(qMax<int>(width(),m_Width));
    ui->DialsFrame->move(0,0);
}

CParametersPanel::CParametersPanel(QWidget *parent) : QSynthPanel(parent) {

}

void CParametersPanel::paintEvent(QPaintEvent *e) {
    static QDPRPixmap screwPix(QSize(12,12),":/screwhead.png");
    QSynthPanel::paintEvent(e);
    QPainter p(this);
    p.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing | QPainter::TextAntialiasing);
    QFont f(p.font()); f.setPointSizeF(10); p.setFont(f);
    QFontMetricsF fm(p.font());
    p.drawPixmap(QRect(3,geometry().top() + 3,12,12),screwPix);
    p.drawPixmap(QRect(3,geometry().top() + 97,12,12),screwPix);
    if (!Dials->empty())
    {
        for (int i = 0; i < (*m_Device)->parameterGroupCount(); i++)
        {
            const CParameterGroup* g = (*m_Device)->parameterGroup(i);
            QColor c = g->color;
            c.setAlphaF(0.2);
            p.setBrush(c);
            p.setPen(QColor(0,0,0,40));
            QRect r1 = Dials->at(g->startIndex)->geometry().adjusted(1,-6,-2,6);
            QRect r2 = (g->endIndex > -1) ? Dials->at(g->endIndex)->geometry().adjusted(1,-6,-2,6) : Dials->last()->geometry().adjusted(1,-6,-2,6);
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
    const bool isFirst = (parentWidget()->mapToParent(QPoint(0,0)).y() == 0);
    for (int i = 100; i > 0; i = i - 10) {
        p.setPen(QColor(0,0,0,i));
        if (isFirst) {
            p.drawLine(x,x,x,geometry().height());
            p.drawLine(x,x,geometry().width(),x);
        }
        else {
            p.drawLine(x,0,x,geometry().height());
        }
        x++;
    }
}
