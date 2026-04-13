#include "cwavelanes.h"
#include "ui_cwavelanes.h"
#include "cwaveeditwidget.h"
#include <QListWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QScrollArea>
#include "cautomationlane.h"
#include <QClipboard>
#include <QWidgetAction>
#include "ctimelineedit.h"
#include "qdprpixmap.h"

CWaveLanes::CWaveLanes(QWidget *parent) :
    QGraphicsView(parent),
    ui(new Ui::CWaveLanes)
{
    ui->setupUi(this);
    videoWindow = new CVideoDialog(this);

    zoomer = new QGraphicsViewZoomer(this);
    zoomer->disableMatrix();
    zoomer->setMin(0.0001);
    zoomer->setMax(1);
    zoomer->setZoom(0.001L);
    connect(zoomer,&QGraphicsViewZoomer::ZoomChanged,this,&CWaveLanes::ZoomToCursor);

    setScene(&Scene);
    setSceneRect(0,0,1,1);
    setAlignment(Qt::AlignTop | Qt::AlignLeft);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    Scene.setItemIndexMethod(QGraphicsScene::NoIndex);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::TextAntialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setMouseTracking(true);

    m_Mixer=nullptr;
    setAcceptDrops(true);
    InfoLabel=new QLabel(this);
    InfoLabel->setAutoFillBackground(true);
    InfoLabel->setFrameStyle(QFrame::Box | QFrame::Plain);
    InfoLabel->hide();
    CurrentLane=-1;
    m_OldDragLane=-1;
    m_OldDragTrack=-1;
    Loading=false;
    m_TimerID=0;
    connect(horizontalScrollBar(),&QAbstractSlider::valueChanged,this,&CWaveLanes::paint);
    connect(horizontalScrollBar(),&QAbstractSlider::valueChanged,this,&CWaveLanes::UpdateAutomationGeometry);
    connect(verticalScrollBar(),&QAbstractSlider::valueChanged,this,&CWaveLanes::paint);
    connect(verticalScrollBar(),&QAbstractSlider::valueChanged,this,&CWaveLanes::UpdateAutomationGeometry);
    setBackgroundBrush(QDPRPixmap(":/Brushed Aluminium 3 Tile.bmp"));
    m_sidebarItem = new CWaveLanesSidebar(BorderWidth,130);
    Scene.addItem(m_sidebarItem);
}

CWaveLanes::~CWaveLanes()
{
    if (m_TimerID) killTimer(m_TimerID);
    m_TimerID = 0;
    closeAutomation();
    for (CWaveLane* L : std::as_const(lanes)) {
        if (L->hasVisible()) {
            if (videoWindow) videoWindow->removeVideo(L->videoItem);
        }
        deviceList.deleteDevice(L);
    }
    if (videoWindow) delete videoWindow;
    lanes.clear();
    if (m_Mixer != nullptr) {
        m_Mixer->removerEffectRacksFromDeviceList(&deviceList);
        deviceList.deleteDevice(m_Mixer);
    }
    deviceList.clear();
    Effects.clear();
    delete ui;
}

void CWaveLanes::init(const int Index, QWidget* MainWindow)
{
    m_Name = "WaveLanes";
    IDevice::init(Index,MainWindow);
    IDevice::addTickerDevice(&deviceList);

    MainMenu->EditMenu->addSeparator();
    SplitAction = MainMenu->EditMenu->addAction("Split",this,&CWaveLanes::Split);
    EditTrackAction = MainMenu->EditMenu->addAction("Edit Track",this,&CWaveLanes::EditTrack);
    QMenu* QuantizeMenu = MainMenu->EditMenu->addMenu("Quantize");
    QuantizeStraightAction = QuantizeMenu->addAction("Straight",this,&CWaveLanes::QuantizeStraight);
    QuantizeTripletAction = QuantizeMenu->addAction("Triplet",this,&CWaveLanes::QuantizeTriplet);
    MainMenu->EditMenu->addSeparator();
    VideoTrackAction=MainMenu->EditMenu->addAction("Track Video Visible",this,&CWaveLanes::ToggleTrackVideo);
    VideoTrackAction->setCheckable(true);
    MainMenu->EditMenu->addSeparator();
    InsertLaneAction=MainMenu->EditMenu->addAction("Insert Lane",this,&CWaveLanes::InsertLane);
    AddLaneAction=MainMenu->EditMenu->addAction("Add Lane",this,&CWaveLanes::AddLane);
    RemoveLaneAction=MainMenu->EditMenu->addAction("Remove Lane",this,&CWaveLanes::RemoveLane);
    AutomationAction = MainMenu->EditMenu->addAction("Show Automation",this,&CWaveLanes::Automation);
    EditLaneAction = MainMenu->EditMenu->addAction("Edit Lane",this,&CWaveLanes::EditLane);
    EditLaneAction->setCheckable(true);
    EffectRackAction = MainMenu->EditMenu->addAction("Show EffectRack",this,&CWaveLanes::EffectRack);
    MainMenu->EditMenu->addSeparator();
    VideoWidgetAction=MainMenu->EditMenu->addAction("Lane Video Visible",this,&CWaveLanes::ToggleLaneVideo);
    VideoWidgetAction->setCheckable(true);
    for (int i=Effects.size();i<3;i++) {
        Effects.append(dynamic_cast<CDeviceContainer*>(deviceList.addDevice(new CDeviceContainer("Effect"),i+1,m_MainWindow)));
    }
    AddLaneInternal();
    m_TimeLine.setOffset(BorderWidth);
    paint();
    execute(true);
}

bool CWaveLanes::event(QEvent *event)
{
    if (event->type() == QEvent::Leave) {
        unsetCursor();
        if ((m_OldDragLane > -1) && (m_OldDragTrack > -1))
        {
            lanes[m_OldDragLane]->paintTrack(m_OldDragTrack,Scene,zoomer->getZoom(),zoomer->visibleRect().toRect(),0);
            m_OldDragLane = -1;
            m_OldDragTrack = -1;
        }
    }
    return QGraphicsView::event(event);
}

ulong64 CWaveLanes::pos2Sample(int Pos) const
{
    return ulong64(ldouble(qMax(Pos-BorderWidth,0))/zoomer->getZoom());
}

int CWaveLanes::sample2Pos(ulong64 sample) const
{
    return int((ldouble(sample)*zoomer->getZoom())+BorderWidth);
}

void CWaveLanes::zoomIn()
{
    setZoom(zoomer->getZoom() * 2);
}

void CWaveLanes::zoomOut()
{
    if (ldouble(sceneRect().width()-((BorderWidth*2)+LaneTrail)) * 0.5L < viewport()->width()-((BorderWidth*2)+LaneTrail)) {
        setZoom(zoomer->getZoom() * ldouble(viewport()->width()-((BorderWidth*2)+LaneTrail))/ldouble(sceneRect().width()-((BorderWidth*2)+LaneTrail)));
    }
    else {
        setZoom(zoomer->getZoom() * 0.5L);
    }
}

void CWaveLanes::zoomMin()
{
    setZoom(zoomer->getZoom() * ldouble(viewport()->width()-((BorderWidth*2)+LaneTrail))/ldouble(sceneRect().width()-((BorderWidth*2)+LaneTrail)));
}

void CWaveLanes::zoomMax()
{
    setZoom(1);
}

void CWaveLanes::setZoom(double z)
{
    if (z < 0) z = 1;
    if (!closeEnough(zoomer->getZoom(),z)) zoomer->setZoom(z);
    paint();
    zoomer->scrollXTo(m_TimeLine.currentPos()-(zoomer->visibleRect().width()/2.0));
    UpdateAutomationGeometry();
    Scene.invalidate(zoomer->visibleRect(),QGraphicsScene::ForegroundLayer);
}

void CWaveLanes::ZoomToCursor(double z, double o){
    const QPointF p = viewport()->mapFromGlobal(QCursor::pos());
    const ulong64 s = ((zoomer->visibleRect().left() + p.x()) - BorderWidth) / o;
    paint();
    zoomer->scrollXTo(sample2Pos(s - (p.x() / z)));
    UpdateAutomationGeometry();
    Scene.invalidate(zoomer->visibleRect(),QGraphicsScene::ForegroundLayer);

}

void CWaveLanes::timerEvent(QTimerEvent *)
{
    if (!m_TimerID) return;
    m_TimeLine.handleTimer(this);
    if (requestIsPlaying()) {
        const int x = m_TimeLine.currentPos();
        if (x < zoomer->visibleRect().left() || x > zoomer->visibleRect().right())
        {
            if (m_EditLane > -1) {
                requestSkip(m_TimeLine.sampleFromX(zoomer->scrollValueX()));
            }
            else {
                zoomer->scrollXTo(m_TimeLine.currentPos());
            }
        }
    }
}

CAudioBuffer* CWaveLanes::getNextA(const int ProcIndex)
{
    if (m_Mixer != nullptr) return m_Mixer->getNextA(ProcIndex+CStereoMixer::jnOut);
    return nullptr;
}

void CWaveLanes::play(const bool FromStart)
{
    qDebug() << "Wavelanes play";
    if (FromStart) reset();
    IDevice::play(FromStart);
    m_TimerID=startTimer(50);
}

void CWaveLanes::pause()
{
    stop();
    IDevice::pause();
}

void CWaveLanes::reset()
{
    for (CWaveLane* l : std::as_const(lanes)) l->reset();
}

void CWaveLanes::stop()
{
    if (m_TimerID) killTimer(m_TimerID);
    m_TimerID=0;
}

void CWaveLanes::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    paint();
}

void CWaveLanes::UpdateGeometry() {
    for (int i = 0; i < lanes.size(); i++)
    {
        for(int j = 0; j < lanes[i]->tracks.size(); j++) {
            lanes[i]->tracks[j]->isActive = ((i == CurrentLane) && (CurrentTrack.contains(j)));
        }
    }
    if (m_EditLane < 0) {
        for (int i = 0; i < lanes.size(); i++) {
            CWaveLane* Lane=lanes[i];
            Lane->geometry=QRect(sample2Pos(0),BorderWidth+RulerHeight+(LaneHeight*i),LaneTrail,LaneHeight-LaneGap);
            Lane->UpdateGeometry(zoomer->getZoom(),LaneTrail);
        }
    }
    else {
        int y = RulerHeight + BorderWidth;
        for (int i = 0; i < lanes.size(); i++) {
            CWaveLane* Lane=lanes[i];
            if (m_EditLane == i) {
                Lane->geometry=QRect(sample2Pos(0),y,LaneTrail,height() - ((lanes.size()*14) + RulerHeight + (BorderWidth * 2)));
            }
            else {
                Lane->geometry=QRect(sample2Pos(0),y,LaneTrail,10);
            }
            Lane->UpdateGeometry(zoomer->getZoom(),LaneTrail);
            y += Lane->geometry.height() + LaneGap;
        }
    }
    setEditMenu();
}

void CWaveLanes::paint()
{
    QMutexLocker locker(&mutex);
    Scene.removeItem(m_sidebarItem);
    QGraphicsItemList l(ProxyItems());
    l.removeFromScene(&Scene);
    Scene.clear();
    l.addToScene(&Scene);
    Scene.addItem(m_sidebarItem);

    UpdateGeometry();
    const ulong64 timelineSamples = requestSamples();
    long MaxLen = sample2Pos(timelineSamples) + LaneTrail;
    QSize NewSize(BorderWidth+MaxLen,(lanes.size()*LaneHeight)+(BorderWidth*2)+RulerHeight);
    QRectF r = sceneRect();
    if (NewSize != sceneRect().size()) {
        r.setSize(NewSize);
        setSceneRect(r);
        zoomer->setMin(zoomer->getZoom() * ldouble(viewport()->width()-((BorderWidth*2)+LaneTrail))/ldouble(sceneRect().width()-((BorderWidth*2)+LaneTrail)));
        m_TimeLine.setFixedWidth(sample2Pos(timelineSamples) - BorderWidth, timelineSamples);
    }
    for (int i = 0; i < lanes.size(); i++) {
        lanes[i]->paint(Scene,zoomer->getZoom(),zoomer->visibleRect().toRect(),i == CurrentLane);
    }
    m_TimeLine.setPen(QPen(Qt::black));
    m_TimeLine.render(&Scene,zoomer->visibleRect().toRect());
    if (InfoLabel->isVisible()) {
        QPoint p(mapToScene(QCursor::pos()).toPoint());
        int L=MouseOverLane(p);
        ShowInfoLabel(pos2Sample(p.x()),L);
    }
    m_sidebarItem->setHeight(qMax(sceneRect().bottom(),zoomer->visibleRect().bottom()));
    m_sidebarItem->setPos(zoomer->visibleRect().left(),0);
    for (int i = 0; i < lanes.size(); i++) {
        CWaveLaneSidebarItem* g = &lanes[i]->sideBarItem;
        g->disconnect();
        g->setParentItem(m_sidebarItem);
        g->setEnabled((m_EditLane < 0) || (m_EditLane == i));
        g->setPos(lanes[i]->geometry.topLeft());
        g->nameEdit.setText(lanes[i]->alias());
        g->setVisible(m_sidebarItem->isOpen() && g->isEnabled());
        connect(g,&CWaveLaneSidebarItem::changed,this,&CWaveLanes::sidebarItemChanged);
    }
}

bool CWaveLanes::fileInUse(const QString& Filename)
{
    QMutexLocker locker(&mutex);
    for (CWaveLane* l : std::as_const(lanes)) if (l->fileInUse(Filename)) return true;
    return false;
}

const QStringList CWaveLanes::fileList()
{
    QStringList li;
    for (CWaveLane* l : std::as_const(lanes)) li.append(l->fileList());
    return li;
}

void CWaveLanes::renameFile(const QString& oldName, const QString& newName)
{
    QMutexLocker locker(&mutex);
    for (CWaveLane* l : std::as_const(lanes)) l->renameFile(oldName,newName);
}

void CWaveLanes::removeFile(const QString &Filename)
{
    QMutexLocker locker(&mutex);
    for (CWaveLane* l : std::as_const(lanes)) l->removeFile(Filename);
    paint();
}

void CWaveLanes::serialize(QDomLiteElement* xml) const
{
    for (int i = 0; i < lanes.size(); i++) {
        lanes[i]->serialize(xml->appendChild("Lane" + QString::number(i+1))->appendChild("Lane"));
    }
    QDomLiteElement* Mixer=xml->appendChild("Mixer");
    m_MixerWidget->serialize(Mixer);
    for (int i = 0; i < Effects.size(); i++) {
        Effects[i]->serializeCustom(Mixer->appendChild("Effect"+QString::number(i))->appendChild("Custom"));
    }
    xml->setAttribute("CurrentLane",CurrentLane);
    QString s;
    for (const int& i : CurrentTrack) s.append(QString::number(i)+"&");
    xml->setAttribute("CurrentTrack",s);
    if (videoWindow) videoWindow->serialize(xml);
    m_TimeLine.serialize(xml);
}

void CWaveLanes::unserialize(const QDomLiteElement* xml)
{
    QMutexLocker locker(&mutex);
    qDebug() << "WaveLanes unserialize";
    requestPause();
    Loading=true;
    closeAutomation();
    if (m_Mixer) {
        m_Mixer->removerEffectRacksFromDeviceList(&deviceList);
        deviceList.deleteDevice(m_Mixer);
        m_Mixer=nullptr;
        while (!m_MixerWidget->channels.empty()) m_MixerWidget->removeChannel();
    }
    for (CWaveLane* L : std::as_const(lanes)) {
        if (L->hasVisible()) videoWindow->removeVideo(L->videoItem);
        deviceList.deleteDevice(L);
    }
    lanes.clear();
    for (CDeviceContainer* d : std::as_const(Effects)) d->ClearDevice();
    if (xml) {
        int i = 0;
        while (const QDomLiteElement* Lane = xml->elementByTag("Lane" + QString::number(++i))) {
            AddLaneInternal();
            CWaveLane* L=lanes.last();
            L->unserialize(Lane->elementByTag("Lane"),zoomer->getZoom());
        }
        if (const QDomLiteElement* Mixer = xml->elementByTag("Mixer")) {
            m_MixerWidget->unserialize(Mixer);
            for (int i = 0; i < m_MixerWidget->channels.size(); i++) {
                if (m_MixerWidget->channels[i]->ID.isEmpty()) {
                    m_MixerWidget->channels[i]->ID = LaneID(i);
                }
            }
            for (int i = 0; i < 3; i++) {
                if (const QDomLiteElement* e = Mixer->elementByTag("Effect"+QString::number(i))) {
                    Effects[i]->unserializeCustom(e->elementByTag("Custom"));
                }
            }
        }
        CurrentLane = xml->attributeValueInt("CurrentLane",CurrentLane);
        QStringList l = xml->attribute("CurrentTrack","").split("&",Qt::SkipEmptyParts);
        CurrentTrack.clear();
        for (const QString& s : std::as_const(l)) CurrentTrack.append(s.toInt());
        if (videoWindow) videoWindow->unserialize(xml);
        m_TimeLine.unserialize(xml);
    }
    if (lanes.isEmpty()) AddLaneInternal();
    paint();
    setEditMenu();
    Loading=false;
}

void CWaveLanes::skip(const ulong64 samples)
{
    reset();
    IDevice::skip(samples);
    m_TimeLine.skip(samples);
}

int CWaveLanes::MouseOverLane(QPoint Pos)
{
    for(int i = 0; i < lanes.size(); i++) if (lanes[i]->geometry.contains(Pos)) return i;
    return -1;
}

int CWaveLanes::MouseOverTrack(QPoint Pos, int Lane)
{
    if (Lane > -1) {
        return lanes[Lane]->MouseOverTrack(Pos);
    }
    return -1;
}

void CWaveLanes::QuantizeStraight()
{
    if (canCopy()) {
        MainMenu->UndoMenu->addItem("Quantize Straight");
        for (const int& i : std::as_const(CurrentTrack)) {
            const ulong64 Start = lanes[CurrentLane]->tracks[i]->start;
            lanes[CurrentLane]->tracks[i]->start = m_TimeLine.beat2Sample(qRound(m_TimeLine.sample2Beat(Start,2)),2);
        }
        paint();
    }
}

void CWaveLanes::QuantizeTriplet()
{
    if (canCopy()) {
        MainMenu->UndoMenu->addItem("Quantize Triplet");
        for (const int& i : std::as_const(CurrentTrack)) {
            const ulong64 Start = lanes[CurrentLane]->tracks[i]->start;
            lanes[CurrentLane]->tracks[i]->start = m_TimeLine.beat2Sample(qRound(m_TimeLine.sample2Beat(Start,3)),3);
        }
        paint();
    }
}

void CWaveLanes::ShowInfoLabel(ulong64 Start,int Lane) {
    if (Lane > -1) ShowInfoLabel(Start, lanes[Lane]);
}

void CWaveLanes::ShowInfoLabel(ulong64 Start, CWaveLane* Lane)
{
    const int Top = Lane->geometry.bottom();
    const ldouble mSecs = presets.samplesTomSecs(Start);
    InfoLabel->hide();
    setFontSizeScr(InfoLabel,11);
    InfoLabel->setText("Sample: "+QString::number(Start)+" \nTime: "+m_TimeLine.timeToText(mSecs,CTimeLine::TimeLineMilliseconds)+" \nBar: " + m_TimeLine.timeToText(mSecs,CTimeLine::TimelineBars));
    InfoLabel->move(sample2Pos(Start)-zoomer->visibleRect().left(),Top);//,fm.horizontalAdvance("Sample "+QString::number(Start)+" \n")+4,(fm.height()*3)+4);
    InfoLabel->adjustSize();
    InfoLabel->show();
}

void CWaveLanes::mouseDoubleClickEvent(QMouseEvent *event)
{
    const QPointF scenePos = mapToScene(event->pos());
    if (m_sidebarItem->isOpen()) {
        QGraphicsView::mouseDoubleClickEvent(event);
        return;
    }
    if (automationVisible(scenePos)) {
        QGraphicsView::mouseDoubleClickEvent(event);
        return;
    }

    const QPoint Pos=mapToScene(event->pos()).toPoint();
    if (event->button()==Qt::LeftButton) {
        if (m_TimeLine.handleDoubleClick(Pos,this)) return;
    }
    const int Lane = MouseOverLane(Pos);
    const int Track = MouseOverTrack(Pos,Lane);
    if ((Lane != CurrentLane) || (!CurrentTrack.contains(Track))) {
        CurrentLane = Lane;
        CurrentTrack.clear();
        if (Track > -1) CurrentTrack.append(Track);
        paint();
    }
    if (CurrentLane > -1) {
        EditLane();
    }
}

void CWaveLanes::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    m_TimeLine.drawPlayLine(painter);
}

void CWaveLanes::UpdateEditTrack(CWaveGenerator::LoopParameters LP)
{
    m_EditTrack->loopParameters = LP;
}

void CWaveLanes::EditTrack() {
    if (canCopy()) {
        MainMenu->UndoMenu->addItem("Edit Track");
        QDialog d(this,Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);
        d.resize(600,200);
        auto e = new CWaveEditWidget(&d);
        connect(e,&CWaveEditWidget::Changed,this,&CWaveLanes::UpdateEditTrack);
        m_EditTrack = lanes[CurrentLane]->tracks[CurrentTrack.first()];
        auto l = new QHBoxLayout(&d);
        l->setContentsMargins(0,0,0,0);
        l->setSpacing(0);
        d.setLayout(l);
        d.setWindowTitle(QFileInfo(m_EditTrack->name).baseName());
        d.setGeometry(QRect(mapToGlobal(this->geometry().topLeft()),this->size()));
        l->addWidget(e);
        e->Init(&m_EditTrack->waveGenerator,m_EditTrack->loopParameters,false);
        e->ZoomRegion();
        d.exec();
        paint();
        setEditMenu();
    }
}

void CWaveLanes::updateVideoWindow(int Lane) {
    if (m_Playing) return;
    if (Lane == -1) Lane = CurrentLane;
    if (Lane > -1) lanes[CurrentLane]->skip(requestCurrentSample());
}

void CWaveLanes::ToggleLaneVideo() {
    if (CurrentLane > -1) {
        lanes[CurrentLane]->videoVisible = !lanes[CurrentLane]->videoVisible;
        updateVideoWindow();
    }
    setEditMenu();
}

void CWaveLanes::ToggleTrackVideo() {
    if (CurrentLane > -1) {
        for (int t : std::as_const(CurrentTrack)) {
            lanes[CurrentLane]->tracks[t]->videoVisible = !lanes[CurrentLane]->tracks[t]->videoVisible;
        }
        updateVideoWindow();
    }
    setEditMenu();
}

void CWaveLanes::EditLane() {
    if (m_EditLane > -1) {
        m_EditLane = -1;
        setZoom(m_EditZoom);
        paint();
    }
    else if (CurrentLane > -1) {
        if (m_EditLane < 0) {
            m_TimeLine.setCurrentSample(pos2Sample(mapToScene(mapFromGlobal(cursor().pos())).x()));
            m_EditZoom = zoomer->getZoom();
            setZoom(0.1);
        }
        m_EditLane = CurrentLane;
        paint();
    }
}

void CWaveLanes::setEditMenu() {
    QuantizeStraightAction->setEnabled(canCopy());
    QuantizeTripletAction->setEnabled(canCopy());
    RemoveLaneAction->setEnabled((CurrentLane>-1) & (lanes.size()>1));
    SplitAction->setEnabled(canCopy());
    AutomationAction->setEnabled(CurrentLane>-1);
    EditTrackAction->setEnabled(canCopy());
    VideoWidgetAction->setEnabled(canVideo());
    if ((CurrentLane > -1) && (CurrentLane < lanes.size())) {
        if (lanes[CurrentLane]->videoItem) {
            VideoWidgetAction->setChecked(lanes[CurrentLane]->videoVisible);
        }
    }
    VideoTrackAction->setEnabled(trackCanVideo());
    if ((CurrentLane > -1) && (CurrentLane < lanes.size())) {
        if (!CurrentTrack.isEmpty()) {
            VideoTrackAction->setChecked(lanes[CurrentLane]->tracks[CurrentTrack.first()]->videoVisible);
        }
    }
    EditLaneAction->setEnabled(CurrentLane>-1);
    InsertLaneAction->setEnabled(CurrentLane>-1);
    EffectRackAction->setEnabled(CurrentLane>-1);
    MainMenu->EditMenu->setSelectionStatus(canCopy());
    EditLaneAction->setChecked(m_EditLane > -1);
}

void CWaveLanes::exportLaneAudio(const QString &filename) {
    if (m_Mixer) {
        int tempSolo = m_Mixer->SoloChannel;
        m_Mixer->SoloChannel = CurrentLane;
        IDevice::exportWave(filename);
        m_Mixer->SoloChannel = tempSolo;
    }
}

void CWaveLanes::exportAudio(const QString &filename) {
    IDevice::exportWave(filename);
}

void CWaveLanes::exportVideo(const QString &filename) {
    if (!videoWindow) return;
    const QSize outputSize(videoWindow->outputSize());
    const qreal frameRate = 30;
    QFile(filename).remove();

    QGraphicsScene* tempScene = videoWindow->scene();
    videoWindow->setScene(nullptr);
    CChannelBuffer* audio = IDevice::render();

    VideoExporter exporter(filename, outputSize, frameRate, presets.SampleRate, 2);

    ulong64 mSec = 0;
    for (CWaveLane* l : std::as_const(lanes)) {
        ulong64 ms = l->milliSeconds();
        if (ms > mSec) mSec = ms;
    }
    const ulong64 totalFrames = frameRate * mSec / 1000.0;

    //QImage img(outputSize, QImage::Format_ARGB32);
    QImage img(outputSize, QImage::Format_ARGB32_Premultiplied);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    for (CWaveLane* l : std::as_const(lanes)) {
        if (l->videoItem) {
            l->videoItem->setRenderRect(videoWindow->resolution());
            l->videoItem->setEnabled(l->videoVisible);
        }
    }
    abortExport = false;
    CVideoProgressWindow exportProgress;
    exportProgress.setMax(totalFrames);
    exportProgress.setVisible(true);
    connect(&exportProgress,&CVideoProgressWindow::abort,
            this,
            [this]()
            {
                abortExport = true;
            });

    setExportMode(true);

    CChannelBuffer frameBuffer(presets.SampleRate / frameRate,2);
    ulong64 sample = 0;
    for (ulong64 f = 0; f < totalFrames; ++f)
    {
        double t = f / frameRate;
        exportProgress.setValue(f);
        getExportFrame(t);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

        img.fill(Qt::black);
        for (CWaveLane* l : std::as_const(lanes)) {
            if (l->videoItem) {
                if (l->videoItem->isEnabled()) l->videoItem->paint(&p,nullptr,nullptr);
            }
        }
        exporter.addFrame(img,f);
        frameBuffer.copy(*audio,sample);
        std::vector<float>b = frameBuffer.toInterleaved();
        sample += frameBuffer.size();
        exporter.addAudio(b.data());
        if (abortExport) break;
    }

    QEventLoop loop;
    exporter.finish([&](){
        loop.quit();
    });
    loop.exec();

    delete audio;

    exportProgress.setVisible(false);

    setExportMode(false);

    if (abortExport) {
        QFile(filename).remove();
        abortExport = false;
    }
    videoWindow->setScene(tempScene);
}

void CWaveLanes::EffectRack() {
    if (CurrentLane > -1) {
        m_MixerWidget->channels[CurrentLane]->toggleEffectRack();
    }
}

bool CWaveLanes::canCopy() { return (!CurrentTrack.isEmpty()) && (CurrentLane > -1); }

bool CWaveLanes::canVideo() { if (CurrentLane > -1) {
        if (lanes[CurrentLane]->hasVisible()) return true;
    }
    return false;
}

bool CWaveLanes::trackCanVideo() {
    if (CurrentLane > -1 ) {
        if (!CurrentTrack.isEmpty()) {
            if (lanes[CurrentLane]->tracks[CurrentTrack.first()]->hasVideo()) {
                return true;
            }
        }
    }
    return false;
}

bool CWaveLanes::trackIsImage() {
    if (CurrentLane > -1 ) {
        if (!CurrentTrack.isEmpty()) {
            if (lanes[CurrentLane]->tracks[CurrentTrack.first()]->hasImage()) {
                return true;
            }
        }
    }
    return false;
}

void CWaveLanes::setExportMode(bool m) {
    for (CWaveLane* l : std::as_const(lanes)) {
        if (l->videoItem) l->setExportMode(m);
    }
}

void CWaveLanes::getExportFrame(double t) {
    for (CWaveLane* l : std::as_const(lanes)) {
        if (l->videoItem) l->setExportTime(t);
    }
}

void CWaveLanes::sidebarItemChanged(CWaveLaneSidebarItem *item){
    m_sidebarItem->setPopup(false);
    if (!m_sidebarItem->isUnderMouse()) m_sidebarItem->collapse();
    int index = CurrentLane;
    for (int i = 0; i < lanes.size(); i++) {
        if (lanes[i]->geometry.topLeft() == item->pos().toPoint()) {
            index = i;
            break;
        }
    }
    if (index < 0) return;
    CWaveLane* lane = lanes[index];
    CSF2ChannelWidget* channel = m_MixerWidget->channels[index];

    const QString s = item->nameEdit.text();
    if (lane->alias() != s) {
        lane->setAlias(s);
        if (lane->videoItem) lane->videoItem->name = s;
        channel->setName(s);
    }

    const bool videoMuted = item->videoMuteButton.isChecked();
    if (lane->videoItem) {
        if (videoMuted) {
            if (lane->videoVisible) {
                lane->videoVisible = !videoMuted;
                updateVideoWindow(index);
            }
        }
        if (!videoMuted) {
            if (!lane->videoVisible) {
                lane->videoVisible = !videoMuted;
                updateVideoWindow(index);
            }
        }
    }

    const bool audioMuted = item->muteButton.isChecked();
    if (audioMuted) {
        if (!channel->effectsPanel->muted()) {
            channel->effectsPanel->setMute(audioMuted);
        }
    }
    if (!audioMuted) {
        if (channel->effectsPanel->muted()) {
            channel->effectsPanel->setMute(audioMuted);
        }
    }

    const bool audioSolo = item->soloButton.isChecked();
    if (audioSolo) {
        if (m_MixerWidget->soloChannel() != index) {
            m_MixerWidget->setSoloChannel(index);
            channel->effectsPanel->setSolo(audioSolo);
        }
    }
    if (!audioSolo) {
        if (m_MixerWidget->soloChannel() == index) {
            m_MixerWidget->setSoloChannel(-1);
            channel->effectsPanel->setSolo(audioSolo);
        }
    }

    const bool automation = item->automationButton.isChecked();
    if (automation) {
        if (!automationWidget(index)) {
            Automation(index);
            UpdateAutomationGeometry();
        }
    }
    if (!automation) {
        if (automationWidget(index)) {
            automationWidget(index)->close();
        }
    }
    qDebug() << "automationwidgets" << ProxyWidgets().size();
}

void CWaveLanes::mousePressEvent(QMouseEvent *event)
{
    StartPos = mapToScene(event->pos()).toPoint();
    if (m_sidebarItem->isOpen()) {
        QGraphicsItem* item = Scene.itemAt(StartPos,transform());
        if (QGraphicsProxyWidget* w = qgraphicsitem_cast<QGraphicsProxyWidget*>(item)) {
            if (QLCDEdit* k = qobject_cast<QLCDEdit*>(w->widget())) {
                m_sidebarItem->setPopup(true);
                k->popupMenu(event->globalPosition().toPoint());
                return;
            }
        }
        QGraphicsView::mousePressEvent(event);
        return;
    }
    if (automationVisible(StartPos)) {
        QGraphicsView::mousePressEvent(event);
        return;
    }
    const int Lane = MouseOverLane(StartPos);
    const int Track = MouseOverTrack(StartPos,Lane);
    if (event->button()==Qt::LeftButton) {
        if (m_TimeLine.handleMousePress(StartPos,this)) return;
    }
    if (event->button()==Qt::RightButton) {
        if (StartPos.y() < timelineheight) {

            CTimeLineMenu* d = new CTimeLineMenu(&m_TimeLine,this);
            connect(d,&CTimeLineMenu::Changed,this,&CWaveLanes::paint);
            d->popup(cursor().pos());
            return;
        }
    }
    if ((Lane != CurrentLane) || (!CurrentTrack.contains(Track)))
    {
        CurrentLane = Lane;
        if (Lane > -1) {
            if (m_EditLane > -1) m_EditLane = Lane;
        }
        if (!event->modifiers().testFlag(Qt::ControlModifier)) CurrentTrack.clear();
        if (Track > -1) {
            if (!CurrentTrack.contains(Track)) CurrentTrack.append(Track);
        }
        paint();
        if ((CurrentLane > -1) && (Track > -1)) {
            lanes[CurrentLane]->paintEdges(StartPos,Track,Scene,zoomer->getZoom(),zoomer->visibleRect().toRect());
        }
    }
    if ((CurrentLane > -1) && (!CurrentTrack.isEmpty())) ShowInfoLabel(lanes[CurrentLane]->tracks[CurrentTrack.first()]->start,CurrentLane);
    if (event->button()==Qt::RightButton) {
        MainMenu->EditMenu->popup(event->globalPosition().toPoint());
        return;
    }
    if (CurrentLane > -1) {
        qDebug() << Lane << Track << CurrentLane << lanes[CurrentLane]->DragTracks << CurrentTrack;
        lanes[CurrentLane]->DragTracks = CurrentTrack;
        qDebug() << Lane << Track << CurrentLane << lanes[CurrentLane]->DragTracks << CurrentTrack;
        long64 s = lanes[CurrentLane]->handleMousePress(StartPos);
        if (s > -1) {
            ShowInfoLabel(s,CurrentLane);
            lanes[CurrentLane]->drawOutsideWave(Scene,zoomer->visibleRect().toRect());
            DragBackup = new QDomLiteElement("UndoItem");
            serialize(DragBackup);
            if (lanes[CurrentLane]->DragTrackEdge == CWaveLane::NoEdge) {
                MD = true;
                setCursor(Qt::OpenHandCursor);
            }
        }
    }
    //QGraphicsView::mousePressEvent(event);
}

void CWaveLanes::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint Pos = mapToScene(event->pos()).toPoint();
    if (m_sidebarItem->isOpen()) {
        InfoLabel->hide();
        unsetCursor();
        QGraphicsView::mouseMoveEvent(event);
        return;
    }
    if (automationVisible(Pos)) {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }
    if (m_TimeLine.handleMouseMove(Pos,this)) return;
    if (CurrentLane > -1) {
        long64 s = lanes[CurrentLane]->handleMouseMove(Pos,&m_TimeLine);
        if (s > -1) {
            paint();
            lanes[CurrentLane]->drawOutsideWave(Scene,zoomer->visibleRect().toRect());
            for (const int& i : (const QList<int>)CurrentTrack) lanes[CurrentLane]->paintTrack(i,Scene,zoomer->getZoom(),zoomer->visibleRect().toRect(),-1);
            ShowInfoLabel(s,CurrentLane);
            return;
        }
    }
    const int LaneIndex = MouseOverLane(Pos);
    const int TrackIndex = MouseOverTrack(Pos,LaneIndex);
    if (LaneIndex > -1) {
        ShowInfoLabel(pos2Sample(Pos.x()),LaneIndex);
        if (TrackIndex > -1) {
            if ((TrackIndex != m_OldDragTrack) && (m_OldDragTrack > -1)) {
                lanes[LaneIndex]->paintTrack(m_OldDragTrack,Scene,zoomer->getZoom(),zoomer->visibleRect().toRect(),0);
            }
            if (lanes[LaneIndex]->paintEdges(Pos,TrackIndex,Scene,zoomer->getZoom(),zoomer->visibleRect().toRect())) {
                m_OldDragTrack = TrackIndex;
                m_OldDragLane = LaneIndex;
                setCursor(Qt::SizeHorCursor);
                return;
            }
        }
    }
    else {
        InfoLabel->hide();
    }
    if (!MD) unsetCursor();
    if ((m_OldDragLane > -1) && (m_OldDragTrack > -1)) {
        lanes[m_OldDragLane]->paintTrack(m_OldDragTrack,Scene,zoomer->getZoom(),zoomer->visibleRect().toRect(),0);
        m_OldDragLane = -1;
        m_OldDragTrack = -1;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void CWaveLanes::mouseReleaseEvent(QMouseEvent *event)
{
    const QPoint Pos = mapToScene(event->pos()).toPoint();
    if (m_sidebarItem->isOpen()) {
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }
    if (automationVisible(Pos)) {
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }
    if (MD) {
        MD = false;
        unsetCursor();
    }
    if (DragBackup) {
        if (Pos != StartPos) MainMenu->UndoMenu->addElement(DragBackup,"Drag");
        delete DragBackup;
        DragBackup = nullptr;
    }
    if (m_TimeLine.handleMouseRelease(Pos, this)) return;
    if (CurrentLane > -1) {
        CWaveLane* Lane=lanes[CurrentLane];
        CWaveTrack* t = Lane->handleMouseRelease();
        for (const int& i : std::as_const(CurrentTrack)) {
            if (CWaveTrack* t1 = Lane->tracks[i]) {
                Lane->sanityCheck(t1);
                if (!Lane->tracks[i]->isValid) CurrentTrack.removeOne(i);
            }
        }
        paint();
        if (t) {
            int TrackIndex = Lane->tracks.indexOf(t);
            if (TrackIndex > -1) Lane->paintEdges(Pos,TrackIndex,Scene,zoomer->getZoom(),zoomer->visibleRect().toRect());
        }
    }
    //QGraphicsView::mouseReleaseEvent(event);
}


void CWaveLanes::CopyDoc(QDomLiteElement* xml)
{
    QMutexLocker locker(&mutex);
    if (!lanes.isEmpty()) {
        if (canCopy()) {
            QApplication::clipboard()->clear();
            QDomLiteElement* e = xml->appendChild("Tracks");
            for (const int&i : (const QList<int>)CurrentTrack) {
                QDomLiteElement* t = e->appendChild("Track");
                lanes[CurrentLane]->serializeTrack(t,lanes[CurrentLane]->tracks[i]);
            }
        }
    }
}

void CWaveLanes::PasteDoc(const QDomLiteElement* xml)
{
    QMutexLocker locker(&mutex);
        if (!lanes.isEmpty()) {
            if (CurrentLane > -1) {
                MainMenu->UndoMenu->addItem("Paste");
                ulong64 Start = m_TimeLine.currentSample();//pos2Sample(StartPos.x());
                QDomLiteElement* tracks = xml->elementByTag("Tracks");
                ulong64 f = 0;
                int i = 0;
                for (QDomLiteElement* e : (const QDomLiteElementList)tracks->elementsByTag("Track")) {
                    if (i == 0) f = e->attributeValueULongLong("StartPoint");
                    i++;
                    if (CWaveTrack* t = lanes[CurrentLane]->unserializeTrack(e,zoomer->getZoom())) {
                        t->start = (e->attributeValueULongLong("StartPoint")-f)+Start;
                        lanes[CurrentLane]->sanityCheck(t);
                    }
                }
                paint();
                setEditMenu();
            }
        }
}

void CWaveLanes::Split()
{
    if (!lanes.isEmpty()) {
        if (canCopy()) {
            MainMenu->UndoMenu->addItem("Split");
            CWaveTrack* t = lanes[CurrentLane]->tracks[CurrentTrack.first()];
            const ulong64 s = m_TimeLine.currentSample();
            lanes[CurrentLane]->cloneTrack(t,zoomer->getZoom())->cropStart(s);
            t->cropEnd(s);
            paint();
        }
    }
}

QGraphicsProxyWidget *CWaveLanes::addProxyWidget(QWidget *a, int lane) {
    a->resize(lanes[lane]->geometry.adjusted(0,0,-50,0).size());
    QGraphicsProxyWidget* w = Scene.addWidget(a);
    w->setZValue(5);
    return w;
}

QList<QGraphicsItem *> CWaveLanes::ProxyItems() const {
    QList<QGraphicsItem*> l;
    const QGraphicsItemList g(Scene.items());
    for (QGraphicsItem* i : g) {
        if (auto proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(i)) {
            if (proxy->zValue() > 4) {
                if (qobject_cast<CAutomationLane*>(proxy->widget())) {
                    l.append(i);
                }
            }
        }
    }
    return l;
}

QList<QWidget *> CWaveLanes::ProxyWidgets() const {
    QList<QWidget*> l;
    for (QGraphicsItem* i : ProxyItems()) {
        l.append(qgraphicsitem_cast<QGraphicsProxyWidget*>(i)->widget());
    }
    return l;
}

bool CWaveLanes::automationVisible(const QPointF &scenePos) {
    const QGraphicsItem* w = Scene.itemAt(scenePos,transform());
    if (ProxyItems().contains(w)) return true;
    return false;
}

CAutomationLane *CWaveLanes::automationWidget(int lane) {
    for (QGraphicsItem* i : ProxyItems()) {
        if (i->pos().toPoint() == lanes[lane]->geometry.topLeft()) {
            return qobject_cast<CAutomationLane*>(qgraphicsitem_cast<QGraphicsProxyWidget*>(i)->widget());
        }
    }
    return nullptr;
}

void CWaveLanes::Automation(int lane) {
    int l = -1;
    if (lane > -1) {
        l = lane;
    }
    else {
        if (CurrentLane > -1) l = CurrentLane;
    }
    if (l > -1) {
        if (!lanes[l]->tracks.isEmpty()) {
            CAutomationLane* a = new CAutomationLane();
            a->fill(lanes[l],0,&deviceList,false);
            QGraphicsProxyWidget* w = addProxyWidget(a,l);
            w->setPos(lanes[l]->geometry.adjusted(0,0,-50,0).topLeft());
        }
    }
}

void CWaveLanes::UpdateAutomationGeometry() {
    for (int i = 0; i < lanes.size(); i++) {
        if (CAutomationLane* a = automationWidget(i)) {
            a->resize(lanes[i]->geometry.adjusted(0,0,-50,0).size());
        }
    }
}

void CWaveLanes::closeAutomation() {
    for (QWidget* w : ProxyWidgets()) {
        qobject_cast<CAutomationLane*>(w)->close();
    }
    qDebug() << "closeAutomation" << ProxyWidgets().size();
}

void CWaveLanes::DeleteDoc()
{
    MainMenu->UndoMenu->addItem("Delete Track");
    for (const int& i : (const QList<int>)CurrentTrack) RemoveTrackAt(CurrentLane, i);
    setEditMenu();
}

void CWaveLanes::RemoveTrackAt(int Lane, int Track)
{
    QMutexLocker locker(&mutex);
    if (!lanes.isEmpty()) {
        if (canCopy()) {
            const QString FN = lanes[Lane]->tracks[Track]->name;
            delete lanes[Lane]->tracks.takeAt(Track);
            for (int i = CurrentTrack.size() - 1; i >= 0; i--) {
                if (CurrentTrack[i] >= Track) CurrentTrack[i]--;
                if (CurrentTrack[i] < 0) CurrentTrack.removeAt(i);
            }
            lanes[Lane]->destroyVideoWidget();
            //setEditMenu();
            paint();
            const ulong64 samples = requestSamples();
            if (requestCurrentSample() > samples) {
                requestPause();
                requestSkip(samples);
            }
            emit FileRemoved(FN);
        }
    }
}

void CWaveLanes::updateMixer()
{
    if (m_Mixer != nullptr) m_Mixer->setDisabled(true);
    m_MixerWidget->stop();
    for (int i = 0; i < m_MixerWidget->channels.count(); i++) qDebug() << "Channel ID 1" << m_MixerWidget->channels[i]->ID;
    mutex.lock();
    deviceList.disconnectAll();
    if (!m_Mixer) {
        m_Mixer=new CStereoMixer(0,3);
        deviceList.addDevice(m_Mixer,1,nullptr);
    }
    m_MixerWidget->hide();
    m_MixerWidget->hideMaster();
    for (int i = 0; i < m_MixerWidget->channels.count(); i++) qDebug() << "Channel ID 2" << m_MixerWidget->channels[i]->ID;
    if (lanes.size()!=int(m_Mixer->channelCount())) {
        QDomLiteElement channelXML;
        for (uint i = 0; i < m_Mixer->channelCount(); i++) {
            QDomLiteElement* e = new QDomLiteElement("Channel");
            m_MixerWidget->channels[i]->serialize(e);
            channelXML.appendChild(e);
        }
        qDebug() << "save" << channelXML.toString();
        m_Mixer->removerEffectRacksFromDeviceList(&deviceList);
        deviceList.deleteDevice(m_Mixer);
        m_Mixer = new CStereoMixer(lanes.size(),3);
        deviceList.addDevice(m_Mixer,1,nullptr);
        m_Mixer->addEffectRacksToDeviceList(&deviceList,m_MainWindow);
        while (m_MixerWidget->channels.size()>lanes.size()) m_MixerWidget->removeChannel();
        while (m_MixerWidget->channels.size()<lanes.size()) m_MixerWidget->appendChannel();
        for (int i = 0; i < lanes.size(); i++) {
            CSF2ChannelWidget* ch = m_MixerWidget->channels[i];
            ch->init(m_Mixer->channels[i],lanes[i]->alias());
            qDebug() << "Adding channel" << lanes[i]->ID << i;
            for (QDomLiteElement* e : std::as_const(channelXML.childElements)) {
                if (e->attribute("ID") == lanes[i]->ID) {
                    ch->unserialize(e);
                    if (e->attributeValueBool("Solo")) m_Mixer->SoloChannel = i;
                    break;
                }
            }
            ch->ID = lanes[i]->ID;
            ch->setVisible(true);
            lanes[i]->parameters[0]->connectToWidget(ch->volSlider,&CChannelVol::volChanged,&CChannelVol::setVol);
            lanes[i]->parameters[1]->connectToWidget(ch->effectsPanel,&CChannelEffects::panValueChanged,&CChannelEffects::setPanValue);
        }
        channelXML.clearChildren();
    }
    for (int i = 0; i < m_MixerWidget->channels.count(); i++) qDebug() << "Channel ID 3" << m_MixerWidget->channels[i]->ID;
    for (int i = 0; i < lanes.size(); i++) {
        deviceList.connect("StereoMixer 1 In " + QString::number(i+1),lanes[i]->deviceID() + " Out");
    }
    for (int i = 0; i < 3; i++) {
        deviceList.connect("Effect "+ QString::number(i+1) +" In","StereoMixer 1 Send "+ QString::number(i+1));
        deviceList.connect("StereoMixer 1 Return "+ QString::number(i+1),"Effect "+ QString::number(i+1) +" Out");
    }
    m_MixerWidget->showMaster(m_Mixer,&Effects);
    mutex.unlock();
    m_Mixer->setDisabled(false);
    m_MixerWidget->start();
    m_MixerWidget->show();
    for (int i = 0; i < m_MixerWidget->channels.count(); i++) qDebug() << "Channel ID 4" << m_MixerWidget->channels[i]->ID;
}

void CWaveLanes::ShowMixer()
{
    execute(true);
}

void CWaveLanes::AddLane()
{
    MainMenu->UndoMenu->addItem("Add Lane");
    AddLaneInternal();
    paint();
    setEditMenu();
}

void CWaveLanes::InsertLane()
{
    MainMenu->UndoMenu->addItem("Insert Lane");
    AddLaneInternal(CurrentLane);
    paint();
    setEditMenu();
}

void CWaveLanes::AddLaneInternal(int index) {
    QMutexLocker locker(&mutex);
    for (int i = 0; i < m_MixerWidget->channels.count(); i++) qDebug() << "Channel ID addlaneinternal" << m_MixerWidget->channels[i]->ID;
    auto L = new CWaveLane;
    L->videoDialog = videoWindow;
    deviceList.addDevice(L,lanes.size()+1,m_MainWindow);
    int i = 0;
    QStringList IDList;
    for (const CWaveLane* l : std::as_const(lanes)) IDList.append(l->ID);
    while (IDList.contains(LaneID(i))) i++;
    L->ID = LaneID(i);
    L->setAlias(L->ID);
    if (index > -1) {
        lanes.insert(index,L);
        CurrentLane = index;
    }
    else {
        lanes.append(L);
        CurrentLane = lanes.size() - 1;
    }
    updateMixer();
    CurrentTrack.clear();
    setEditMenu();
}

void CWaveLanes::RemoveLane()
{
    QMutexLocker locker(&mutex);
    if (!lanes.isEmpty())
    {
        if (CurrentLane>-1)
        {
            MainMenu->UndoMenu->addItem("Delete Lane");
            closeAutomation();
            if (lanes[CurrentLane]->hasVisible()) {
                videoWindow->removeVideo(lanes[CurrentLane]->videoItem);
            }
            deviceList.deleteDevice(lanes[CurrentLane]);
            lanes.removeAt(CurrentLane);
            CurrentLane=-1;
            CurrentTrack.clear();
            updateMixer();
            const ulong64 samples = requestSamples();
            if (requestCurrentSample() > samples) {
                    requestPause();
                    requestSkip(samples);
            }
            paint();
            setEditMenu();
        }
    }
}

QString CWaveLanes::DropFileName(const QMimeData* d, const QObject* s) {
    qDebug() << d->urls() << d->html() << d->formats();
    if (!d->urls().isEmpty()) {
        QString path = d->urls().constFirst().toLocalFile();
        if (QFileInfo::exists(path)) return path;
    }
    const QString sender = s->metaObject()->className();
    if (!sender.compare("QListWidget")) {
        if (auto l = dynamic_cast<const QListWidget*>(s))
        {
            QListWidgetItem* i = l->item(l->currentRow());
            return i->data(34).toString();
        }
    }
    return QString();
}

void CWaveLanes::dragEnterEvent(QDragEnterEvent *e)
{
    //qDebug() << "Drag enter";
    if (!DropFileName(e->mimeData(),e->source()).isEmpty()) e->acceptProposedAction();
}

void CWaveLanes::dragMoveEvent(QDragMoveEvent* e) {
    //qDebug() << "Drag move";
    if (!DropFileName(e->mimeData(),e->source()).isEmpty()) e->acceptProposedAction();
}

void CWaveLanes::dropEvent(QDropEvent *e)
{
    qDebug() << "drop";
    QMutexLocker locker(&mutex);
    const QString path = DropFileName(e->mimeData(),e->source());
    if (path.isEmpty()) return;
    InfoLabel->hide();
    const QPoint currentPos = mapToScene(e->position().toPoint()).toPoint();
    const int Lane = MouseOverLane(currentPos);
    const int Track = MouseOverTrack(currentPos,CurrentLane);
    ulong64 Start=pos2Sample(currentPos.x());
    if (Lane < 0) {
        if (CurrentLane < 0) {
            AddLane();
            Start = 0;
        }
        else {
            Start = m_TimeLine.currentSample();
        }
    }
    else {
        CurrentLane = Lane;
        CurrentTrack.clear();
        if (Track > -1) CurrentTrack.append(Track);
        setEditMenu();
    }
    if (path.endsWith(".aup",Qt::CaseInsensitive)) {
        MainMenu->Recent(path);
        paint();
        e->acceptProposedAction();
        return;
    }
    if (AddFile(path, Start)) e->acceptProposedAction();
}

bool CWaveLanes::AddFile(QString FN,ulong64 Start) {
    auto T=new CWaveTrack(FN,Start);
    CWaveLane* L=lanes[CurrentLane];
    if (T->isValid) {
        MainMenu->UndoMenu->addItem("Add file");
        Loading=true;
        L->addFile(T);
        paint();
        Loading=false;
        emit FileAdded(FN);
        setEditMenu();
        return true;
    }
    delete T;
    return false;
}
