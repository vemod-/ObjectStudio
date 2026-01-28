#include "cdesktopcomponent.h"
#include "ui_cdesktopcomponent.h"
#include <QGraphicsSimpleTextItem>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QCalendarWidget>
#include "cconnectionhelper.h"
#include <QHBoxLayout>
//#include <quazip.h>
//#include <quazipfile.h>
#include "cparametersmenu.h"
#include "../Projectpage/cprojectpage.h"
#include <QScrollBar>
#include "caddins.h"
#include "cresourceinitializer.h"

#define shadowColor QColor(0,0,0,40)
#define shadowOffset QPoint(5,5)
#define zeroPoint QPoint(0,0)

QList<QGraphicsItem*> CJackContainer::paint(QGraphicsScene* Scene)
{
    static QDPRPixmap freeDeviceJack(jackSize,":/Jack.png");
    static QDPRPixmap connectedDeviceJack(jackSize,":/Plug.png");
    QList<QGraphicsItem*> items;
    for (JackRect& j : jackRects)
    {
        QPen p(j.jack->JackColor(),jackPen);
        items.append(Scene->addEllipse(QRect(j.translated(geometry.topLeft()).topLeft(),QSize(j.size() - QSize(jackPen,jackPen))),p,QBrush(QColor(0,0,0,80))));
        QGraphicsPixmapItem* i = (j.jack->isConnected()) ? Scene->addPixmap(connectedDeviceJack) : Scene->addPixmap(freeDeviceJack);
        i->setPos(j.translated(geometry.topLeft()).topLeft()-QPoint(1,1));
        items.append(i);
    }
    return items;
}

//---------------------------------------------------------------------------------------

void CDeviceComponent::getPic()
{
    if (m_View == BackView) {
        geometry.setSize(deviceBackSize);
    }
    else if (m_View == TopView) {
        geometry.setSize(deviceTopSize);
    }
    if (m_px) delete m_px;
    m_px=nullptr;
    if (m_Device->hasUI())
    {
        const QPixmap* px = m_Device->picture();
        if (px)
        {
            //qDebug() << px->size();
            m_px=new QPixmap(px->scaled((geometry.size()-QSize(deviceResolution / 5, deviceResolution / 12))*2,Qt::KeepAspectRatio,Qt::SmoothTransformation));
            m_px->setDevicePixelRatio(2);
            delete px;
        }
    }
}

CDeviceComponent::CDeviceComponent() : m_Device(nullptr), m_Active(false), m_px(nullptr) {
    geometry.moveTopLeft(QPoint(100,100));
}

CDeviceComponent::CDeviceComponent(IDevice *Device, const QString &ClassName) : m_Device(nullptr), m_Active(false), m_px(nullptr)
{
    geometry.moveTopLeft(QPoint(100,100));
    init(Device,ClassName);
}

CDeviceComponent::~CDeviceComponent() { if (m_px) delete m_px; }

void CDeviceComponent::init(IDevice *Device, const QString &ClassName)
{
    m_ClassName = ClassName;
    m_Device = Device;
    for (int i = 0; i < Device->jackCount(); i++) jackRects.append(JackRect(Device->jack(i)));
    getPic();
}

IDevice *CDeviceComponent::device() const { return m_Device; }

const QString CDeviceComponent::className() const { return m_ClassName; }

void CDeviceComponent::setSelected(const bool Active) {
    m_Active=Active;
    //if (Active) getPic();
}

bool CDeviceComponent::inside(const QRect &r) { return r.contains(geometry); }

CDeviceComponent::DeviceView CDeviceComponent::view() {
    return m_View;
}

void CDeviceComponent::setView(DeviceView v) {
    m_View = v;
}

QList<QGraphicsItem*> CDeviceComponent::paint(QGraphicsScene* Scene)
{
    static QDPRPixmap rackTopPix(deviceTopSize,":/RackTop.png");
    static QDPRPixmap rackBackPix(deviceBackSize,":/RackBack.png",Qt::KeepAspectRatio);
    static QDPRPixmap rackFrontPix(deviceBackSize,":/RackFront.png",Qt::KeepAspectRatio);
    QList<QGraphicsItem*> items;
    if (m_Device != nullptr)
    {
        double inJackFactor = 0.1;
        double outJackFactor = 0.1;
        double textFactor = 0.5;
        bool uiPic = false;
        jackRects.clear();
        for (int i = 0; i < m_Device->jackCount(); i++) jackRects.append(JackRect(m_Device->jack(i)));
        if (geometry.left()<1) geometry.moveLeft(1);
        if (geometry.top()<1) geometry.moveTop(1);
        if (m_View == BackView) {
            textFactor = -0.4;
            inJackFactor = 0.19;
            outJackFactor = 0.27;
            geometry.setSize(deviceBackSize);
            QGraphicsPixmapItem* shadow = (m_Active) ? Scene->addPixmap(rackBackPix.activeShadow()) : Scene->addPixmap(rackBackPix.inactiveShadow());
            shadow->setPos(geometry.topLeft());
            QGraphicsPixmapItem* front = Scene->addPixmap(rackBackPix.shadowedPixmap());
            front->setPos(geometry.topLeft());
            items.append(front);
        }
        else if (m_View == TopView) {
            uiPic = true;
            geometry.setSize(deviceTopSize);
            QGraphicsPixmapItem* shadow = (m_Active) ? Scene->addPixmap(rackTopPix.activeShadow()) : Scene->addPixmap(rackTopPix.inactiveShadow());
            shadow->setPos(geometry.topLeft());
            QGraphicsPixmapItem* front = Scene->addPixmap(rackTopPix.shadowedPixmap());
            front->setPos(geometry.topLeft());
            items.append(front);
        }
        else if (m_View == FrontView) {
            textFactor = -0.4;
            inJackFactor = 0.02;
            outJackFactor = 0.5;
            geometry.setSize(deviceBackSize);
            QGraphicsPixmapItem* shadow = (m_Active) ? Scene->addPixmap(rackFrontPix.activeShadow()) : Scene->addPixmap(rackFrontPix.inactiveShadow());
            shadow->setPos(geometry.topLeft());
            QGraphicsPixmapItem* front = Scene->addPixmap(rackFrontPix.shadowedPixmap());
            front->setPos(geometry.topLeft());
            items.append(front);
            if (!m_frontPix.isNull()) {
                QGraphicsPixmapItem* front1 = Scene->addPixmap(m_frontPix);
                front1->setPos(geometry.left() + (deviceResolution / 6.4),geometry.bottom() - (deviceResolution / 2.08));
                items.append(front1);
            }
        }
        else if (m_View == Drawing) {
            uiPic = true;
            geometry.setSize(deviceTopSize);
            QPainterPath path(zeroPoint);
            path.addRoundedRect(geometry,5,5);
            for (int i = 0; i < 10 ; i++) Scene->addPath(path.translated(i+1,i+1),Qt::NoPen,QColor(0,0,0,10));

            QPen p(Qt::NoPen);
            p.setWidth(1);
            QLinearGradient lg(0,geometry.top(),0,geometry.height()+geometry.top());
            if (m_Active)
            {
                p=QPen(Qt::black);
                lg.setColorAt(0,QColor(0xee,0xee,0xee));
                lg.setColorAt(0.49999,QColor(0xbb,0xbb,0xbb));
                lg.setColorAt(0.5,QColor(0xaf,0xaf,0xaf));
                lg.setColorAt(1,QColor(0x99,0x99,0x99));
            }
            else
            {
                p=QPen(Qt::gray);
                lg.setColorAt(0,QColor(0xdd,0xdd,0xdd));
                lg.setColorAt(0.49999,QColor(0xaa,0xaa,0xaa));
                lg.setColorAt(0.5,QColor(0x8f,0x8f,0x8f));
                lg.setColorAt(1,QColor(0x77,0x77,0x77));
            }
            QBrush b(lg);
            //path=QPainterPath(zeroPoint);
            //path.addRoundedRect(geometry,5,5);
            items.append(Scene->addPath(path,p,b));
        }
        if (uiPic) {
            if (m_Device->hasUI())
            {
                if (m_px)
                {
                    QGraphicsPixmapItem* pi = Scene->addPixmap(*m_px);
                    const QSize sz = QSizeF((geometry.size() - pi->boundingRect().size()) / 2).toSize();
                    pi->setPos(geometry.topLeft()+QPoint(sz.width(),sz.height()));
                    pi->setZValue(0);
                    items.append(pi);
                }
            }
        }
        QString Caption = m_Device->caption();//m_Device->name() + " " + QString::number(m_Device->index());
        QString FileName;
        if (!m_Device->alias().isEmpty()) {
            FileName = "(" + Caption + ")";
        }
        else if (!m_Device->filename().isEmpty()) {
            FileName = "(" + Caption + ")";
            Caption = QFileInfo(m_Device->filename()).fileName();
        }
        QFont f;
        QFontMetrics fm(f);
        if (fm.horizontalAdvance(Caption) > geometry.width()) Caption = Caption.left(7)+QStringLiteral("...") + Caption.right(7);
        QPoint TextOffset((geometry.width() - fm.horizontalAdvance(Caption))/2,(geometry.height() - fm.height()) * textFactor);
        captionRect.setTopLeft(geometry.topLeft() + TextOffset);
        captionRect.setSize(QSize(fm.horizontalAdvance(Caption),fm.height()));
        if (!FileName.isEmpty()) TextOffset.setY(TextOffset.y() - (fm.height()/2));
        qDebug() << geometry << TextOffset << textFactor << Caption << f;
        items.append(CConnectionHelper::DrawShadowText(Caption,f,geometry.topLeft()+TextOffset,Scene));
        if (!FileName.isEmpty())
        {
            if (fm.horizontalAdvance(FileName) > geometry.width()) FileName = FileName.left(7) + QStringLiteral("...") + FileName.right(7);
            TextOffset.setX((geometry.width() - fm.horizontalAdvance(FileName)) / 2);
            TextOffset.setY(TextOffset.y() + fm.height());
            items.append(CConnectionHelper::DrawShadowText(FileName,f,geometry.topLeft()+TextOffset,Scene));
        }
        /*
        int InCount=1;
        int OutCount=1;
        for (JackRect& j : jackRects)
        {
            (j.jack->isInJack()) ? InCount++ : OutCount++;
        }
*/
        int MaxCount = qMax(m_Device->inJackCount(),m_Device->outJackCount());

        int InIndex=0;
        int OutIndex=0;
        if (m_View == FrontView) {
            const float InFactor = (geometry.width() * 8.0) / (MaxCount * 12.0);
            const float OutFactor = (geometry.width() * 8.0) / (MaxCount * 12.0);
            for (JackRect& j : jackRects) {
                (j.jack->isInJack()) ? j.moveCenter(QPoint(geometry.width() - ((InIndex++ * InFactor) + (deviceResolution * 0.5)),deviceResolution * inJackFactor)) :
                    j.moveCenter(QPoint(geometry.width() - ((OutIndex++ * OutFactor) + (deviceResolution * 0.5)),geometry.height() - (deviceResolution * outJackFactor)));
                //qDebug() << front->boundingRect() << front->pixmap().size() << geometry << j.center() << j.topLeft() << j.size();
            }
        }
        else {
            for (JackRect& j : jackRects) {
                const float InFactor = (geometry.width() * 10.0) / (MaxCount * 12.0);
                const float OutFactor = (geometry.width() * 10.0) / (MaxCount * 12.0);
                (j.jack->isInJack()) ? j.moveCenter(QPoint((InIndex++ * InFactor) + (deviceResolution * 0.2),deviceResolution * inJackFactor)) :
                    j.moveCenter(QPoint((OutIndex++ * OutFactor) + (deviceResolution * 0.2),geometry.height() - (deviceResolution * outJackFactor)));
                //qDebug() << front->boundingRect() << front->pixmap().size() << geometry << j.center() << j.topLeft() << j.size();
            }
            items.append(CJackContainer::paint(Scene));
        }
    }
    return items;
}

void CDeviceComponent::setFrontPix(const QDPRPixmap &p) {
    m_frontPix = QDPRPixmap(p).scaled(p.size() * 0.19,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    const int w = qMin<int>(deviceResolution * 2.6 * qApp->devicePixelRatio(), m_frontPix.width());
    if (w < m_frontPix.width()) m_frontPix = m_frontPix.copy(0, 0, w, m_frontPix.height());
}

bool CDeviceComponent::frontPixSet() {
    return !m_frontPix.isNull();
}

//---------------------------------------------------------------------------

QList<QGraphicsItem*> CJackBar::paint(QGraphicsScene* Scene)
{
    QList<QGraphicsItem*> items;
    //if (geometry.topLeft()==zeroPoint) Scene->addRect(geometry.translated(0,shadowOffset.y()),Qt::NoPen,shadowColor);
    QLinearGradient lg(0,geometry.top(),0,geometry.height()+geometry.top());
    lg.setColorAt(0,QColor(0xdd,0xdd,0xdd));
    lg.setColorAt(0.49999,QColor(0xbb,0xbb,0xbb));
    lg.setColorAt(0.5,QColor(0x9f,0x9f,0x9f));
    lg.setColorAt(1,QColor(0x78,0x78,0x78));
    QBrush b(lg);

    items.append(Scene->addRect(geometry,Qt::NoPen,b));

    for (int i=0;i<jackRects.size();i++)
    {
        JackRect* JR = &jackRects[i];
        JR->moveCenter(QPoint((i*20)+height,height * 0.5));
    }
    items.append(CJackContainer::paint(Scene));
    return items;
}

//---------------------------------------------------------------------------

CDesktopComponent::CDesktopComponent(QWidget *parent) :
    QGraphicsView(parent),
    ui(new Ui::CDesktopComponent)
{
    CResourceInitializer::initializeResources();
    ui->setupUi(this);
    m_MainWindow = parent;
    setFrameStyle(0);
    setAcceptDrops(false);
    zoomer = new QGraphicsViewZoomer(this);
    connect(zoomer,&QGraphicsViewZoomer::ZoomChanged,this,&CDesktopComponent::changeZoom);
    setBackgroundBrush(QDPRPixmap(":/paper-texture.jpg"));
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    setDragMode(NoDrag);
    Rubberband=new QiPhotoRubberband(this);
    Rubberband->hide();
    Marked=false;
    m_MD=false;
    Dragging=false;
    MouseDown=false;
    m_DeviceIndex=-1;
    setScene(&Scene);
    setOptimizationFlags(QGraphicsView::DontSavePainterState | QGraphicsView::DontAdjustForAntialiasing);
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    Scene.setItemIndexMethod(QGraphicsScene::NoIndex);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::TextAntialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setMouseTracking(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_ParentWindow=nullptr;
    DeviceList.setHost(this);
    MainMenu = new CMainMenu(this,m_MainWindow,CPresets::Organization(),CPresets::AppName(),QString(),_DocumentPath,parent);
    MainMenu->actionWizard->setEnabled(false);
    MainMenu->actionWizard->setVisible(false);
}

CDesktopComponent::~CDesktopComponent()
{
    delete ui;
}

void CDesktopComponent::init(QWidget *mainWindow, QWidget *parent) {
    m_MainWindow = mainWindow;
    m_ParentWindow = parent;
}

IJack* CDesktopComponent::addJack(IJack* Jack, int PolyIndex)
{
    QMutexLocker locker(&mutex);
    DeviceList.addJack(Jack,PolyIndex);
    if (PolyIndex==0)
    {
        (Jack->isInJack()) ? JackBar2.addJack(Jack) : JackBar1.addJack(Jack);
    }
    return Jack;
}

void CDesktopComponent::addInsideJack(IJack *J, IDevice *d, const QString &alias) {
    IJack* J1 = addJack(J->createInsideJack(JacksCreated.size(),d),0);
    J1->setAlias(alias);
    JacksCreated.append(J1);
    (J->isOutJack()) ? InsideJacks.append(dynamic_cast<CInJack*>(J1)) : InsideJacks.append(dynamic_cast<CInJack*>(J));
}

void CDesktopComponent::removeJack(IJack* jack, int PolyIndex){
    DeviceList.disconnectJack(jack);
    DeviceList.removeJack(jack,PolyIndex);
    for (int i = 0; i < JackBar1.jackCount(); i++) {
        if (JackBar1.jack(i) == jack) JackBar1.jackRects.removeAt(i);
    }
    for (int i = 0; i < JackBar2.jackCount(); i++) {
        if (JackBar2.jack(i) == jack) JackBar2.jackRects.removeAt(i);
    }
}

void CDesktopComponent::reorderJackbarJacks(QList<IJack*>* jacksCreated) {
    QMutexLocker locker(&mutex);
    JackBar1.jackRects.clear();
    JackBar2.jackRects.clear();
    for (IJack* Jack : *jacksCreated) {
        (Jack->isInJack()) ? JackBar2.addJack(Jack) : JackBar1.addJack(Jack);
    }
}

void CDesktopComponent::removeDeviceJack(IJack* jack){
    DeviceList.disconnectJack(jack);
    DeviceList.removeJack(jack);
    updateDeviceJacks();
}

void CDesktopComponent::addDeviceJack(IJack *jack){
    DeviceList.addJack(jack);
    updateDeviceJacks();
}

void CDesktopComponent::updateDeviceJacks() {
    DrawConnections();
    connectionsChanged();
}

void CDesktopComponent::parameterChange(IDevice* device, const CParameter* parameter)
{
    if (device)
    {
        if (parameter)
        {
            QMutexLocker locker(&mutex);
            const int d = DeviceList.indexOfDevice(device);
            if (d > -1) {
                emit controlChanged(device,parameter);
                DeviceList.updateParameter(d,parameter);
            }
        }
        else {
            int i = DeviceList.indexOfDevice(device);
            if (i > -1) {
                qDebug() << "No parameter";
                Devices[i]->getPic();
                //qDebug() << "emit parameterschanged";
                emit parametersChanged(device);
                //qDebug() << "drawconnections";
                DrawConnections();
                //qDebug() << "after drawconnections";
            }
        }
    }
}

void CDesktopComponent::closeAutomation(IDevice* device) {
    emit requestCloseAutomation(device);
}

void CDesktopComponent::activate(IDevice *Device)
{
    SelectDevice(DeviceList.indexOfDevice(Device));
}

void CDesktopComponent::takeString(IDevice *Device, const int type, const QString &s)
{
    if (Device->deviceID()=="MIDIFilePlayer 1") qDebug() << type << s;
}

void CDesktopComponent::PluginMenuClicked(QString ClassName)
{
    qDebug() << "plugin menu" << ClassName;
    MainMenu->UndoMenu->addItem("Add Device");
    if (CDeviceComponent* D = addDevice(ClassName))
    {
        D->geometry.moveTopLeft(StartPoint);
        DrawConnections();
    }
}

void CDesktopComponent::MacroMenuClicked(QString ProgramName)
{
    const QStringList Names=ProgramName.split("&&&&&&");
    MainMenu->UndoMenu->addItem("Add Macro Device");
    if (CDeviceComponent* D = addDevice(Names[0]))
    {
        D->geometry.moveTopLeft(StartPoint);
        CParametersMenu::OpenPreset(D->device(),Names[1]);
        D->getPic();
        emit parametersChanged(D->device());
        SelectDevice(Devices.size()-1);
    }
}

void CDesktopComponent::editDeviceCaption() {
    if (m_DeviceIndex > -1) {
        if (m_LineEdit) {
            currentDevice()->setAlias(static_cast<QLineEdit*>(m_LineEdit->widget())->text());
            Scene.removeItem(m_LineEdit);
            m_LineEdit->deleteLater();
            m_LineEdit = nullptr;
            DrawConnections();
            emit parametersChanged(currentDevice());
        }
    }
}

CDeviceComponent* CDesktopComponent::addDevice(const QString &ClassName)
{
    CDeviceComponent* D = addDevice(ClassName,DeviceList.findFreeIndex(ClassName));
    if (D) emit deviceAdded(D->device());
    return D;
}

CDeviceComponent* CDesktopComponent::addDevice(const QString& ClassName, const int ID)
{
    QMutexLocker locker(&mutex);
    const int MenuIndex=CAddIns::indexOf(ClassName);
    qDebug() << "addDevice" << ClassName << MenuIndex << ID;
    if (MenuIndex<0) return nullptr;
    IDevice* D=DeviceList.createDevice(instancefn(MenuIndex),ID,m_MainWindow);
    qDebug() << D->deviceID() << "added";
    CDeviceComponent* DC = (D) ? addDeviceComponent(D,ClassName) : nullptr;
    return DC;
}

CDeviceComponent* CDesktopComponent::addDeviceComponent(IDevice *Device, const QString& ClassName)
{
    auto DC = new CDeviceComponent(Device,ClassName);
    Devices.append(DC);
    SelectDevice(Devices.size()-1);
    return DC;
}

void CDesktopComponent::RemoveDevice(IDevice* Device)
{
    const int Index=DeviceList.indexOfDevice(Device);
    if (Index==m_DeviceIndex) SelectDevice(-1);
    QMutexLocker locker(&mutex);
    emit deviceRemoved(Device);
    DeviceList.deleteDevice(Device);
    delete Devices.takeAt(Index);
}

void CDesktopComponent::clear()
{
    emit playStopped();
    SelectDevice(-1);
    QMutexLocker locker(&mutex);
    emit devicesCleared();
    DisconnectJackBar(JackBar1);
    DisconnectJackBar(JackBar2);
    DeviceList.clear();
    qDeleteAll(Devices);
    Devices.clear();
}

void CDesktopComponent::clearJacksCreated(){
    clear();
    qDeleteAll(JacksCreated);
    JacksCreated.clear();
    InsideJacks.clear();
}

void CDesktopComponent::DisconnectJackBar(CJackBar& JackBar)
{
    QMutexLocker locker(&mutex);
    for (int i=0;i<JackBar.jackCount();i++) DeviceList.disconnectJack(JackBar.jack(i)->jackID());
}

Qt::CursorShape CDesktopComponent::connectCursor(IJack* J1,IJack* J2)
{
    if (J1 != J2)
    {
        setToolTip(J2->captionX());
        return  ((J1->canConnectTo(J2)) && (!J1->isConnectedTo(J2))) ? Qt::PointingHandCursor : Qt::ForbiddenCursor;
    }
    setToolTip(QString());
    return Qt::OpenHandCursor;
}

void CDesktopComponent::SetConnectCursor(const QPoint& Pos)
{
    if (IJack* HoverJack=MouseOverJack(Pos))
    {
        QApplication::restoreOverrideCursor();
        QApplication::setOverrideCursor(connectCursor(DragJack,HoverJack));
        return;
    }
    QApplication::restoreOverrideCursor();
    setToolTip(QString());
}

void CDesktopComponent::ConnectDrop(const QPoint& Pos)
{
    if (Dragging)
    {
        MainMenu->UndoMenu->addItem("Add Connection");
        setToolTip(QString());
        Dragging=false;
        QApplication::restoreOverrideCursor();
        IJack* HoverJack=MouseOverJack(Pos);
        if (HoverJack) DeviceList.connect(HoverJack->jackID(),DragJack->jackID());
        DrawConnections();
        emit connectionsChanged();
    }
}

void CDesktopComponent::hideRubberband() {
    if (m_LineEdit) {
        delete m_LineEdit;
        m_LineEdit = nullptr;
    }
    if (Rubberband->isVisible()) Rubberband->hide();
    MarkList.clear();
    MainMenu->EditMenu->setSelectionStatus(canCopy());
    DrawConnections();
}

void CDesktopComponent::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    hideRubberband();
}

bool CDesktopComponent::selectedDeviceIsValid() const
{
    return ((m_DeviceIndex>-1) && (m_DeviceIndex < Devices.size()));
}

bool CDesktopComponent::canCopy() const {
    return selectedDeviceIsValid() || !MarkList.isEmpty();
}

void CDesktopComponent::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx,dy);
    hideRubberband();
}

void CDesktopComponent::changeZoom(const double zoom)
{
    hideRubberband();
    emit zoomChanged(zoom);
}

void CDesktopComponent::setZoom(double zoom)
{
    zoomer->setZoom(zoom);
    changeZoom(zoom);
}

bool CDesktopComponent::findSuffix(const QString &path, const QString &filter) {
    const QStringList l = filter.split(" ");
    for (const QString& s : l) if (path.endsWith(s,Qt::CaseInsensitive)) return true;
    return false;
}

bool CDesktopComponent::initWithFile(const QString &path, QPoint pos) {
    QString ClassName;
    if (findSuffix(path,".wav .mp3 .m4a .mp4 .flac .ogg .au .aif .aiff .aifc .aup")) ClassName = "WaveRecorder";
    if (path.endsWith(".mid",Qt::CaseInsensitive)) ClassName = "MIDIFile2Wave";
    if (findSuffix(path,".mus .mxl .musicxml")) ClassName = "ObjectComposer";
    if (!ClassName.isEmpty()) {
        if (CDeviceComponent* D = addDevice(ClassName))
        {
            MainMenu->UndoMenu->addItem("Add " + ClassName);
            D->geometry.moveTopLeft(QGraphicsView::mapToScene(mapFromGlobal(pos)).toPoint());
            DeviceList.connect(D->device()->deviceID() + " Out","This Out");
            D->device()->initWithFile(path);
            D->device()->execute(true);
            SelectDevice(Devices.size()-1);
            return true;
        }
    }
    return false;
}

void CDesktopComponent::DrawConnections()
{
    Scene.clear();
    m_LineEdit = nullptr;
    rotateItem = nullptr;
    QList<QGraphicsItem*> items;

    QRect MaxRect;
    for (const CDeviceComponent* d : std::as_const(Devices)) MaxRect=MaxRect.united(d->geometry);
    MaxRect=MaxRect.united(QRect((QPoint(0,0)),QGraphicsView::mapToScene(rect().bottomRight()).toPoint()));
    setSceneRect(MaxRect);
    JackBar1.geometry=QRect(0,0,MaxRect.width(),CJackBar::height);
    items.append(JackBar1.paint(&Scene));
    JackBar2.geometry=QRect(0,MaxRect.height()-CJackBar::height,MaxRect.width(),CJackBar::height);
    items.append(JackBar2.paint(&Scene));
    //Scene.addRect(0,CJackBar::height+shadowOffset.y(),shadowOffset.x(),MaxRect.height()-(CJackBar::height+shadowOffset.y()),Qt::NoPen,QBrush(shadowColor));
    int x = 0;
    for (int i = 100; i > 0; i = i - 10) {
        Scene.addLine(x,x + CJackBar::height,x,MaxRect.height()-CJackBar::height,QColor(0,0,0,i));
        Scene.addLine(x,x + CJackBar::height,MaxRect.width(),x + CJackBar::height,QColor(0,0,0,i));
        x++;
    }
    QList<CJackContainer*> paintedContainers;
    paintedContainers.append(&JackBar1);
    paintedContainers.append(&JackBar2);

    if (isVisible()) {
        for (int i = 0; i < Devices.size(); i++) {
            if (!Devices[i]->frontPixSet()) {
                QPixmap p;
                emit requestParametersPixmap(Devices[i]->device(),&p);
                if (!p.isNull()) {
                    Devices[i]->setFrontPix(p);
                    //Devices[i]->frontPix = QDPRPixmap(p.realSize() * 0.199,p,Qt::KeepAspectRatio);
                }
            }
        }
    }

    for (int i = 0; i < Devices.size(); i++) {
        if (i!=m_DeviceIndex) {
            items.append(Devices[i]->paint(&Scene));
            if (Devices[i]->view() == CDeviceComponent::FrontView) {
                DrawDeviceConnections(Devices[i],paintedContainers);
            }
            else {
                items.append(DrawDeviceConnections(Devices[i],paintedContainers));
            }
        }
    }

    if (selectedDeviceIsValid()) {
        items.append(currentDeviceComponent()->paint(&Scene));
        if (currentDeviceComponent()->view() == CDeviceComponent::FrontView) {
            DrawDeviceConnections(currentDeviceComponent(),paintedContainers);
        }
        else {
            items.append(DrawDeviceConnections(currentDeviceComponent(),paintedContainers));
        }
    }
    for(QGraphicsItem* i : items) i->setZValue(1);
}

QList<QGraphicsItem*> CDesktopComponent::DrawDeviceConnections(CDeviceComponent* Device,QList<CJackContainer*>& paintedContainers)
{
    QList<QGraphicsItem*> items;
    for (int j = 0; j < Device->jackCount(); j++) {
        for (const CJackContainer* k : paintedContainers) {
            for (int l = 0; l < k->jackCount(); l++) {
                if (Device->jack(j)->isConnectedTo(k->jack(l))) {
                    const QPoint Pos1=k->jackPos(l);
                    const QPoint Pos2=Device->jackPos(j);
                    if (!Device->geometry.contains(Pos1)) {
                        QColor c(Device->jack(j)->JackColor());
                        c.setAlphaF(CPresets::presets().ConnectionsOpacity);
                        if (Device->jack(j)->isInJack()) {
                            items.append(CConnectionHelper::DrawArrow(Pos1,Pos2,c,&Scene));
                            CConnectionHelper::DrawArrow(Pos1+shadowOffset,Pos2+shadowOffset,shadowColor,&Scene);
                        }
                        else {
                            items.append(CConnectionHelper::DrawArrow(Pos2,Pos1,c,&Scene));
                            CConnectionHelper::DrawArrow(Pos2+shadowOffset,Pos1+shadowOffset,shadowColor,&Scene);
                        }
                    }
                }
            }
        }
    }
    paintedContainers.append(Device);
    return items;
}

void CDesktopComponent::serializeDevice(IDevice* d, const QRect& geometry, QDomLiteElement* xml) const
{
    QDomLiteElement* Device = xml->appendChild("Device");
    Device->setAttribute("Index",d->index());
    Device->setAttribute("Top",geometry.top());
    Device->setAttribute("Left",geometry.left());
    Device->setAttribute("Type",d->name());
    Device->setAttribute("ClassName",QString(d->name()+".dll"));
    d->serializeDevice(Device);
}

void CDesktopComponent::serializeConnection(CInJack* jack, QDomLiteElement* xml) const
{
    for (int i = 0; i < jack->outJackCount(); i++)
    {
        QDomLiteElement* Connection = xml->appendChild("Connection","InJack",jack->jackID());
        Connection->setAttribute("OutJack",jack->outJack(i)->jackID());
    }
}

void CDesktopComponent::undoSerialize(QDomLiteElement* xml) const {
    serialize(xml);
}

void CDesktopComponent::serialize(QDomLiteElement* xml) const
{
    QDomLiteElement* Items=xml->appendChild("Items");
    for (const CDeviceComponent* dc : Devices) serializeDevice(dc->device(), dc->geometry, Items);
    emit requestSerializeAutomationXML(Items);
    for (int i = 0; i < DeviceList.inJackCount(); i++) serializeConnection(DeviceList.inJack(i),Items);
    if (m_ParentWindow)
    {
        QDomLiteElement* Position = Items->appendChild("Position");
        Position->setAttribute("Top",m_ParentWindow->pos().y());
        Position->setAttribute("Left",m_ParentWindow->pos().x());
        Position->setAttribute("Height",m_ParentWindow->height());
        Position->setAttribute("Width",m_ParentWindow->width());
        Position->setAttribute("Visible",m_ParentWindow->isVisible());
    }
}

QPair<QString,QString> CDesktopComponent::unserializeDevice(const QDomLiteElement* xml, const QPoint& StartPoint, bool ReIndex)
{
    QPair<QString,QString> r;
    const QString Name=xml->attribute("Type");
    int Index=xml->attributeValueInt("Index");
    if (ReIndex)
    {
        r.first=Name + " " + QString::number(Index);
        Index=DeviceList.findFreeIndex(Name);
        r.second=Name + " " + QString::number(Index);
    }
    qDebug() << CAddIns::addInNames();
    if (CDeviceComponent* D = addDevice(Name,Index))
    {
        IDevice* d = D->device();
        emit deviceAdded(d);
        D->geometry.moveTopLeft(QPoint(xml->attributeValueInt("Left"),xml->attributeValueInt("Top"))+StartPoint);
        DeviceList.unserializeDevice(xml,d);
        D->getPic();
        emit parametersChanged(d);
    }
    return r;
}

void CDesktopComponent::unserializeConnection(const QDomLiteElement* xml, const QList<QPair<QString,QString>>& ReIndexer)
{
    QString InJack=xml->attribute("InJack");
    QString OutJack=xml->attribute("OutJack");
    for (const QPair<QString,QString>& i : ReIndexer)//(int i=0; i < ReIndexer.size(); i++)
    {
        if (InJack.contains(i.first)) InJack=i.second + InJack.mid(i.first.length());
        if (OutJack.contains(i.first)) OutJack=i.second + OutJack.mid(i.first.length());
    }
    DeviceList.connect(InJack,OutJack);
}

void CDesktopComponent::undoUnserialize(const QDomLiteElement* xml) {
    unserialize(xml);
}

void CDesktopComponent::unserialize(const QDomLiteElement* xml)
{
    if (!xml) return;
    QMutexLocker locker(&mutex);
    clear();
    if (const QDomLiteElement* Items=xml->elementByTag("Items"))
    {
        for(const QDomLiteElement* XMLDevice : (const QDomLiteElementList)Items->elementsByTag("Device")) unserializeDevice(XMLDevice);
        emit requestUnserializeAutomationXML(Items);
        for(const QDomLiteElement* XMLConnection : (const QDomLiteElementList)Items->elementsByTag("Connection")) unserializeConnection(XMLConnection);
        if (m_ParentWindow)
        {
#ifdef Q_OS_IOS
            m_ParentWindow->showFullScreen();
#else
            if (const QDomLiteElement* XMLPosition=Items->elementByTag("Position"))
            {
                m_ParentWindow->move(QPoint(XMLPosition->attributeValueInt("Left"),XMLPosition->attributeValueInt("Top")));
                m_ParentWindow->resize(QSize(XMLPosition->attributeValueInt("Width"),XMLPosition->attributeValueInt("Height")));
                m_ParentWindow->setVisible(XMLPosition->attributeValueBool("Visible"));
            }
#endif
        }
    }
    SelectDevice(0);
    emit connectionsChanged();
}

void CDesktopComponent::CloseDoc() {
    emit playStopped();
    clear();
}

void CDesktopComponent::OpenDoc(QString path)
{
    QMutexLocker l(&mutex);
    unserialize(CProjectPage::openFile(path).documentElement);
    emit MilliSecondsChanged();
    SelectDevice(0);
}

void CDesktopComponent::SaveDoc(QString path)
{
    QFileInfo f(path);
    QString p = _DocumentPath + f.baseName() + ".zip";
    QDomLiteDocument Doc("ObjectStudioProject","Custom");
    serialize(Doc.documentElement);
    CProjectPage::saveFile(p,&Doc,this->grab());
}

int CDesktopComponent::DeviceIndex(const QPoint& Pos) const
{
    if (selectedDeviceIsValid()) {
        if (currentDeviceComponent()->contains(Pos)) return m_DeviceIndex;
    }
    for (int i = Devices.size() - 1; i >= 0; i--) {
        if (Devices[i]->contains(Pos)) return i;
    }
    return -1;
}

void CDesktopComponent::hideForms()
{
    DeviceList.hideForms();
}

IJack* CDesktopComponent::MouseOverJack(const QPoint &Pos)
{
    QPoint dummy;
    return MouseOverJack(Pos,dummy);
}

int CDesktopComponent::MouseOverRotateButton(const QPoint &Pos) {
    for (int i = 0; i < Devices.size(); i++) {
        if (QRect(Devices[i]->geometry.topRight() - QPoint(12,12),QSize(24,24)).contains(Pos)) return i;
    }
    return -1;
}

IJack* CDesktopComponent::MouseOverJack(const QPoint& Pos, QPoint& JackPoint)
{
    int JackIndex = JackBar1.jackIndex(Pos);
    if (JackIndex > -1)
    {
        JackPoint=JackBar1.jackPos(JackIndex);
        return JackBar1.jack(JackIndex);
    }
    JackIndex = JackBar2.jackIndex(Pos);
    if (JackIndex > -1)
    {
        JackPoint=JackBar2.jackPos(JackIndex);
        return JackBar2.jack(JackIndex);
    }
    if (selectedDeviceIsValid())
    {
        JackIndex = currentDeviceComponent()->jackIndex(Pos);
        if (JackIndex > -1)
        {
            JackPoint=currentDeviceComponent()->jackPos(JackIndex);
            return currentDevice()->jack(JackIndex);
        }
    }
    for(CDeviceComponent* dc : std::as_const(Devices))
    {
        JackIndex = dc->jackIndex(Pos);
        if (JackIndex > -1)
        {
            JackPoint=dc->jackPos(JackIndex);
            return dc->device()->jack(JackIndex);
        }
    }
    return nullptr;
}

void CDesktopComponent::SelectDevice(IDevice* d)
{
    if (d) SelectDevice(DeviceList.indexOfDevice(d));
}

void CDesktopComponent::SelectDevice(const int Index)
{
    m_DeviceIndex=Index;
    for (CDeviceComponent* d : std::as_const(Devices)) d->setSelected(false);
    if (Index > -1)
    {
        if (Devices.isEmpty()) {
            SelectDevice(-1);
            return;
        }
        Devices[Index]->setSelected(true);
        Devices[Index]->getPic();
        emit parametersChanged(Devices[Index]->device());
        MainMenu->EditMenu->setSelectionStatus(canCopy());
        Devices[Index]->device()->raiseForm();
    }
    DrawConnections();
}

void CDesktopComponent::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "mousePress" << event->pos();
    Rubberband->hide();
    if (m_LineEdit) {
        delete m_LineEdit;
        m_LineEdit = nullptr;
    }
    MarkList.clear();
    MainMenu->EditMenu->setSelectionStatus(canCopy());
    Dragging=false;
    DragJack=nullptr;
    MouseDown=true;
    StartPoint=QGraphicsView::mapToScene(event->pos().x(),event->pos().y()).toPoint();
    int DI = DeviceIndex(StartPoint);
    int MR = MouseOverRotateButton(StartPoint);
    if (DI == -1) DI = MR;
    if (DI > -1) {
        if (m_DeviceIndex != DI) SelectDevice(DI);
        if (MR == DI) {
            int v = currentDeviceComponent()->view();
            v++;
            if (v > CDeviceComponent::Drawing) v = 0;
            currentDeviceComponent()->setView((CDeviceComponent::DeviceView)v);
            DrawConnections();
        }
    }
    DragJack=MouseOverJack(StartPoint,DragJackPos);
    if (DragJack)
    {
        if (event->button()==Qt::LeftButton)
        {
            QApplication::restoreOverrideCursor();
            QApplication::setOverrideCursor(Qt::OpenHandCursor);
            Dragging=true;
            return;
        }
        if (event->button()==Qt::RightButton)
        {
            CConnectionsMenu* m = new CConnectionsMenu(DragJack,&DeviceList,this);
            connect(m,&CConnectionsMenu::aboutToChange,MainMenu->UndoMenu,&CUndoMenu::addItem,Qt::DirectConnection);
            connect(m,&CConnectionsMenu::connectionsChanged,this,&CDesktopComponent::connectionsChanged);
            connect(m,&CConnectionsMenu::connectionsChanged,this,&CDesktopComponent::DrawConnections);
            m->popup(mapToGlobal(event->pos()));
            MouseDown=false;
            return;
        }
    }
    if (DI > -1)
    {
        if (event->button()==Qt::LeftButton)
        {
            m_MD=true;
            QApplication::restoreOverrideCursor();
            QApplication::setOverrideCursor(Qt::PointingHandCursor);
            Start=StartPoint;
            StartPos=Devices[DI]->geometry.topLeft();
            MouseDown=false;
            DragBackup = new QDomLiteElement("UndoItem");
            serialize(DragBackup);
            return;
        }
        if (event->button()==Qt::RightButton)
        {
            QMenu* DeviceMenu=new QMenu(this);
            DeviceMenu->setAttribute(Qt::WA_DeleteOnClose);
            DeviceMenu->addActions(MainMenu->EditMenu->actions());
            DeviceMenu->addSeparator();
            CParametersMenu* pm = new CParametersMenu(Devices[DI]->device(),this);
            connect(pm,&CParametersMenu::aboutToChange,MainMenu->UndoMenu,&CUndoMenu::addItem,Qt::DirectConnection);
            connect(pm,&CParametersMenu::parametersChanged,this,&CDesktopComponent::parametersChanged);
            DeviceMenu->addMenu(pm);
            DeviceMenu->addSeparator();
            DeviceMenu->addAction("Disconnect",this,&CDesktopComponent::RemoveConnections);
            DeviceMenu->addAction("Show/Hide UI",this,&CDesktopComponent::toggleUI);
            DeviceMenu->popup(mapToGlobal(event->pos()));
            MouseDown=false;
            return;
        }
    }
    else
    {
        if (event->button()==Qt::RightButton)
        {
            MouseDown=false;
            QSignalMenu* PluginsPopup=new QSignalMenu("New Device",this);
            connect(PluginsPopup,qOverload<QString>(&QSignalMenu::menuClicked),this,&CDesktopComponent::PluginMenuClicked);
            const QStringList plugs=CAddIns::addInNames();
            for (const QString& p : plugs) PluginsPopup->addAction(p,p);
            QMenu* MacrosPopup=new QMenu("Saved Device",this);
            for (const QString& p : plugs)
            {
                const QStringList macros = CProgramBank::programNames(p);
                QSignalMenu* MacroDevicePopup=nullptr;
                if (!macros.isEmpty())
                {
                    if (!MacroDevicePopup)
                    {
                        MacroDevicePopup=new QSignalMenu(p,this);
                        MacrosPopup->addMenu(MacroDevicePopup);
                        connect(MacroDevicePopup,qOverload<QString>(&QSignalMenu::menuClicked),this,&CDesktopComponent::MacroMenuClicked);
                    }
                    for (const QString& s: macros)
                    {
                        MacroDevicePopup->addAction(s,p + "&&&&&&" + s);
                    }
                }
            }

            QMenu* DesktopMenu=new QMenu(this);
            DesktopMenu->setAttribute(Qt::WA_DeleteOnClose);
            DesktopMenu->addActions(MainMenu->EditMenu->actions());
            DesktopMenu->addSeparator();
            DesktopMenu->addMenu(PluginsPopup);
            DesktopMenu->addMenu(MacrosPopup);
            DesktopMenu->addSeparator();
            DesktopMenu->addActions(MainMenu->FileMenu->actions());
            DesktopMenu->addSeparator();
            DesktopMenu->addMenu(MainMenu->RecentMenu);
            DesktopMenu->popup(mapToGlobal(event->pos()));
            return;
        }
    }
}

void CDesktopComponent::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint Pos = QGraphicsView::mapToScene(event->pos().x(),event->pos().y()).toPoint();
    if (Pos == MousePos) return;
    MousePos=Pos;
    if (m_MD) //drag device
    {
        if (Pos != Start)
        {
            currentDeviceComponent()->geometry.moveTopLeft(StartPos+(Pos-StartPoint));
            Start=Pos;
            DrawConnections();
        }
        return;
    }
    if (Dragging) //drag connection
    {
        SetConnectCursor(Pos);
        DragList.clear();
        DrawConnections();
        QColor c(DragJack->JackColor());
        c.setAlphaF(CPresets::presets().ConnectionsOpacity);
        if (DragJack->isOutJack())
        {
            DragList.append(CConnectionHelper::DrawArrow(DragJackPos,Pos,c,&Scene));
            for (QGraphicsItem* i : std::as_const(DragList)) i->setZValue(3);//for (int i=0;i<DragList.size();i++) DragList[i]->setZValue(3);
            DragList.append(CConnectionHelper::DrawArrow(DragJackPos+shadowOffset,Pos+shadowOffset,shadowColor,&Scene));
        }
        else
        {
            DragList.append(CConnectionHelper::DrawArrow(Pos,DragJackPos,c,&Scene));
            for (QGraphicsItem* i : std::as_const(DragList)) i->setZValue(3);//for (int i=0;i<DragList.size();i++) DragList[i]->setZValue(3);
            DragList.append(CConnectionHelper::DrawArrow(Pos+shadowOffset,DragJackPos+shadowOffset,shadowColor,&Scene));
        }
        return;
    }
    const IJack* HoverJack = MouseOverJack(Pos);
    (HoverJack) ? setToolTip(HoverJack->captionX()) : setToolTip(QString());
    if (MouseDown) //rubberband
    {
        if (DeviceIndex(Pos) == -1)
        {
            Rubberband->setGeometry(this->rect());
            Rubberband->setWindowGeometry(mapFromScene(QRect(StartPoint,Pos).normalized()));
            if (!Rubberband->isVisible()) Rubberband->show();
        }
    }
    else {
        int i = MouseOverRotateButton(Pos);
        if (i > -1) {
            if (rotateItem == nullptr) {
                rotateItem = Scene.addPixmap(QDPRPixmap(QSize(24,24),":/Rotate.png"));
                rotateItem->setPos(Devices[i]->geometry.topRight() - QPoint(12,12));
                rotateItem->setZValue(2);
            }
        }
        else {
            if (rotateItem) {
                Scene.removeItem(rotateItem);
                delete rotateItem;
                rotateItem = nullptr;
            }
        }
    }
}

void CDesktopComponent::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << "mouseRelease" << event->pos();
    MouseDown=false;
    const QPoint Pos=QGraphicsView::mapToScene(event->pos().x(),event->pos().y()).toPoint();
    if (DragBackup) {
        if (Pos != StartPos) MainMenu->UndoMenu->addElement(DragBackup,"Drag");
        delete DragBackup;
        DragBackup = nullptr;
    }
    if (event->button()==Qt::LeftButton)
    {
        if (m_MD)
        {
            QApplication::restoreOverrideCursor();
            if (Pos != Start)
            {
                if ((m_DeviceIndex > -1) && (m_DeviceIndex < Devices.size())) {
                    currentDeviceComponent()->geometry.moveTopLeft(StartPos+(Pos-StartPoint));
                }
                Start=Pos;
                DrawConnections();
            }
            m_MD=false;
        }
    }
    ConnectDrop(Pos);
    if (Rubberband->isVisible())
    {
        MarkList.clear();
        QRect MarkRect(mapToScene(Rubberband->windowGeometry()));
        for(CDeviceComponent* d : std::as_const(Devices))
        {
            if (d->inside(MarkRect)) MarkList.append(d->device());
        }
        MainMenu->EditMenu->setSelectionStatus(canCopy());
        if (!MarkList.empty())
        {
            QMenu* MarkMenu=new QMenu(this);
            MarkMenu->setAttribute(Qt::WA_DeleteOnClose);
            MarkMenu->addActions(MainMenu->EditMenu->actions());
            MarkMenu->addSeparator();
            CPasteParametersAction* pp = new CPasteParametersAction(MarkList,MarkMenu);
            connect(pp,&CPasteParametersAction::aboutToChange,MainMenu->UndoMenu,&CUndoMenu::addItem,Qt::DirectConnection);
            MarkMenu->addAction(pp);
            MarkMenu->addSeparator();
            MarkMenu->addAction("Disconnect",this,&CDesktopComponent::RemoveConnections);
            MarkMenu->popup(mapToGlobal(event->pos()));
        }
    }
}

void CDesktopComponent::mouseDoubleClickEvent(QMouseEvent *event)
{
    Dragging=false;
    DragJack=nullptr;
    m_MD=false;
    QMutexLocker locker(&mutex);
    QApplication::restoreOverrideCursor();
    const QPoint Pos=QGraphicsView::mapToScene(event->pos().x(),event->pos().y()).toPoint();
    if (IJack* J=MouseOverJack(Pos))
    {
        MainMenu->UndoMenu->addItem("Disconnect");
        DeviceList.disconnectJack(J->jackID());
        setToolTip(J->captionX());
        DrawConnections();
        emit connectionsChanged();
        return;
    }
    SelectDevice(DeviceIndex(Pos));
    if (m_DeviceIndex > -1) {
        if (currentDeviceComponent()->captionRect.contains(Pos)) {
            QLineEdit* l = new QLineEdit();
            l->setFrame(false);
            l->setAttribute(Qt::WA_TranslucentBackground);
            m_LineEdit = Scene.addWidget(l);
            m_LineEdit->show();
            m_LineEdit->setZValue(3);
            QRect editRect = currentDeviceComponent()->geometry;
            editRect.setTop(currentDeviceComponent()->captionRect.top());
            editRect.setBottom(currentDeviceComponent()->captionRect.bottom());
            m_LineEdit->setGeometry(editRect);
            if (!currentDevice()->alias().isEmpty()) l->setText(currentDevice()->alias());
            l->setFocus();
            connect(l,&QLineEdit::editingFinished,this,&CDesktopComponent::editDeviceCaption);
            return;
        }
    }
    toggleUI();
}

void CDesktopComponent::NewDoc()
{
    clear();
    emit connectionsChanged();
    SelectDevice(0);
    emit MilliSecondsChanged();
}

void CDesktopComponent::DeleteDoc()
{
    MainMenu->UndoMenu->addItem("Delete");
    QMutexLocker locker(&mutex);
    if ((MarkList.isEmpty()) && (selectedDeviceIsValid())) MarkList.append(currentDevice());
    for(IDevice* d : std::as_const(MarkList)) RemoveDevice(d);
    SelectDevice(Devices.size()-1);
}

void CDesktopComponent::CopyDoc(QDomLiteElement* xml)
{
    if ((MarkList.empty()) && (selectedDeviceIsValid())) MarkList.append(currentDevice());
    if (!MarkList.empty())
    {
        QDomLiteElement* devices = xml->appendChild("Devices");
        for (IDevice* d : std::as_const(MarkList))
        {
            QRect r=Devices[DeviceList.indexOfDevice(d)]->geometry;
            (MarkList.size()>1) ? r.translate(-QGraphicsView::mapToScene(Rubberband->windowGeometry().topLeft()).toPoint()) : r.setTopLeft(zeroPoint);
            serializeDevice(d,r,devices);
        }
        for(IDevice* d : std::as_const(MarkList))
        {
            for (int i=0;i<d->inJackCount();i++) serializeConnection(d->inJack(i),devices);
        }
    }
}

void CDesktopComponent::PasteDoc(const QDomLiteElement* xml)
{
    if (QDomLiteElement* devices = xml->elementByTag("Devices")) {
        if (devices->childCount()) {
            MainMenu->UndoMenu->addItem("Paste");
            QList<QPair<QString,QString>> ReIndexer;
            for(const QDomLiteElement* XMLDevice : (const QDomLiteElementList)devices->elementsByTag("Device")) ReIndexer.append(unserializeDevice(XMLDevice, StartPoint, true));
            for(const QDomLiteElement* XMLConnection : (const QDomLiteElementList)devices->elementsByTag("Connection")) unserializeConnection(XMLConnection, ReIndexer);
        }
    }
    (!selectedDeviceIsValid()) ? SelectDevice(0) : DrawConnections();
    emit connectionsChanged();
}

void CDesktopComponent::RemoveConnections()
{
    if ((MarkList.isEmpty()) && (selectedDeviceIsValid())) MarkList.append(currentDevice());
    MainMenu->UndoMenu->addItem("Disconnect Device");
    for(IDevice* d : std::as_const(MarkList)) DeviceList.disconnectDevice(d);
    DrawConnections();
    emit connectionsChanged();
}

void CDesktopComponent::toggleUI()
{
    if (selectedDeviceIsValid()) currentDevice()->toggleUI();
    DrawConnections();
}

