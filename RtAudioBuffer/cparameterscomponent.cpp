#include "cparameterscomponent.h"
//#include "ui_cparameterscomponent.h"
#include <QInputDialog>
//#include <QMessageBox>
#include <QClipboard>
#include "cparametersmenu.h"
#include "qdprpixmap.h"

#define rackLeftWidth 68

CParametersComponent::CParametersComponent(QGraphicsScene* s)
{
    m_Device = nullptr;
    m_Scene = s;
    m_RackLeftPix = m_Scene->addPixmap(QDPRPixmap(QSize(rackUnitHeight,rackUnitHeight),":/RackLeft.png",Qt::KeepAspectRatio));
    m_NameLabel = new EffectLabel();
    m_UILabel = new QLCDLabel();
    m_UILabel->setFixedSize(130,91);
    m_UILabel->setFrameShadow(QFrame::Sunken);
    m_UILabel->setFrameShape(QFrame::Panel);
    m_UILabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_IDLabel = new QLCDLabel();
    m_IDLabel->setFixedSize(120,12);
    m_IDLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_PresetLabel = new QLCDLabel();
    m_PresetLabel->setFixedSize(120,12);
    m_PresetLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_NameLabel->setFixedSize(120,80);
    m_NameLabel->setEffect(EffectLabel::Raised);
    m_NameLabel->setTextColor(QColor(0,0,0,200));
    m_NameLabel->setShadowColor(QColor(255,255,255,200));
    m_NameLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    QFont f(m_NameLabel->font());
    f.setPointSize(15);
    m_NameLabel->setFont(f);
    m_Width=0;
    f = m_PresetLabel->font();
    f.setPointSize(9);
    m_PresetLabel->setFont(f);
    m_IDLabel->setFont(f);
    m_ProxyPresetLabel = m_Scene->addWidget(m_PresetLabel);
    m_ProxyIDLabel = m_Scene->addWidget(m_IDLabel);
    m_ProxyNameLabel = m_Scene->addWidget(m_NameLabel);
    m_ProxyUILabel = m_Scene->addWidget(m_UILabel);
}

CParametersComponent::~CParametersComponent()
{
    //m_Scene->clear();
    for (QGraphicsProxyWidget* w : std::as_const(m_ProxyDials)) {
        m_Scene->removeItem(w);
        delete w;
    }
    for (QGraphicsItem* w : std::as_const(m_GroupList)) {
        m_Scene->removeItem(w);
        delete w;
    }
    for (QGraphicsItem* w : std::as_const(m_FrameList)) {
        m_Scene->removeItem(w);
        delete w;
    }
    m_Scene->removeItem(m_ProxyNameLabel);
    delete m_ProxyNameLabel;
    m_Scene->removeItem(m_ProxyUILabel);
    delete m_ProxyUILabel;
    m_Scene->removeItem(m_ProxyIDLabel);
    delete m_ProxyIDLabel;
    m_Scene->removeItem(m_ProxyPresetLabel);
    delete m_ProxyPresetLabel;
}

QString CParametersComponent::deviceID()
{
    if (m_Device) return m_Device->deviceID();
    return QString();
}

void CParametersComponent::init(IDevice* Device)
{
    m_Device = Device;
    if (Device != nullptr)
    {
        Parameters.clear();
        for (QGraphicsProxyWidget* w : std::as_const(m_ProxyDials)) {
            m_Scene->removeItem(w);
            delete w;
        }
        m_ProxyDials.clear();
        Dials.clear();
        for (int i = 0; i < Device->parameterCount(); i++)
        {
            Parameters.append(Device->parameter(i));
            auto d = new CKnobControl();
            m_ProxyDials.append(m_Scene->addWidget(d));
            m_ProxyDials.last()->setZValue(1);
            connect(d, &CKnobControl::valueChanged, [=] { updateParameterValue(i); });
            connect(d,&CKnobControl::requestAutomation,this,&CParametersComponent::showAutomation);
            Dials.append(d);
            d->show();
        }
        if (m_Device->alias().isEmpty()) {
            m_NameLabel->setText(m_Device->name());
        }
        else {
            m_NameLabel->setText(m_Device->alias() + "\n" + m_Device->name());
        }
        m_IDLabel->setText(m_Device->deviceID());
    }
}

void CParametersComponent::updateControls()
{
    if (m_Device)
    {
        for (int i = 0;i < m_Device->parameterCount(); i++) Dials.at(i)->setValue(m_Device->parameter(i));
        m_PresetLabel->setText(m_Device->currentProgramMatches());
    }
}

void CParametersComponent::updateControl(const CParameter* Parameter)
{
    if (m_Device)
    {
        for (int i = 0;i < m_Device->parameterCount(); i++) {
            if (m_Device->parameter(i) == Parameter) Dials.at(i)->setValue(m_Device->parameter(i));
        }
        m_PresetLabel->setText(m_Device->currentProgramMatches());
    }
}

void CParametersComponent::showParameters(int index)
{
    static QDPRPixmap screwPix(QSize(10,10),":/screwhead.png");
    m_Index = index;
    qDebug() << "CParametersComåponent showParameters";
    for (QGraphicsItem* i : std::as_const(m_GroupList)) {
        m_Scene->removeItem(i);
        delete i;
    }
    m_GroupList.clear();
    m_Width = 172;
    m_UILabel->clear();
    m_PresetLabel->clear();
    m_RackLeftPix->setPos(0,calcY(0));
    m_ProxyPresetLabel->setPos(rackLeftWidth + 12,calcY(96));
    m_ProxyIDLabel->setPos(rackLeftWidth + 12,calcY(80));
    m_ProxyNameLabel->setPos(rackLeftWidth + 12,calcY(0));
    m_ProxyUILabel->setPos(rackLeftWidth + 136,calcY(12));
    if (m_Device)
    {
        if (m_Device->alias().isEmpty()) {
            m_NameLabel->setText(m_Device->name());
        }
        else {
            m_NameLabel->setText(m_Device->alias() + "\n" + m_Device->name());
        }
        if (m_Device->parameterCount() != Dials.size()) init(m_Device);
        for (int i = 0;i<m_Device->parameterCount();i++) Dials.at(i)->setValue(m_Device->parameter(i));
        int dialOffset = rackLeftWidth + m_NameLabel->width() + 16;

        m_ProxyUILabel->setVisible(false);
        if (m_Device->hasUI())
        {
            const QPixmap* px = m_Device->picture();
            if (px)
            {
                QPixmap pm(*px);
                m_UILabel->setPixmap(pm);
                delete px;
                dialOffset += m_UILabel->width() + 4;
                m_Width += m_UILabel->width();
                m_ProxyUILabel->setVisible(true);
            }
        }
        m_PresetLabel->setText(m_Device->currentProgramMatches());
        for (int i = 0; i < m_ProxyDials.size(); i++) {
            m_ProxyDials[i]->setPos(dialOffset + (i * 75),calcY(8));
            m_Width += 75;
        }
        if (!m_ProxyDials.isEmpty()) {
            QFont f;
            f.setPointSizeF(10);
            QFontMetricsF fm(f);
            for (int i = 0; i < (m_Device)->parameterGroupCount(); i++)
            {
                const CParameterGroup* g = (m_Device)->parameterGroup(i);
                QColor c = g->color;
                c.setAlphaF(0.2);
                QRectF r1 = m_ProxyDials.at(g->startIndex)->geometry().adjusted(1,-6,-2,6);
                QRectF r2 = (g->endIndex > -1) ? m_ProxyDials.at(g->endIndex)->geometry().adjusted(1,-6,-2,6) : m_ProxyDials.last()->geometry().adjusted(1,-6,-2,6);
                QRectF r = r1.united(r2);
                QPainterPath path;
                path.addRoundedRect(r, 5, 5);
                m_GroupList.append(m_Scene->addPath(path,QColor(0,0,0,40),c));
                if (!g->Name.isEmpty())
                {
                    QGraphicsSimpleTextItem* t = m_Scene->addSimpleText(g->Name,f);
                    t->setBrush(QColor(255,255,255,200));
                    t->setPos(r.center().x() - (fm.boundingRect(g->Name).width() / 2) - 1, r.top() - 2);
                    QGraphicsSimpleTextItem* s = m_Scene->addSimpleText(g->Name,f);
                    s->setBrush(QColor(0,0,0,200));
                    s->setPos(r.center().x() - (fm.boundingRect(g->Name).width() / 2), r.top() - 1);
                    m_GroupList.append(t);
                    m_GroupList.append(s);
                }
            }
        }
    }
    for (QGraphicsItem* i : std::as_const(m_FrameList)) {
        m_Scene->removeItem(i);
        delete i;
    }
    m_FrameList.clear();
    QGraphicsPixmapItem* s = m_Scene->addPixmap(screwPix);
    s->setPos(rackLeftWidth,calcY(4));
    m_FrameList.append(s);
    QGraphicsPixmapItem* s1 = m_Scene->addPixmap(screwPix);
    s1->setPos(rackLeftWidth,calcY(98));
    m_FrameList.append(s1);
    QRect r(m_Scene->sceneRect().toRect());
    r.setLeft(rackLeftWidth - 4);
    r.setTop(calcY(0));
    r.setHeight(rackUnitHeight);
    m_FrameList.append(m_Scene->addLine(QLine(r.bottomLeft(),r.bottomRight()),QPen(Qt::darkGray)));
    m_FrameList.append(m_Scene->addLine(QLine(r.topRight(),r.bottomRight()),QPen(Qt::darkGray)));
    m_FrameList.append(m_Scene->addLine(QLine(r.topLeft(),r.topRight()),QPen(Qt::white)));
    m_FrameList.append(m_Scene->addLine(QLine(r.topLeft(),r.bottomLeft()),QPen(Qt::white)));
}

void CParametersComponent::updateParameterValue(int i)
{
    const int v = Dials[i]->value();
    Parameters[i]->setValue(v);
    Dials.at(i)->setLabels(Parameters[i]);
    m_PresetLabel->setText(m_Device->currentProgramMatches());
}

void CParametersComponent::wheelEvent(QWheelEvent* /*event*/)
{
    /*
    const int move = event->pixelDelta().rx();
    if (move != 0)
    {
        if (m_Width > width())
        {
            int l = m_DialsFrame->geometry().left()+move;
            if (l > 0) l = 0;
            if (l < width()-m_Width) l = width()-m_Width;
            ui->DialsFrame->move(l,0);
            event->accept();
            return;
        }
    }
    event->ignore();
*/
}

bool CParametersComponent::swallowMousePress(QMouseEvent *event, QGraphicsItem* item)
{
    if (event->button() == Qt::RightButton) {
        if (m_ProxyDials.contains(item)) {
            QGraphicsProxyWidget* w = static_cast<QGraphicsProxyWidget*>(item);
            CKnobControl* k = static_cast<CKnobControl*>(w->widget());
            k->popupMenu(event->globalPosition().toPoint());
        }
        else {
            parametersMenu()->popup(event->globalPosition().toPoint());
        }
        return true;
    }
    if ((m_ProxyNameLabel == item) || (m_ProxyUILabel == item))
    {
        if (m_Device) m_Device->toggleUI();
        return true;
    }
    if (event->button() == Qt::LeftButton) emit mousePress(m_Device, event->globalPosition().toPoint());
    return false;
}

QMenu* CParametersComponent::parametersMenu()
{
    CParametersMenu* m = new CParametersMenu(m_Device,nullptr);
    m->setAttribute(Qt::WA_DeleteOnClose,true);
    connect(m,&CParametersMenu::showAutomationRequested,this,&CParametersComponent::showDefaultAutomation);
    connect(m,&CParametersMenu::aboutToChange,this,&CParametersComponent::aboutToChange,Qt::DirectConnection);
    connect(m,&CParametersMenu::parametersChanged,this,&CParametersComponent::parametersChanged);
    connect(m,&CParametersMenu::updateControls,this,&CParametersComponent::updateControls);
    return m;
}
