#include "cparameterscontainer.h"
#include "qdprpixmap.h"
#include "cparametersmenu.h"
#include <QScrollBar>
#include "cconnectionhelper.h"
#include "cautomationlane.h"

#define shadowColor QColor(0,0,0,40)
#define shadowOffset QPoint(5,5)

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
    Scene.setBackgroundBrush(Qt::transparent);
    zoomer = new QGraphicsViewZoomer(this);
    connect(zoomer,&QGraphicsViewZoomer::ZoomChanged,this,&CParametersContainer::setZoom);
    QPalette p = palette();
    p.setBrush(QPalette::Window, QBrush(QDPRPixmap(":/paper-texture.jpg")));
    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);
    setPalette(p);
    setAcceptDrops(true);
    //verticalScrollBar()->setPageStep(rackUnitHeight);
}

CParametersContainer::~CParametersContainer()
{
    Scene.clear();
}

void CParametersContainer::Init(CDeviceList *l) {
    m_DL = l;
    adjustSizes();
}

int CParametersContainer::deviceIndex(IDevice* Device)
{
    if (Device == nullptr) return -1;
    const QString ID = Device->deviceID();
    for (int i = 0; i < parameterDevices.size(); i++)
    {
        if (ID == parameterDevices[i]->deviceID()) return i;
    }
    return -1;
}

int CParametersContainer::deviceCount() { return parameterDevices.size(); }

void CParametersContainer::addDevice(IDevice* Device)
{
    if (Device != nullptr)
    {
        auto p = new CParametersComponent(scene());
        parameterDevices.append(p);
        p->init(Device);
        p->showParameters(parameterDevices.size() - 1);
        connect(p,&CParametersComponent::parametersChanged,this,&CParametersContainer::ParametersChanged);
        connect(p,&CParametersComponent::aboutToChange,this,&CParametersContainer::aboutToChange,Qt::DirectConnection);
        connect(p,&CParametersComponent::showAutomationRequested,this,&CParametersContainer::showAutomation);
        connect(p,&CParametersComponent::popupTriggered,this,&CParametersContainer::popupTriggered);
        devices.append(new CJacksDevice());
        devices.last()->init(Device);
        DrawConnections();
    }
    setVisible(true);
    adjustSizes();
    const int i = deviceIndex(Device);
    if (i > -1) animateTo(i);
}

void CParametersContainer::removeDevice(IDevice* Device)
{
    const int i = deviceIndex(Device);
    if (i > -1)
    {
        delete parameterDevices.takeAt(i);
        drawParameters();
        delete devices.takeAt(i);
        DrawConnections();
    }
    adjustSizes();
}

void CParametersContainer::moveDevice(int index, int move)
{
    if (index < 0) return;
    if (index > parameterDevices.size() -1) return;
    if (move == 0) return;
    int newIndex = std::clamp<int>(index + move,0,parameterDevices.size()-1);
    if (newIndex == index) return;
    CParametersComponent* temp = parameterDevices.takeAt(index);
    parameterDevices.insert(newIndex, temp);
    drawParameters();
    CJacksDevice* t = devices.takeAt(index);
    devices.insert(newIndex, t);
    DrawConnections();
}

void CParametersContainer::parametersPixmap(IDevice *d, QPixmap* pix) {
    const int i = deviceIndex(d);
    if (i > -1) {
        connectionItems.setVisible(false);
        const int di = deviceIndex(d);
        if (di > -1) devices[di]->PlugImages.setVisible(false);
        QRectF sceneRectPart(0, i * rackUnitHeight, sceneRect().width(), rackUnitHeight);
        QPixmap pm(sceneRectPart.size().toSize());
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        scene()->render(&p, QRectF(pm.rect()), sceneRectPart);
        *pix = pm;
        connectionItems.setVisible(true);
        if (di > -1) devices[di]->PlugImages.setVisible(true);
    }
}

void CParametersContainer::drawParameters() {
    int i = 0;
    for (CParametersComponent* p : std::as_const(parameterDevices)) {
        p->showParameters(i++);
    }
}

QRectF CParametersContainer::deviceRect(IDevice *device) {
    int index = deviceIndex(device);
    if (index > -1) {
        QPointF p(0,rackUnitHeight * index);
        QSizeF s(width(),rackUnitHeight);
        return QRectF(p * zoomer->getZoom(),s * zoomer->getZoom());
    }
    return QRect();
}

double CParametersContainer::constantWidth() {
    return qMax<int>(width(), width() / zoomer->getZoom());
}

double CParametersContainer::unitHeight() {
    return rackUnitHeight * zoomer->getZoom();
}

QGraphicsProxyWidget *CParametersContainer::addProxyWidget(QWidget *a) {
    a->resize(constantWidth(),rackUnitHeight);
    QGraphicsProxyWidget* w = Scene.addWidget(a);
    w->setZValue(5);
    return w;
}

QList<QWidget *> CParametersContainer::ProxyWidgets() const {
    QList<QWidget*> l;
    for (QGraphicsItem* i : Scene.items()) {
        if (auto proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(i)) {
            if (i->zValue() > 4) l.append(proxy->widget());
        }
    }
    return l;
}

void CParametersContainer::mousePressEvent(QMouseEvent *event) {
    const QPoint scenePos = mapToScene(event->pos()).toPoint();
    if (hasAutomation(scenePos)) {
        QGraphicsView::mousePressEvent(event);
        return;
    }
    Dragging = false;
    DragJack = nullptr;
    const QString JackID = MouseOverJack(scenePos);
    if (!JackID.isEmpty()) {
        if (event->button()==Qt::RightButton) {
            CConnectionsMenu* m = new CConnectionsMenu(m_DL->jack(JackID),m_DL,this);
            connect(m,&CConnectionsMenu::aboutToChange,this,&CParametersContainer::aboutToChange,Qt::DirectConnection);
            connect(m,&CConnectionsMenu::connectionsChanged,this,&CParametersContainer::DrawChangedConnections);
            m->popup(event->globalPosition().toPoint());
            return;
        }
        DragJack = m_DL->jack(JackID);
        IDevice* d = m_DL->device(DragJack->owner());
        DragJackPos = jackPoint(d,d->jackIndex(JackID));
        if (DragJack)
        {
            if (event->button()==Qt::LeftButton)
            {
                QApplication::restoreOverrideCursor();
                QApplication::setOverrideCursor(Qt::OpenHandCursor);
                Dragging=true;
                return;
            }
        }
    }
    int i = scenePos.y() / rackUnitHeight;
    if ((i >= 0) && (i < parameterDevices.size())) {
        const QGraphicsItemList items(scene()->items(scenePos, Qt::IntersectsItemShape, Qt::DescendingOrder, transform()));
        QGraphicsItem* item = nullptr;
        for (QGraphicsItem* it : std::as_const(items))
        {
            if (auto proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(it)) item = proxy;
        }
        if (parameterDevices[i]->swallowMousePress(event,item)) return;
        if (!parameterDevices[i]->itemIsKnob(item)) startDrag(m_DL->device(devices[i]->deviceID()),scenePos);
        QGraphicsView::mousePressEvent(event);
    }
}

void CParametersContainer::clear()
{
    for (CParametersComponent* p: std::as_const(parameterDevices))
    {
        parameterDevices.removeOne(p);
        delete p;
    }
    drawParameters();
    for (CJacksDevice* d : std::as_const(devices))
    {
        devices.removeOne(d);
        delete d;
    }
    DrawConnections();
    adjustSizes();
}

void CParametersContainer::showParameters(IDevice* Device)
{
    qDebug() << "CParametersContainer showParameters";
    const int i = deviceIndex(Device);
    if (i > -1) parameterDevices[i]->showParameters(i);
    if (((i * unitHeight()) > verticalScrollBar()->sliderPosition()) && (((i + 1) * unitHeight()) < verticalScrollBar()->sliderPosition()+height())) return;
    if (i > -1) animateTo(i);
}
/*
void CParametersContainer::updateControls(IDevice* Device)
{
    //qDebug() << "CParametersContainer updateControls";
    const int i = deviceIndex(Device);
    if (i > -1) parameterDevices[i]->updateControls();
}
*/
void CParametersContainer::updateControl(IDevice* Device, const CParameter* Parameter)
{
    //qDebug() << "CParametersContainer updateControl";
    if (Device == nullptr) return;
    const int i = deviceIndex(Device);
    if (i > -1) parameterDevices[i]->updateControl(Parameter);
}

QPoint CParametersContainer::jackPoint(IDevice* device, int i)
{
    int di = deviceIndex(device);
    if (di > -1)
    {
        return devices[di]->jackPoint(i);
    }
    return {-1,-1};
}

bool CParametersContainer::hasAutomation(const QPoint& p){
    QGraphicsItem* w = Scene.itemAt(p,transform());
    if (w) {
        if (w->zValue() > 4) return true;
    }
    return false;
}

void CParametersContainer::DrawConnections()
{
    for (int i = 0; i < devices.size(); i++) devices[i]->paint(&Scene,i);
    updateConnections();
}

void CParametersContainer::updateConnections(){
    connectionItems.erase(&Scene);
    for (CJacksDevice* d : std::as_const(devices)) {
        IDevice* d1 = m_DL->device(d->deviceID());
        for (int i = 0; i < d1->jackCount(); i++) {
            if (d->PlugImages.childItems().size() > i) d->PlugImages.childItems().at(i)->setVisible(d1->jack(i)->isConnected());
        }
    }
    QList<IDevice*> paintedContainers;
    for (IDevice* inDevice : std::as_const(*m_DL->devices()))
    {
        connectionItems.append(DrawDeviceConnections(inDevice,paintedContainers));
    }
    connectionItems.append(DrawThisConnections());
    QRectF r(Scene.sceneRect());
    QPoint p = transform().map(QPoint(0,devices.size() * rackUnitHeight));
    r.setHeight(p.ry());
    Scene.setSceneRect(r);
}

QGraphicsItemList CParametersContainer::DrawThisConnections(){
    QGraphicsItemList l;
    int inJackCount = 0;
    int outJackCount = 0;
    for (int ji = 0; ji < m_DL->jackCount(); ji++) {
        if (m_DL->jack(ji)->owner() == "This") {
            const IJack* thisJack = m_DL->jack(ji);
            for (IDevice* d : std::as_const(*m_DL->devices())) {
                for (int i = 0; i < d->jackCount(); i++) {
                    const IJack* j = d->jack(i);
                    if (j->isConnectedTo(thisJack)) {
                        QColor c = j->JackColor();
                        c.setAlphaF(CPresets::presets().ConnectionsOpacity);
                        if (thisJack->isInJack()) {
                            l.append(CConnectionHelper::DrawCord(QPoint(inJackCount * 15,sceneRect().height()),jackPoint(d,i),c, &Scene));
                        }
                        else {
                            l.append(CConnectionHelper::DrawCord(jackPoint(d,i),QPoint(outJackCount * 15,0),c, & Scene));
                        }
                    }
                }
            }
            (thisJack->isInJack()) ? inJackCount++ : outJackCount++;
        }
    }
    return l;
}

QGraphicsItemList CParametersContainer::DrawDeviceConnections(IDevice* device, QList<IDevice*>& paintedContainers)
{
    QGraphicsItemList l;
    for (int i = 0; i < device->jackCount(); i++)
    {
        for (IDevice* outDevice : paintedContainers)
        {
            for (int j = 0; j < outDevice->jackCount(); j++)
            {
                if (device->jack(i)->isConnectedTo(outDevice->jack(j)))
                {
                    QPoint point1 = jackPoint(device,i);
                    if (point1 != QPoint(-1,-1))
                    {
                        QPoint point2 = jackPoint(outDevice,j);
                        if (point2 != QPoint(-1,-1))
                        {
                            QColor c = device->jack(i)->JackColor();
                            c.setAlphaF(CPresets::presets().ConnectionsOpacity);
                            if (device->jack(i)->isInJack())
                            {
                                l.append(CConnectionHelper::DrawCord(point2,point1,c,&Scene));
                            }
                            else
                            {
                                l.append(CConnectionHelper::DrawCord(point1,point2,c,&Scene));
                            }
                        }
                    }
                }
            }
        }
    }
    paintedContainers.append(device);
    return l;
}

void CParametersContainer::DrawChangedConnections()
{
    emit connectionsChanged();
    updateConnections();
}

void CParametersContainer::setZoom(double /*zoom*/) {
    for (QWidget* a : ProxyWidgets()) {
        a->resize(constantWidth(),a->height());
    }
    adjustSizes();
}

QGraphicsItemList CParametersContainer::DrawConnection(QPoint p1, QPoint p2, const QColor& color, const qreal linewidth)
{
    QGraphicsItemList l;
    QPainterPath p;
    QRect r(p1,p2);
    r=r.normalized();
    int adjust = 60 - r.width();
    if (adjust < 0) adjust=0;
    r.adjust(adjust,(r.height()/5)+50,-adjust,(r.height()/5)+100);
    if (p1.x() > p2.x()) std::swap(p1,p2);
    p.moveTo(p1);
    if (p1.y() < p2.y())
    {
        p.cubicTo(r.bottomLeft(),p2+((r.bottomRight()-p2)/2),p2);
    }
    else
    {
        p.cubicTo(p1+((r.bottomLeft()-p1)/2),r.bottomRight(),p2);
    }
    l.append(Scene.addPath(p.translated(5,5),QPen(QColor(0,0,0,40),linewidth,Qt::SolidLine,Qt::RoundCap)));
    QColor c(color);
    c.setAlphaF(CPresets::presets().ConnectionsOpacity);
    l.append(Scene.addPath(p,QPen(c,linewidth,Qt::SolidLine,Qt::RoundCap)));
    for (QGraphicsItem* i : l) {
        i->setZValue(2);
    }
    return l;
}

void CParametersContainer::adjustSizes(){
    if (deviceCount()) {
        setMaximumHeight(unitHeight() * deviceCount());
        setMinimumHeight(unitHeight());
        setVisible(true);
    }
    else {
        setMaximumHeight(0);
        setMinimumHeight(0);
        setVisible(false);
    }
    QRectF r(sceneRect());
    r.setTopLeft(QPointF(0,0));
    r.setHeight(rackUnitHeight * deviceCount());
    setSceneRect(r);
    emit sizeChanged();
}

void CParametersContainer::startDrag(IDevice *d, QPoint scenePos) {
    if (m_DL->deviceCount() < 1) return;
    const int index = m_DL->indexOfDevice(d);
    QPoint viewPos = mapFromScene(scenePos);
    qDebug() << rect() << viewPos << scenePos << QPoint(0,unitHeight() * index) << viewPos - QPoint(0,unitHeight() * index);
    QPixmap pm;
    parametersPixmap(d,&pm);
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-objectstudio-deviceid", QByteArray(d->deviceID().toLatin1()));
    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pm.scaled(pm.size() * zoomer->getZoom(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    QPoint hotspot = (scenePos * zoomer->getZoom()) - QPoint(0, unitHeight() * index);
    drag->setHotSpot(hotspot);
    Qt::DropActions a = drag->exec(Qt::MoveAction);
    qDebug() << "Drag Target" << drag->target() << a << drag;
    if (a == Qt::IgnoreAction) {
        QPoint viewCursor = mapFromGlobal(QCursor::pos());
        if (!rect().contains(viewCursor)) emit deviceRemoved(d);
    }
}

QString CParametersContainer::MouseOverJack(const QPoint &Pos)
{
    for (CJacksDevice* d : std::as_const(devices)) {
        const int j = d->MouseOverJack(Pos);
        if (j > -1) return d->jackID(j);
    }
    return QString();
}

void CParametersContainer::mouseMoveEvent(QMouseEvent* event)
{
    static QDPRPixmap plugPix = QDPRPixmap(rackJackSize,":/Plug.png").shadowedPixmap(10);
    const QPoint scenePos = mapToScene(event->pos()).toPoint();
    if (hasAutomation(scenePos)) {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }
    if (Dragging) //drag connection
    {
        CConnectionHelper::SetConnectCursor(this,m_DL->jack(MouseOverJack(scenePos)),DragJack);
        DragList.erase(&Scene);
        QColor c(DragJack->JackColor());
        c.setAlphaF(CPresets::presets().ConnectionsOpacity);
        QGraphicsPixmapItem* plug2 = Scene.addPixmap(plugPix);
        plug2->setPos(DragJackPos);
        DragList.append(plug2);
        QGraphicsPixmapItem* plug1 = Scene.addPixmap(plugPix);
        plug1->setPos(scenePos);
        DragList.append(plug1);
        DragList.setPos(-rackJackSize.width() / 2, -rackJackSize.height() / 2);
        DragList.setZValue(2);
        if (DragJack->isOutJack())
        {
            DragList.append(CConnectionHelper::DrawCord(DragJackPos,scenePos,c,&Scene));
        }
        else
        {
            DragList.append(CConnectionHelper::DrawCord(scenePos,DragJackPos,c,&Scene));
        }
        return;
    }
    if (scenePos != MousePoint)
    {
        setToolTip(MouseOverJack(scenePos));
        MousePoint = scenePos;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void CParametersContainer::mouseReleaseEvent(QMouseEvent *event){
    const QPoint scenePos = mapToScene(event->pos()).toPoint();
    if (hasAutomation(scenePos)) {
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }
    DragList.erase(&Scene);
    if (Dragging)
    {
        setToolTip(QString());
        Dragging=false;
        QApplication::restoreOverrideCursor();
        IJack* HoverJack = m_DL->jack(MouseOverJack(scenePos));
        if (HoverJack) m_DL->connect(HoverJack->jackID(),DragJack->jackID());
        updateConnections();
        emit connectionsChanged();
    }
}

void CParametersContainer::mouseDoubleClickEvent(QMouseEvent *event) {
    const QPoint scenePos = mapToScene(event->pos()).toPoint();
    if (hasAutomation(scenePos)) {
        QGraphicsView::mouseDoubleClickEvent(event);
        return;
    }
    Dragging=false;
    DragJack=nullptr;
    QMutexLocker locker(&mutex);
    QApplication::restoreOverrideCursor();
    if (IJack* J = m_DL->jack(MouseOverJack(scenePos)))
    {
        m_DL->disconnectJack(J->jackID());
        setToolTip(J->captionX());
        updateConnections();
        emit connectionsChanged();
        return;
    }
}

void CParametersContainer::dragEnterEvent(QDragEnterEvent *e) {
    qDebug() << "Drag enter";
    if (e->mimeData()->hasFormat("application/x-objectstudio-deviceid")) {
        e->acceptProposedAction();
    }
}

void CParametersContainer::dragMoveEvent(QDragMoveEvent *e) {
    qDebug() << "drag move" << rect() << e->position();
    if (e->mimeData()->hasFormat("application/x-objectstudio-deviceid")) {
        if (rect().contains(e->position().toPoint())) e->acceptProposedAction();
    }
}

void CParametersContainer::dragLeaveEvent(QDragLeaveEvent* e) {
    qDebug() << "Drag leave" << rect() << e->isAccepted();
}

void CParametersContainer::dropEvent(QDropEvent *e) {
    qDebug() << "dropEvent" << e->position() << e->dropAction();
    QByteArray b(e->mimeData()->data("application/x-objectstudio-deviceid"));
    int deviceIndex = m_DL->indexOfDevice(m_DL->device(QString::fromLatin1(b)));
    QPointF scenePos = mapToScene(e->position().toPoint());
    int newIndex = scenePos.y() / rackUnitHeight;
    int move = newIndex - deviceIndex;
    if (deviceIndex < 0) return;
    if (deviceIndex > m_DL->deviceCount() -1) return;
    if (newIndex < 0) return;
    if (newIndex > m_DL->deviceCount() -1) return;
    if (move == 0) return;
    if (newIndex == deviceIndex) return;
    e->acceptProposedAction();
    emit devicesReordered(deviceIndex,move);
}

void CParametersContainer::createAutomationLane(IDevice* d, int parameterIndex)
{
    CAutomationLane* a = new CAutomationLane();
    a->fill(d,parameterIndex,m_DL);
    QGraphicsProxyWidget* w = addProxyWidget(a);
    w->setPos(0,deviceIndex(d) * rackUnitHeight);
    connect(this,&CParametersContainer::closeAutomation,a,&CAutomationLane::close);
}

void CParametersContainer::showAutomation(IDevice* d, int ParameterIndex)
{
    if (ParameterIndex == -1) ParameterIndex = 0;
    createAutomationLane(d, ParameterIndex);
}

void CParametersContainer::unserialize(const QDomLiteElement* xml) {
    if (QDomLiteElement* Lanes = xml->elementByTag("AutomationLanes")) {
        for (const QDomLiteElement* e : (const QDomLiteElementList)Lanes->elementsByTag("AutomationLane")) {
            if (IDevice* d = m_DL->device(e->attribute("DeviceID"))) {
                createAutomationLane(d,e->attributeValueInt("ParameterIndex"));
            }
        }
    }
}

void CParametersContainer::serialize(QDomLiteElement* xml) const
{
    QDomLiteElement* Lanes = xml->appendChild("AutomationLanes");
    for (QWidget* a : ProxyWidgets()) {
        if (QDomLiteElement* l = Lanes->appendChild("AutomationLane")) static_cast<CAutomationLane*>(a)->serialize(l);
    }
}

void CParametersContainer::animateTo(int i)
{
    const int oldPos = verticalScrollBar()->sliderPosition();
    const int newPos = unitHeight() * i;
    if (newPos == oldPos) return;
    qDebug() << "AnimateTo" << oldPos << newPos << verticalScrollBar()->maximum() << verticalScrollBar()->minimum();
    QPropertyAnimation *animation = new QPropertyAnimation(verticalScrollBar(), "sliderPosition");
    animation->setEasingCurve(QEasingCurve::OutQuart);
    animation->setDuration((abs(oldPos-newPos)/4)+500);
    animation->setStartValue(oldPos);
    animation->setEndValue(newPos);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}
