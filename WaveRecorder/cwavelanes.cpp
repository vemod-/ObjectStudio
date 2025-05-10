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

CWaveLanes::CWaveLanes(QWidget *parent) :
    QGraphicsView(parent),
    ui(new Ui::CWaveLanes)
{
    ui->setupUi(this);
    zoomer = new QGraphicsViewZoomer(this);
    zoomer->disableMatrix();
    zoomer->setMin(0.0001);
    zoomer->setMax(1);
    zoomer->setZoom(0.001L);
    connect(zoomer,&QGraphicsViewZoomer::ZoomChanged,this,&CWaveLanes::setZoom);

    setScene(&Scene);
    setSceneRect(0,0,1,1);
    setAlignment(Qt::AlignTop | Qt::AlignLeft);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
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
    rulerTempo=120;
    rulerBeats=4;
    MixFactor=1;
    Loading=false;
    m_TimerID=0;
    connect(horizontalScrollBar(),&QAbstractSlider::valueChanged,this,&CWaveLanes::paint);
    connect(horizontalScrollBar(),&QAbstractSlider::valueChanged,this,&CWaveLanes::UpdateAutomationGeometry);
    setBackgroundBrush(QPixmap(":/Brushed Aluminium 3 Tile.bmp"));
}

CWaveLanes::~CWaveLanes()
{
    if (m_TimerID) killTimer(m_TimerID);
    m_TimerID=0;
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
    AddLaneAction=MainMenu->EditMenu->addAction("Add Lane",this,&CWaveLanes::AddLane);
    RemoveLaneAction=MainMenu->EditMenu->addAction("Remove Lane",this,&CWaveLanes::RemoveLane);
    AutomationAction = MainMenu->EditMenu->addAction("Automation",this,&CWaveLanes::Automation);
    EditLaneAction = MainMenu->EditMenu->addAction("Edit Lane",this,&CWaveLanes::EditLane);
    EditLaneAction->setCheckable(true);
    EffectRackAction = MainMenu->EditMenu->addAction("EffectRack",this,&CWaveLanes::EffectRack);
    for (int i=Effects.size();i<3;i++)
    {
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
            lanes[m_OldDragLane]->paintTrack(m_OldDragTrack,Scene,zoomer->getZoom(),visibleRect(),0);
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
    if (ldouble(sceneRect().width()-((BorderWidth*2)+LaneTrail)) * 0.5L < viewport()->width()-((BorderWidth*2)+LaneTrail))
    {
        setZoom(zoomer->getZoom() * ldouble(viewport()->width()-((BorderWidth*2)+LaneTrail))/ldouble(sceneRect().width()-((BorderWidth*2)+LaneTrail)));
    }
    else
    {
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
    if (zoomer->getZoom() != z) zoomer->setZoom(z);
    paint();
    horizontalScrollBar()->setValue(qMax(0,m_TimeLine.currentPos()-(visibleRect().width()/2)));
    UpdateAutomationGeometry();
}

void CWaveLanes::timerEvent(QTimerEvent *)
{
    if (!m_TimerID) return;
    m_TimeLine.handleTimer(this);
    if (requestIsPlaying()) {
        if (!visibleRect().contains(m_TimeLine.currentPos(),1))
        {
            if (m_EditLane > -1) {
                requestSkip(m_TimeLine.sampleFromX(horizontalScrollBar()->value()));
            }
            else {
                horizontalScrollBar()->setValue(m_TimeLine.currentPos());
            }
        }
    }
}

CAudioBuffer* CWaveLanes::getNextA(const int ProcIndex)
{
    if (m_Mixer != nullptr) return m_Mixer->getNextA(ProcIndex+CStereoMixer::jnOut);
    return nullptr;//&m_NullBufferStereo;
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
    //deviceList.pause();
}

void CWaveLanes::reset()
{
    for (CWaveLane* l : std::as_const(lanes)) l->reset();
    CalcMixFactor();
}

void CWaveLanes::CalcMixFactor()
{
    MixFactor = mixFactorf(lanes.size());
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

double CWaveLanes::quarterRate() const
{
    return (60*presets.SampleRate)/rulerTempo;
}

void CWaveLanes::UpdateGeometry() {
    for (int i=0;i<lanes.size();i++)
    {
        for(int j=0;j<lanes[i]->tracks.size();j++)
        {
            lanes[i]->tracks[j]->isActive=((i==CurrentLane) && (CurrentTrack.contains(j)));
        }
    }
    if (m_EditLane < 0) {
        for (int i=0;i<lanes.size();i++)
        {
            CWaveLane* Lane=lanes[i];
            Lane->geometry=QRect(sample2Pos(0),BorderWidth+RulerHeight+(LaneHeight*i),LaneTrail,LaneHeight-LaneGap);
            Lane->UpdateGeometry(zoomer->getZoom(),LaneTrail);
        }
    }
    else {
        int y = RulerHeight + BorderWidth;
        for (int i=0;i<lanes.size();i++)
        {
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
    Scene.clear();
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
    for (int i=0;i<lanes.size();i++)
    {
        lanes[i]->paint(Scene,zoomer->getZoom(),visibleRect(),i == CurrentLane);
    }
    m_TimeLine.setPen(QPen(Qt::black));
    m_TimeLine.render(&Scene,visibleRect());
    CalcMixFactor();
    if (InfoLabel->isVisible())
    {
        QPoint p(mapToScene(QCursor::pos()).toPoint());
        int L=MouseOverLane(p);
        ShowInfoLabel(pos2Sample(p.x()),L);
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
    for (int i = 0; i < lanes.size(); i++)
    {
        lanes[i]->serialize(xml->appendChild("Lane" + QString::number(i+1))->appendChild("Lane"));
    }
    QDomLiteElement* Mixer=xml->appendChild("Mixer");
    m_MixerWidget->serialize(Mixer);
    for (int i=0;i<Effects.size();i++)
    {
        Effects[i]->serializeCustom(Mixer->appendChild("Effect"+QString::number(i))->appendChild("Custom"));
    }
    xml->setAttribute("RulerBeats",rulerBeats);
    xml->setAttribute("RulerTempo",rulerTempo);
    xml->setAttribute("CurrentLane",CurrentLane);
    QString s;
    for (const int& i : CurrentTrack) s.append(QString::number(i)+"&");
    xml->setAttribute("CurrentTrack",s);
}

void CWaveLanes::unserialize(const QDomLiteElement* xml)
{
    QMutexLocker locker(&mutex);
    requestPause();
    Loading=true;
    if (m_Mixer)
    {
        m_Mixer->removerEffectRacksFromDeviceList(&deviceList);
        deviceList.deleteDevice(m_Mixer);
        m_Mixer=nullptr;
        while (!m_MixerWidget->channels.empty()) m_MixerWidget->removeChannel();
    }
    for (CWaveLane* L : std::as_const(lanes)) deviceList.deleteDevice(L);
    lanes.clear();
    for (CDeviceContainer* d : std::as_const(Effects)) d->ClearDevice();
    if (xml)
    {
        int i=0;
        while (const QDomLiteElement* Lane = xml->elementByTag("Lane" + QString::number(++i)))
        {
            AddLaneInternal();
            CWaveLane* L=lanes.last();
            L->unserialize(Lane->elementByTag("Lane"),zoomer->getZoom());
        }
        if (const QDomLiteElement* Mixer = xml->elementByTag("Mixer"))
        {
            m_MixerWidget->unserialize(Mixer);
            for (int i = 0; i < 3; i++)
            {
                if (const QDomLiteElement* e = Mixer->elementByTag("Effect"+QString::number(i)))
                {
                    Effects[i]->unserializeCustom(e->elementByTag("Custom"));
                }
            }
        }
        rulerBeats=xml->attributeValueInt("RulerBeats",4);
        rulerTempo=xml->attributeValueInt("RulerTempo",120);
        CurrentLane = xml->attributeValueInt("CurrentLane",CurrentLane);
        QStringList l = xml->attribute("CurrentTrack","").split("&",Qt::SkipEmptyParts);
        CurrentTrack.clear();
        for (const QString& s : l) CurrentTrack.append(s.toInt());
    }
    if (lanes.isEmpty()) AddLaneInternal();
    paint();
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
    for(int i=0;i<lanes.size();i++) if (lanes[i]->geometry.contains(Pos)) return i;
    return -1;
}

int CWaveLanes::MouseOverTrack(QPoint Pos, int Lane)
{
    if (Lane>-1)
    {
        return lanes[Lane]->MouseOverTrack(Pos);
    }
    return -1;
}

double CWaveLanes::sample2Beat(ulong64 sample, int div) const
{
    return sample/(quarterRate()/double(div));
}

ulong64 CWaveLanes::beat2Sample(int beat, int div) const
{
    return ulong64(beat*ldouble(quarterRate()/div));
}

void CWaveLanes::QuantizeStraight()
{
    if (canCopy())
    {
        MainMenu->UndoMenu->addItem("Quantize Straight");
        for (const int& i : std::as_const(CurrentTrack)) {
            ulong64 Start=lanes[CurrentLane]->tracks[i]->start;
            lanes[CurrentLane]->tracks[i]->start=beat2Sample(qRound(sample2Beat(Start,2)),2);
        }
        paint();
    }
}

void CWaveLanes::QuantizeTriplet()
{
    if (canCopy())
    {
        MainMenu->UndoMenu->addItem("Quantize Triplet");
        for (const int& i : std::as_const(CurrentTrack)) {
            ulong64 Start=lanes[CurrentLane]->tracks[i]->start;
            lanes[CurrentLane]->tracks[i]->start=beat2Sample(qRound(sample2Beat(Start,3)),3);
        }
        paint();
    }
}

void CWaveLanes::ShowInfoLabel(ulong64 Start,int Lane) {
    if (Lane > -1) ShowInfoLabel(Start, lanes[Lane]);
}

void CWaveLanes::ShowInfoLabel(ulong64 Start, CWaveLane* Lane)
{
    int Top = Lane->geometry.bottom();
    ldouble mSecs = presets.samplesTomSecs(Start);
    InfoLabel->hide();
    setFontSizeScr(InfoLabel,11);
    InfoLabel->setText("Sample: "+QString::number(Start)+" \nTime: "+m_TimeLine.timeToText(mSecs,CTimeLine::TimeLineMilliseconds)+" \nBar: " + m_TimeLine.timeToText(mSecs,CTimeLine::TimelineBars));
    InfoLabel->move(sample2Pos(Start)-visibleRect().left(),Top);//,fm.horizontalAdvance("Sample "+QString::number(Start)+" \n")+4,(fm.height()*3)+4);
    InfoLabel->adjustSize();
    InfoLabel->show();
}

void CWaveLanes::mouseDoubleClickEvent(QMouseEvent *event)
{
    QPoint Pos=mapToScene(event->pos()).toPoint();
    if (event->button()==Qt::LeftButton) {
        if (m_TimeLine.handleDoubleClick(Pos,this)) return;
    }
    int Lane=MouseOverLane(Pos);
    int Track=MouseOverTrack(Pos,Lane);
    if ((Lane != CurrentLane) || (!CurrentTrack.contains(Track)))
    {
        CurrentLane=Lane;
        CurrentTrack.clear();
        if (Track > -1) CurrentTrack.append(Track);
        paint();
    }
    if (CurrentLane > -1) {
        EditLane();
    }
}

void CWaveLanes::UpdateEditTrack(CWaveGenerator::LoopParameters LP)
{
    m_EditTrack->loopParameters=LP;
}

void CWaveLanes::EditTrack() {
    if (canCopy()) {
        MainMenu->UndoMenu->addItem("Edit Track");
        QDialog d(this,Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);
        d.resize(600,200);
        auto e=new CWaveEditWidget(&d);
        connect(e,&CWaveEditWidget::Changed,this,&CWaveLanes::UpdateEditTrack);
        m_EditTrack=lanes[CurrentLane]->tracks[CurrentTrack.first()];
        auto l=new QHBoxLayout(&d);
        l->setContentsMargins(0,0,0,0);
        l->setSpacing(0);
        d.setLayout(l);
        d.setWindowTitle(QFileInfo(m_EditTrack->name).baseName());
        d.setGeometry(QRect(mapToGlobal(this->geometry().topLeft()),this->size()));
        l->addWidget(e);
        e->Init(&m_EditTrack->waveGenerator,m_EditTrack->loopParameters,false);
        d.exec();
        paint();
    }
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
    EditLaneAction->setEnabled(CurrentLane>-1);
    EffectRackAction->setEnabled(CurrentLane>-1);
    MainMenu->EditMenu->setSelectionStatus(canCopy());
    EditLaneAction->setChecked(m_EditLane > -1);
}

void CWaveLanes::EffectRack() {
    if (CurrentLane > -1) {
        m_MixerWidget->channels[CurrentLane]->toggleEffectRack();
    }
}

void CWaveLanes::mousePressEvent(QMouseEvent *event)
{
    StartPos=mapToScene(event->pos()).toPoint();
    int Lane=MouseOverLane(StartPos);
    int Track=MouseOverTrack(StartPos,Lane);
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
            lanes[CurrentLane]->paintEdges(StartPos,Track,Scene,zoomer->getZoom(),visibleRect());
        }
    }
    if ((CurrentLane > -1) && (!CurrentTrack.isEmpty())) ShowInfoLabel(lanes[CurrentLane]->tracks[CurrentTrack.first()]->start,CurrentLane);
    if (event->button()==Qt::RightButton) {
        MainMenu->EditMenu->popup(mapToGlobal(event->pos()));
        return;
    }
    if (CurrentLane > -1) {
        lanes[CurrentLane]->DragTracks = CurrentTrack;
        long64 s = lanes[CurrentLane]->handleMousePress(StartPos);
        if (s > -1) {
            ShowInfoLabel(s,CurrentLane);
            lanes[CurrentLane]->drawOutsideWave(Scene,visibleRect());
            DragBackup = new QDomLiteElement("UndoItem");
            serialize(DragBackup);
        }
    }
}

void CWaveLanes::mouseMoveEvent(QMouseEvent *event)
{
    QPoint Pos=mapToScene(event->pos()).toPoint();
    if (m_TimeLine.handleMouseMove(Pos,this)) return;
    if (CurrentLane > -1) {
        long64 s = lanes[CurrentLane]->handleMouseMove(Pos,&m_TimeLine);
        if (s > -1)
        {
            paint();
            lanes[CurrentLane]->drawOutsideWave(Scene,visibleRect());
            for (const int& i : (const QList<int>)CurrentTrack) lanes[CurrentLane]->paintTrack(i,Scene,zoomer->getZoom(),visibleRect(),-1);
            ShowInfoLabel(s,CurrentLane);
            return;
        }
    }
    int LaneIndex=MouseOverLane(Pos);
    int TrackIndex=MouseOverTrack(Pos,LaneIndex);
    if (LaneIndex > -1)
    {
        ShowInfoLabel(pos2Sample(Pos.x()),LaneIndex);
        if (TrackIndex > -1)
        {
            if ((TrackIndex != m_OldDragTrack) && (m_OldDragTrack > -1)) lanes[LaneIndex]->paintTrack(m_OldDragTrack,Scene,zoomer->getZoom(),visibleRect(),0);
            if (lanes[LaneIndex]->paintEdges(Pos,TrackIndex,Scene,zoomer->getZoom(),visibleRect())) {
                m_OldDragTrack = TrackIndex;
                m_OldDragLane = LaneIndex;
                setCursor(Qt::SizeHorCursor);
                return;
            }
        }
    }
    else
    {
        InfoLabel->hide();
    }
    unsetCursor();
    if ((m_OldDragLane > -1) && (m_OldDragTrack > -1))
    {
        lanes[m_OldDragLane]->paintTrack(m_OldDragTrack,Scene,zoomer->getZoom(),visibleRect(),0);
        m_OldDragLane = -1;
        m_OldDragTrack = -1;
    }
}

void CWaveLanes::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint Pos=mapToScene(event->pos()).toPoint();
    if (DragBackup) {
        if (Pos != StartPos) MainMenu->UndoMenu->addElement(DragBackup,"Drag");
        delete DragBackup;
        DragBackup = nullptr;
    }
    if (m_TimeLine.handleMouseRelease(Pos, this)) return;
    if (CurrentLane > -1)
    {
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
            if (TrackIndex > -1) Lane->paintEdges(Pos,TrackIndex,Scene,zoomer->getZoom(),visibleRect());
        }
    }
}


void CWaveLanes::CopyDoc(QDomLiteElement* xml)
{
    QMutexLocker locker(&mutex);
    if (!lanes.isEmpty())
    {
        if (canCopy())
        {
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
        if (!lanes.isEmpty())
        {
            if (CurrentLane > -1)
            {
                MainMenu->UndoMenu->addItem("Paste");
                ulong64 Start = m_TimeLine.currentSample();//pos2Sample(StartPos.x());
                QDomLiteElement* tracks = xml->elementByTag("Tracks");
                ulong64 f = 0;
                int i = 0;
                for (QDomLiteElement* e : (const QDomLiteElementList)tracks->elementsByTag("Track"))
                {
                    if (i == 0) f = e->attributeValueULongLong("StartPoint");
                    i++;
                    if (CWaveTrack* t = lanes[CurrentLane]->unserializeTrack(e,zoomer->getZoom())) {
                        t->start = (e->attributeValueULongLong("StartPoint")-f)+Start;
                        lanes[CurrentLane]->sanityCheck(t);
                    }
                }
                paint();
            }
        }
}

void CWaveLanes::Split()
{
    if (!lanes.isEmpty())
    {
        if (canCopy())
        {
            MainMenu->UndoMenu->addItem("Split");
            CWaveTrack* t = lanes[CurrentLane]->tracks[CurrentTrack.first()];
            const ulong64 s = m_TimeLine.currentSample();
            lanes[CurrentLane]->cloneTrack(t,zoomer->getZoom())->cutStart(s);
            t->cutEnd(s);
            paint();
        }
    }
}

void CWaveLanes::Automation() {
    if (CurrentLane > -1) {
        CAutomationLane* a = new CAutomationLane(this);
        a->setGeometry(lanes[CurrentLane]->geometry.adjusted(0,0,-50,0));
        a->updateGeometry();
        a->fill(lanes[CurrentLane],0,&deviceList,false);
        a->show();
    }
}

void CWaveLanes::UpdateAutomationGeometry() {
    for (CAutomationLane* a : (const QList<CAutomationLane*>)findChildren<CAutomationLane*>()) {
        a->setGeometry(lanes[CurrentLane]->geometry.adjusted(0,0,-50,0).translated(-horizontalScrollBar()->value(),0));
        a->updateGeometry();
    }
}


void CWaveLanes::DeleteDoc()
{
    MainMenu->UndoMenu->addItem("Delete Track");
    for (const int& i : (const QList<int>)CurrentTrack) RemoveTrackAt(CurrentLane, i);
}

void CWaveLanes::RemoveTrackAt(int Lane, int Track)
{
    QMutexLocker locker(&mutex);
    if (!lanes.isEmpty())
    {
        if (canCopy())
        {
                QString FN = lanes[Lane]->tracks[Track]->name;
                delete lanes[Lane]->tracks.takeAt(Track);
                for (int i = CurrentTrack.size() - 1; i >= 0; i--) {
                    if (CurrentTrack[i] >= Track) CurrentTrack[i]--;
                    if (CurrentTrack[i] < 0) CurrentTrack.removeAt(i);
                }
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
    mutex.lock();
    deviceList.disconnectAll();
    if (!m_Mixer)
    {
        m_Mixer=new CStereoMixer(0,3);
        deviceList.addDevice(m_Mixer,1,nullptr);
    }
    m_MixerWidget->hide();
    m_MixerWidget->hideMaster();
    if (lanes.size()!=int(m_Mixer->channelCount()))
    {
        QDomLiteElement channelXML;
        for (uint i = 0; i < m_Mixer->channelCount(); i++) {
            QDomLiteElement* e = new QDomLiteElement("Channel");
            m_MixerWidget->channels[i]->serialize(e);
            e->tag = m_MixerWidget->channels[i]->ID;
            channelXML.appendChild(e);
        }
        m_Mixer->removerEffectRacksFromDeviceList(&deviceList);
        deviceList.deleteDevice(m_Mixer);
        m_Mixer = new CStereoMixer(lanes.size(),3);
        deviceList.addDevice(m_Mixer,1,nullptr);
        m_Mixer->addEffectRacksToDeviceList(&deviceList,m_MainWindow);
        while (m_MixerWidget->channels.size()>lanes.size()) m_MixerWidget->removeChannel();
        while (m_MixerWidget->channels.size()<lanes.size()) m_MixerWidget->appendChannel();
        for (int i = 0; i < lanes.size(); i++)
        {
            CSF2ChannelWidget* ch = m_MixerWidget->channels[i];
            ch->init(m_Mixer->channels[i],"Lane " + QString::number(i+1));
            if (QDomLiteElement* e = channelXML.elementByTag(lanes[i]->ID)) ch->unserialize(e);
            ch->ID = lanes[i]->ID;
            ch->setVisible(true);
            lanes[i]->parameters[0]->connectToWidget(ch->volSlider,&CChannelVol::volChanged,&CChannelVol::setVol);
            lanes[i]->parameters[1]->connectToWidget(ch->effectsPanel,&CChannelEffects::panValueChanged,&CChannelEffects::setPanValue);
        }
        channelXML.clearChildren();
    }
    for (int i = 0; i < lanes.size(); i++)
    {
        deviceList.connect("StereoMixer 1 In " + QString::number(i+1),"WaveLane " + QString::number(i+1) + " Out");
    }
    for (int i=0;i<3;i++)
    {
        deviceList.connect("Effect "+ QString::number(i+1) +" In","StereoMixer 1 Send "+ QString::number(i+1));
        deviceList.connect("StereoMixer 1 Return "+ QString::number(i+1),"Effect "+ QString::number(i+1) +" Out");
    }
    m_MixerWidget->showMaster(m_Mixer,&Effects);
    mutex.unlock();
    m_Mixer->setDisabled(false);
    m_MixerWidget->start();
    m_MixerWidget->show();
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
}

void CWaveLanes::AddLaneInternal() {
    QMutexLocker locker(&mutex);
    auto L = new CWaveLane;
    deviceList.addDevice(L,lanes.size()+1,m_MainWindow);
    int i = 1;
    QStringList IDList;
    for (const CWaveLane* l : std::as_const(lanes)) IDList.append(l->ID);
    while (IDList.contains("Lane " + QString::number(i))) i++;
    L->ID = "Lane " + QString::number(i);
    lanes.append(L);
    updateMixer();
    CurrentTrack.clear();
    CurrentLane = lanes.size()-1;
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
        }
    }
}

QString CWaveLanes::DropFileName(const QMimeData* d, const QObject* s) {
    qDebug() << d->urls() << d->html() << d->formats();
    if (d->urls().size()) {
        QString path = d->urls().first().toLocalFile();
        if (QFileInfo::exists(path)) {
            return path;
        }
    }
    QString sender = s->metaObject()->className();
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
    int Lane = MouseOverLane(e->position().toPoint());
    int Track = MouseOverTrack(e->position().toPoint(),CurrentLane);
    ulong64 Start=pos2Sample(e->position().x());
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
    if (T->isValid)
    {
        MainMenu->UndoMenu->addItem("Add file");
        Loading=true;
        L->tracks.append(T);
        L->sanityCheck(T);
        paint();
        Loading=false;
        emit FileAdded(FN);
        return true;
    }
    delete T;
    return false;
}
