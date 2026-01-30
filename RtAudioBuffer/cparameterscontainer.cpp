#include "cparameterscontainer.h"
#include "qdprpixmap.h"
#include <QScrollBar>

CParametersContainer::CParametersContainer(QWidget *parent) :
    QGraphicsView(parent)//,
{
    setScene(&Scene);
    setOptimizationFlags(QGraphicsView::DontSavePainterState | QGraphicsView::DontAdjustForAntialiasing);
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    Scene.setItemIndexMethod(QGraphicsScene::NoIndex);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::TextAntialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setMouseTracking(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setFrameStyle(0);
    setLineWidth(0);
    Scene.setBackgroundBrush(QDPRPixmap(":/Brushed Aluminium Tile.bmp"));
    setFixedHeight(rackUnitHeight);
    setAcceptDrops(false);
    zoomer = new QGraphicsViewZoomer(this);
}

CParametersContainer::~CParametersContainer()
{
    Scene.clear();
}

int CParametersContainer::deviceIndex(IDevice* Device)
{
    if (Device == nullptr) return -1;
    const QString ID = Device->deviceID();
    for (int i = 0; i < devices.size(); i++)
    {
        if (ID == devices[i]->deviceID()) return i;
    }
    return -1;
}

void CParametersContainer::addDevice(IDevice* Device)
{
    if (Device != nullptr)
    {
        auto p = new CParametersComponent(scene());
        devices.append(p);
        p->init(Device);
        p->showParameters(devices.size() - 1);
        setFixedHeight(rackUnitHeight * devices.size());
        connect(p,&CParametersComponent::parametersChanged,this,&CParametersContainer::ParametersChanged);
        connect(p,&CParametersComponent::aboutToChange,this,&CParametersContainer::aboutToChange,Qt::DirectConnection);
        connect(p,&CParametersComponent::showAutomationRequested,this,&CParametersContainer::automationRequested);
        connect(p,&CParametersComponent::popupTriggered,this,&CParametersContainer::popupTriggered);
        connect(p,&CParametersComponent::mousePress,this,&CParametersContainer::mousePress);
    }
}

void CParametersContainer::removeDevice(IDevice* Device)
{
    const int i = deviceIndex(Device);
    if (i > -1)
    {
        delete devices.takeAt(i);
        if (devices.isEmpty())
        {
            setFixedHeight(rackUnitHeight);
        }
        else
        {
            setFixedHeight(rackUnitHeight * devices.size());
        }
        drawParameters();
    }
}

void CParametersContainer::moveDevice(int index, int move)
{
    if (index < 0) return;
    if (index > devices.size() -1) return;
    if (move == 0) return;
    int newIndex = std::clamp<int>(index + move,0,devices.size()-1);
    if (newIndex == index) return;
    CParametersComponent* temp = devices.takeAt(index);
    devices.insert(newIndex, temp);
    drawParameters();
}

void CParametersContainer::parametersPixmap(IDevice *d, QPixmap* p) {
    const int i = deviceIndex(d);
    if (i > -1) {
        //QDPRPixmap pix = grab().copy(QRect(60*qApp->devicePixelRatio(),i*rackUnitHeight*qApp->devicePixelRatio(),(width()-60)*qApp->devicePixelRatio(),rackUnitHeight*qApp->devicePixelRatio()));
        *p = grab().copy(QRect(60*qApp->devicePixelRatio(),i*rackUnitHeight*qApp->devicePixelRatio(),(width()-60)*qApp->devicePixelRatio(),rackUnitHeight*qApp->devicePixelRatio()));
    }
}

void CParametersContainer::clear()
{
    for (CParametersComponent* p: std::as_const(devices))
    {
        devices.removeOne(p);
        delete p;
    }
    setFixedHeight(rackUnitHeight);
}

void CParametersContainer::showParameters(IDevice* Device)
{
    qDebug() << "CParametersContainer showParameters";
    const int i = deviceIndex(Device);
    if (i > -1) devices[i]->showParameters(i);
}

void CParametersContainer::updateControls(IDevice* Device)
{
    //qDebug() << "CParametersContainer updateControls";
    const int i = deviceIndex(Device);
    if (i > -1) devices[i]->updateControls();
}

void CParametersContainer::updateControl(IDevice* Device, const CParameter* Parameter)
{
    //qDebug() << "CParametersContainer updateControl";
    if (Device == nullptr) return;
    const int i = deviceIndex(Device);
    if (i > -1) devices[i]->updateControl(Parameter);
}
