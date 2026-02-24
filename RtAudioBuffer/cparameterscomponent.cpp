#include "cparameterscomponent.h"
#include <QInputDialog>
#include <QClipboard>
#include "cparametersmenu.h"
#include "qdprpixmap.h"

#define rackLeftWidth 68
#define rackFrontWidth 1140

CParametersComponent::CParametersComponent(QGraphicsScene* s)
{
    static QDPRPixmap rackLeftPix(QSize(rackUnitHeight,rackUnitHeight),":/RackLeft.png",Qt::KeepAspectRatio);
    static QDPRPixmap rackRightPix(QSize(rackUnitHeight,rackUnitHeight),":/RackRight.png",Qt::KeepAspectRatio);
    static QDPRPixmap screwPix(QSize(10,10),":/screwhead.png");
    m_Device = nullptr;
    m_FrameList.addToScene(s);
    m_GroupList.addToScene(s);
    m_ProxyDials.addToScene(s);
    //m_Scene->addItem(&m_FrameList);
    //m_Scene->addItem(&m_GroupList);
    //m_Scene->addItem(&m_ProxyDials);
    m_FrameList.append(new QGraphicsPixmapItem(rackLeftPix));
    QGraphicsPixmapItem* i = new QGraphicsPixmapItem(rackRightPix);
    i->setPos(rackLeftWidth + rackFrontWidth,0);
    m_FrameList.append(i);
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
    QRect r(rackLeftWidth - 4, 0, rackFrontWidth + 5, rackUnitHeight);
    QGraphicsItem* ri = rectItem(r,Qt::NoPen,QDPRPixmap(":/Brushed Aluminium Tile.bmp"));
    m_FrameList.append(ri);
    QGraphicsPixmapItem* s1 = new QGraphicsPixmapItem(screwPix);
    s1->setPos(rackLeftWidth, 4);
    m_FrameList.append(s1);
    QGraphicsPixmapItem* s2 = new QGraphicsPixmapItem(screwPix);
    s2->setPos(rackLeftWidth, 98);
    m_FrameList.append(s2);
    QGraphicsPixmapItem* s3 = new QGraphicsPixmapItem(screwPix);
    s3->setPos(rackLeftWidth + rackFrontWidth - 14 ,4);
    m_FrameList.append(s3);
    QGraphicsPixmapItem* s4 = new QGraphicsPixmapItem(screwPix);
    s4->setPos(rackLeftWidth + rackFrontWidth - 14, 98);
    m_FrameList.append(s4);
    QGraphicsItem* l1 = lineItem(QLine(r.bottomLeft(),r.bottomRight()),QPen(Qt::darkGray));
    QGraphicsItem* l2 = lineItem(QLine(r.topRight(),r.bottomRight()),QPen(Qt::darkGray));
    QGraphicsItem* l3 = lineItem(QLine(r.topLeft(),r.topRight()),QPen(Qt::white));
    QGraphicsItem* l4 = lineItem(QLine(r.topLeft(),r.bottomLeft()),QPen(Qt::white));

    m_FrameList.append(l1);
    m_FrameList.append(l2);
    m_FrameList.append(l3);
    m_FrameList.append(l4);

    QFont f(m_NameLabel->font());
    f.setPointSize(15);
    m_NameLabel->setFont(f);
    f = m_PresetLabel->font();
    f.setPointSize(9);
    m_PresetLabel->setFont(f);
    m_IDLabel->setFont(f);
    QGraphicsProxyWidget* p = createProxyItem(m_PresetLabel);
    p->setPos(rackLeftWidth + 12, 96);
    m_FrameList.append(p);
    QGraphicsProxyWidget* p1 = createProxyItem(m_IDLabel);
    p1->setPos(rackLeftWidth + 12, 80);
    m_FrameList.append(p1);
    m_ProxyNameLabel = createProxyItem(m_NameLabel);
    m_ProxyNameLabel->setPos(rackLeftWidth + 12, 0);
    m_FrameList.append(m_ProxyNameLabel);
    m_ProxyUILabel = createProxyItem(m_UILabel);
    m_ProxyUILabel->setPos(rackLeftWidth + 136, 12);
    m_FrameList.append(m_ProxyUILabel);
}

CParametersComponent::~CParametersComponent()
{
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
        m_ProxyDials.clear();
        m_GroupList.clear();
        Dials.clear();
        for (int i = 0; i < Device->parameterCount(); i++)
        {
            Parameters.append(Device->parameter(i));
            auto d = new CKnobControl();
            QGraphicsProxyWidget* w = createProxyItem(d);
            m_ProxyDials.append(w);
            w->setPos(i * 75, 8);
            //w->setZValue(1);
            connect(d, &CKnobControl::valueChanged, [=] { updateParameterValue(i); });
            connect(d,&CKnobControl::requestAutomation,this,&CParametersComponent::showAutomation);
            Dials.append(d);
        }
        m_ProxyDials.setZValue(1);
        if (!m_ProxyDials.childItems().isEmpty()) {
            QFont f;
            f.setPointSizeF(10);
            QFontMetricsF fm(f);
            for (int i = 0; i < (m_Device)->parameterGroupCount(); i++)
            {
                const CParameterGroup* g = (m_Device)->parameterGroup(i);
                QColor c = g->color;
                c.setAlphaF(0.2);
                const int endIndex = (g->endIndex > -1) ? g->endIndex : m_ProxyDials.childItems().size() - 1;
                const QGraphicsProxyWidget* sw = static_cast<QGraphicsProxyWidget*>(m_ProxyDials.childItems().at(g->startIndex));
                const QGraphicsProxyWidget* ew = static_cast<QGraphicsProxyWidget*>(m_ProxyDials.childItems().at(endIndex));
                QRectF r1 = sw->geometry().adjusted(1,-6,-2,6);
                QRectF r2 = ew->geometry().adjusted(1,-6,-2,6);
                QRectF r = r1.united(r2);
                QPainterPath path;
                path.addRoundedRect(r, 5, 5);
                m_GroupList.append(pathItem(path,QColor(0,0,0,40),c));
                if (!g->Name.isEmpty())
                {
                    QGraphicsSimpleTextItem* t = new QGraphicsSimpleTextItem(g->Name);
                    t->setFont(f);
                    t->setBrush(QColor(255,255,255,200));
                    t->setPos(r.center().x() - (fm.boundingRect(g->Name).width() / 2) - 1, r.top() - 2);
                    QGraphicsSimpleTextItem* s = new QGraphicsSimpleTextItem(g->Name);
                    s->setFont(f);
                    s->setBrush(QColor(0,0,0,200));
                    s->setPos(r.center().x() - (fm.boundingRect(g->Name).width() / 2), r.top() - 1);
                    m_GroupList.append(t);
                    m_GroupList.append(s);
                }
            }
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
    qDebug() << "CParametersComåponent showParameters";
    m_UILabel->clear();
    m_PresetLabel->clear();
    m_FrameList.setPos(0, index * rackUnitHeight);
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
                m_ProxyUILabel->setVisible(true);
            }
        }
        m_PresetLabel->setText(m_Device->currentProgramMatches());
        m_ProxyDials.setPos(dialOffset, index * rackUnitHeight);
        m_GroupList.setPos(dialOffset, index * rackUnitHeight);
    }
}

void CParametersComponent::updateParameterValue(int i)
{
    const int v = Dials[i]->value();
    Parameters[i]->setValue(v);
    Dials.at(i)->setLabels(Parameters[i]);
    m_PresetLabel->setText(m_Device->currentProgramMatches());
}

bool CParametersComponent::swallowMousePress(QMouseEvent *event, QGraphicsItem* item)
{
    if (event->button() == Qt::RightButton) {
        if (itemIsKnob(item)) {
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
    }
    return false;
}

bool CParametersComponent::itemIsKnob(QGraphicsItem *item) {
    return m_ProxyDials.childItems().contains(item);
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

QGraphicsProxyWidget *CParametersComponent::createProxyItem(QWidget *w) {
    QGraphicsProxyWidget* p = new QGraphicsProxyWidget;
    p->setWidget(w);
    return p;
}
