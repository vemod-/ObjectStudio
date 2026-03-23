#include "cwavelane.h"

CWaveLane::CWaveLane()
    : pitchShifterL(presets.SampleRate),pitchShifterR(presets.SampleRate)
{
    Vol=0;
    PS[0]=&pitchShifterL;
    PS[1]=&pitchShifterR;
    DragTrack = -1;
    m_Zoom = 1;
}

CWaveLane::~CWaveLane()
{
    if (videoItem) delete videoItem;
    qDeleteAll(tracks);
    qDeleteAll(parameters);
}

void CWaveLane::play(const bool FromStart)
{
    qDebug() << "play 1";
    createVideoWidget();
    if (videoItem) {
        videoItem->play();
        videoItem->setEnabled(false);
        videoItem->setRenderOpacity(1);
    }
    qDebug() << "play 2";
    if (FromStart) {
        reset();
    }
    //m_Playing = true;
    qDebug() << "play 3";
    IDevice::play(FromStart);
    qDebug() << "play 4";
}

void CWaveLane::pause()
{
    //m_Playing = false;
    qDebug() << "pause 1";
    if (videoItem) {
        videoItem->stop();
        //videoItem->setVisible(videoVisible);
        //videoItem->setVideoStillEmptyFrame();
    }
    qDebug() << "pause 2";
    IDevice::pause();
    qDebug() << "pause 3";
}

void CWaveLane::init(const int Index, QWidget* MainWindow)
{
    m_Name = "WaveLane";
    IDevice::init(Index,MainWindow);
    addJackStereoOut(0);
    addParameter(CParameter::dB,"Volume","dB",0,150,1,"",100);
    addParameterPan();
    for (int i = 0; i < parameterCount(); i++) parameters.append(new CParameterWrapper(parameter(i)));
}

CAudioBuffer* CWaveLane::getNextA(const int ProcIndex)
{
    if (!m_Playing) return nullptr;
    CStereoBuffer* Buffer = StereoBuffer(ProcIndex);
    Buffer->zeroBuffer();
    const ulong64 ModRate=Buffer->size();
    CWaveTrack* PlayingTrack=nullptr;
    CWaveTrack* StartingTrack=nullptr;
    for (CWaveTrack* t : std::as_const(tracks))
    {
        //if (t->loopParameters.playLength()+t->start > Counter)
        if (t->end() > Counter)
        {
            if (t->start < Counter)
            {
                PlayingTrack = t;
                if (videoItem) {
                    videoItem->setEnabled(trackVisible(t));
                    if (t->hasOpacity()) {
                        videoItem->setRenderOpacity(t->fadeOpacity(Counter));
                    }
                }
            }
            else if (t->start < Counter + ModRate)
            {
                StartingTrack = t;
                if (videoItem) {
                    videoItem->setEnabled(trackVisible(t));
                    if (t->hasOpacity()) {
                        videoItem->setRenderOpacity(t->fadeOpacity(Counter));
                    }
                    else {
                        videoItem->setRenderOpacity(1);
                    }
                }
            }
        }
    }
    if ((StartingTrack == nullptr) && (PlayingTrack == nullptr))
    {
        CurrentBuffer.makeNull();
        if (videoItem) {
            videoItem->setEnabled(false);
            videoItem->setRenderOpacity(1);
        }
    }
    for (uint i = 0; i < ModRate; i++)
    {
        if (PlayingTrack)
        {
            if (Counter >= PlayingTrack->end()) // PlayingTrack->start + PlayingTrack->loopParameters.playLength())
            {
                CurrentBuffer.makeNull();
                if (videoItem) {
                    if (!StartingTrack) videoItem->invokePause();
                }
            }
            else
            {
                if (ModulationCounter >= ModRate)
                {
                    const ulong64 trackPos = PlayingTrack->pos(Counter); //((Counter - PlayingTrack->start) * PlayingTrack->loopParameters.Speed) + PlayingTrack->loopParameters.Start;
                    if (PlayingTrack->waveGenerator.currentSample() != trackPos) {
                        PlayingTrack->waveGenerator.skipTo(trackPos);
                        if (videoItem) {
                            if (trackVisible(PlayingTrack)) {
                                videoItem->invokeVideoPlayProperties(PlayingTrack,trackPos);
                                syncCounter = CPresets::presets().SampleRate / (CPresets::presets().ModulationRate * 4);
                            }
                        }
                    }
                    else {
                        if (videoItem) {
                            if (trackVisible(PlayingTrack)) {
                                if (syncCounter-- <= 0) {
                                    videoItem->invokeVideoPlaySync(PlayingTrack,trackPos);
                                    syncCounter = CPresets::presets().SampleRate / (CPresets::presets().ModulationRate * 4);
                                }
                            }
                        }
                    }
                    CurrentBuffer.fromRawData(PlayingTrack->waveGenerator.getNextSpeed(PlayingTrack->loopParameters.Speed),PlayingTrack->waveGenerator.channels(),ModRate);
                    pitchShift(PlayingTrack);
                    ModulationCounter = 0;
                    Vol=PlayingTrack->fadeVolume(Counter);
                }
            }
        }
        if (StartingTrack)
        {
            //if (Counter >= StartingTrack->start + StartingTrack->loopParameters.playLength())
            if (Counter >= StartingTrack->end())
            {
                CurrentBuffer.makeNull();
                if (videoItem) videoItem->invokePause();
            }
            else
            {
                if (Counter == StartingTrack->start)
                {
                    StartingTrack->waveGenerator.reset();
                    StartingTrack->waveGenerator.skipTo(StartingTrack->loopParameters.Start);
                    if (videoItem) {
                        if (trackVisible(StartingTrack)) {
                            videoItem->invokeVideoPlayProperties(StartingTrack,StartingTrack->loopParameters.Start);
                            syncCounter = CPresets::presets().SampleRate / (CPresets::presets().ModulationRate * 4);
                        }
                        videoItem->setEnabled(trackVisible(StartingTrack));
                    }
                    CurrentBuffer.fromRawData(StartingTrack->waveGenerator.getNextSpeed(StartingTrack->loopParameters.Speed),StartingTrack->waveGenerator.channels(),ModRate);
                    pitchShift(StartingTrack);
                    ModulationCounter=0;
                    Vol=StartingTrack->fadeVolume(Counter);
                }
            }
        }
        if (CurrentBuffer.isValid())
        {
            if (CurrentBuffer.channels()==1)
            {
                Buffer->setAt(i,CurrentBuffer.at(ModulationCounter,0)*Vol);
            }
            else
            {
                Buffer->setAt(i,CurrentBuffer.at(ModulationCounter,0)*Vol,CurrentBuffer.at(ModulationCounter,1)*Vol);
            }
        }
        else
        {
            Buffer->zeroAt(i);
        }
        ModulationCounter++;
        Counter++;
    }
    return Buffer;
}

void CWaveLane::pitchShift(CWaveTrack* T)
{
    if (!isZero(T->loopParameters.PitchShift))
    {
        for (uint c = 0; c < T->waveGenerator.channels(); c++)
        {
            PS[c]->process(cent2Factor(T->loopParameters.PitchShift*100.0),CurrentBuffer.size(),CurrentBuffer.data()+(c*CurrentBuffer.size()),TempBuffer.data()+(c*CurrentBuffer.size()));
        }
        CurrentBuffer.fromRawData(TempBuffer.data(),T->waveGenerator.channels(),CurrentBuffer.size());
    }
}

void CWaveLane::reset()
{
    Counter=0;
    ModulationCounter=0;
    CurrentBuffer.makeNull();
    for (CWaveTrack* T : std::as_const(tracks))
    {
        if (!T->isValid)
        {
            tracks.removeOne(T);
            delete T;
        }
    }
}

void CWaveLane::UpdateGeometry(ldouble ZoomFactor, long CanvasRight)
{
    m_Zoom = ZoomFactor;
    for (CWaveTrack* T : std::as_const(tracks))
    {
        if (!T->isValid)
        {
            tracks.removeOne(T);
            delete T;
        }
        else
        {
            T->geometry=QRect(sample2Pos(T->start),geometry.top()+1,ldouble(T->loopParameters.playLength())*ZoomFactor,geometry.height()-2);
        }
    }
    geometry.setRight(sample2Pos(samples()) + CanvasRight);
}

void CWaveLane::paint(QGraphicsScene& Scene, ldouble ZoomFactor, QRect visibleRect, bool Active)
{
    QPainterPath path;
    path.addRoundedRect(geometry,6,6);
    if (Active) {
        Scene.addPath(path,QPen(Qt::darkGray),QBrush(Qt::lightGray));
    }
    else {
        Scene.addPath(path,QPen(Qt::darkGray),QBrush(Qt::gray));
    }

    for (CWaveTrack* t : std::as_const(tracks))
    {
        if (!t->isActive) t->paint(Scene,ZoomFactor,visibleRect,0);
    }
    for (CWaveTrack* t : std::as_const(tracks))
    {
        if (t->isActive) t->paint(Scene,ZoomFactor,visibleRect,0);
    }
}

void CWaveLane::paintTrack(int Track, QGraphicsScene& Scene, ldouble ZoomFactor, QRect visibleRect, int edge) {
    if (edge == -1) {
        tracks[Track]->paint(Scene,ZoomFactor,visibleRect,DragTrackEdge);
        return;
    }
    if (Track > -1) tracks[Track]->paint(Scene,ZoomFactor,visibleRect,edge);
}

int CWaveLane::paintEdges(QPoint p, int t, QGraphicsScene& Scene, ldouble ZoomFactor, QRect visibleRect) {
    if (p.x() < sample2Pos(tracks[t]->start)+4) {
        paintTrack(t,Scene,ZoomFactor,visibleRect,1);
        return 1;
    }
    else if (p.x() > sample2Pos(tracks[t]->end())-4) {
        paintTrack(t,Scene,ZoomFactor,visibleRect,2);
        return 2;
    }
    return 0;
}

bool CWaveLane::fileInUse(const QString& Filename)
{
    for (const CWaveTrack* T : std::as_const(tracks)) if (QFileInfo(T->name)==QFileInfo(Filename)) return true;
    return false;
}

const QStringList CWaveLane::fileList()
{
    QStringList l;
    for (CWaveTrack* T : std::as_const(tracks)) l.append(T->name);
    return l;
}

void CWaveLane::renameFile(const QString& oldName, const QString& newName)
{
    for (CWaveTrack* T : std::as_const(tracks)) if (QFileInfo(T->name)==QFileInfo(oldName)) T->name = newName;
}

void CWaveLane::removeFile(const QString &Filename)
{
    for (CWaveTrack* T : std::as_const(tracks))
    {
        if (QFileInfo(T->name)==QFileInfo(Filename))
        {
            tracks.removeOne(T);
            delete T;
        }
    }
    destroyVideoWidget();
}

CWaveTrack* CWaveLane::unserializeTrack(const QDomLiteElement* xml, ldouble ZoomFactor)
{
    if (!xml) return nullptr;
    const int Position=xml->attributeValueInt("Position");
    ulong64 Start=xml->attributeValueULongLong("StartPoint");
    if (Start==0) Start=ldouble(Position)/ZoomFactor;
    const QString Name=CPresets::resolveFilename(xml->attribute("Path"));

    const auto WT=new CWaveTrack(Name,Start);
    if (WT->isValid)
    {
        uint savedRate = xml->attributeValueInt("OrigRate",44100);
        if (savedRate != CPresets::presets().SampleRate) {
            ldouble rateFactor = ldouble(CPresets::presets().SampleRate)/ldouble(savedRate);
            WT->start *= rateFactor;
        }
        WT->loopParameters.unserialize(xml);
        WT->videoVisible = xml->attributeValueBool("VideoVisible",true);
        tracks.append(WT);
        createVideoWidget();
        return WT;
    }
    delete WT;
    return nullptr;
}

CWaveTrack* CWaveLane::cloneTrack(const CWaveTrack* WT, ldouble ZoomFactor) {
    QDomLiteElement e("Track");
    serializeTrack(&e,WT);
    return unserializeTrack(&e,ZoomFactor);
}

void CWaveLane::serializeTrack(QDomLiteElement* xml, const CWaveTrack* WT) const
{
    xml->setAttribute("Path",WT->name);
    xml->setAttribute("StartPoint",WT->start);
    WT->loopParameters.serialize(xml);
    xml->setAttribute("VideoVisible",WT->videoVisible);
}

void CWaveLane::unserialize(const QDomLiteElement* xml,ldouble ZoomFactor)
{
    if (!xml) return;
    for (const QDomLiteElement* xmlTrack : (const QDomLiteElementList)xml->elementsByTag("Track"))
    {
        unserializeTrack(xmlTrack,ZoomFactor);
    }
    if (QDomLiteElement* p = xml->elementByTag("Parameters")) unserializeParameters(p);
    createVideoWidget();
    videoVisible = xml->attributeValueBool("VideoVisible",true);
    if (videoItem) videoItem->unserialize(xml);
    /*
    if (const QDomLiteElement* eff = xml->elementByTag("EffectRack")) {
        m_EffectRack->unserializeDevice(eff);
        m_EffectRack->raiseForm();
    }
*/
}

void CWaveLane::serialize(QDomLiteElement* xml) const
{
    for (const CWaveTrack* WT : tracks)
    {
        serializeTrack(xml->appendChild("Track"),WT);
    }
    serializeParameters(xml->appendChild("Parameters"));
    xml->setAttribute("VideoVisible",videoVisible);
    if (videoItem) videoItem->serialize(xml);
    //m_EffectRack->serializeDevice(xml->appendChild("EffectRack"));
}

ulong CWaveLane::milliSeconds() const
{
    ulong retval=0;
    for (const CWaveTrack* t : tracks)
    {
        //retval=qMax<ulong>(t->loopParameters.playLength() + t->start,retval);
        retval=qMax<ulong>(t->end(),retval);
    }
    return qMax<ulong>(presets.samplesTomSecs(retval), IDevice::milliSeconds());
}

ulong64 CWaveLane::samples() const
{
    ulong64 retval=0;
    for (const CWaveTrack* t : tracks)
    {
        //retval=qMax<ulong>(t->loopParameters.playLength() + t->start,retval);
        retval=qMax<ulong>(t->end(),retval);
    }
    return qMax<ulong64>(retval, IDevice::samples());
}

void CWaveLane::skip(const ulong64 samples)
{
    qDebug() << "skip 1";
    createVideoWidget();
    qDebug() << "skip 2";
    reset();
    qDebug() << "skip 3";
    Counter = samples;
    if (videoItem) {
        videoItem->setEnabled(videoVisible);
        videoItem->setRenderOpacity(1);
    }
    bool gotImage = false;
    for (CWaveTrack* t : std::as_const(tracks))//for (int i=0;i<tracks.size();i++)
    {
        //if (t->loopParameters.playLength() + t->start > Counter)
        if (t->end() > Counter)
        {
            if (t->start <= Counter)
            {
                qDebug() << "Track found" << trackVisible(t);
                t->waveGenerator.reset();
                //t->waveGenerator.skipTo(((Counter - t->start) * t->loopParameters.Speed) + t->loopParameters.Start);
                t->waveGenerator.skipTo(t->pos(Counter));
                if (videoItem) {
                    //videoItem->setVideoStillProperties(t,CPresets::samplesTomSecs(((Counter - t->start) * t->loopParameters.Speed) + t->loopParameters.Start));
                    videoItem->setVideoStillProperties(t,CPresets::samplesTomSecs(t->pos(Counter)));
                    videoItem->setEnabled(trackVisible(t));
                    videoItem->setRenderOpacity(t->fadeOpacity(Counter));
                    gotImage = true;
                }
            }
        }
    }
    if (!gotImage) {
        if (videoItem) {
            videoItem->setVideoStillEmptyFrame();
            videoItem->setRenderOpacity(1);
        }
    }
    qDebug() << "skip 4";
    IDevice::skip(samples);
    qDebug() << "skip 5";
}

bool CWaveLane::setVideoExportTime(const ulong64 mSec) {
    //createVideoWidget();
    //reset();
    ulong64 samples = CPresets::mSecsToSamples(mSec);
    Counter = samples;
    bool gotFrame = false;
    if (videoItem) {
        for (CWaveTrack* t : std::as_const(tracks))
        {
            //if (t->loopParameters.playLength() + t->start > Counter)
            if (t->end() > Counter)
            {
                if (t->start <= Counter)
                {
                    if (trackVisible(t)) {
                        //videoItem->setVideoExportProperties(t,CPresets::samplesTomSecs(((Counter - t->start) * t->loopParameters.Speed) + t->loopParameters.Start));
                        videoItem->setVideoExportProperties(t,CPresets::samplesTomSecs(t->pos(Counter)));
                        videoItem->setRenderOpacity(t->fadeOpacity(Counter));
                        gotFrame = true;
                        //qDebug() << "gotFrame";
                    }
                }
            }
        }
        if (!gotFrame) {
            videoItem->setVideoExportEmptyFrame();
            //videoItem->setOpacity(1);
            //qDebug() << "Empty frame";
        }
    }
    return gotFrame;
}

int CWaveLane::MouseOverTrack(QPoint Pos)
{
    for(int i = 0; i < tracks.size(); i++)
    {
        if (tracks[i]->geometry.contains(Pos)) return i;
    }
    return -1;
}

ulong64 CWaveLane::pos2Sample(int Pos) const
{
    return long64(ldouble(qMax(Pos-geometry.left(),0))/m_Zoom);
}

int CWaveLane::sample2Pos(long64 sample) const
{
    return int((ldouble(sample)*m_Zoom)+geometry.left());
}

bool CWaveLane::closeToLine(const ulong64 sample, CTimeLine* timeLine) const {
    if ((sample2Pos(sample) < timeLine->currentPos() + 8) && (sample2Pos(sample) > timeLine->currentPos() - 8)) return true;
    return false;
}

long64 CWaveLane::snapTo(const long64 sample, const long64 snapSample, CTimeLine* timeLine) const {
    const long64 s = timeLine->currentSample();
    if ((snapSample != s) && (closeToLine(sample,timeLine))) {
        return s;
    }
    return sample;
}

void CWaveLane::drawOutsideWave(QGraphicsScene& Scene, QRect visibleRect) {
    QRect dragRect = waveRect;
    if (DragTrackEdge == FrontEdge) {
        dragRect.setRight(sample2Pos(tracks[DragTrack]->start));
    }
    else if (DragTrackEdge == EndEdge) {
        dragRect.setLeft(sample2Pos(tracks[DragTrack]->end()));
    }
    else {
        return;
    }
    dragRect = dragRect.intersected(geometry);
    CWaveGenerator::LoopParameters LP = tracks[DragTrack]->loopParameters;
    LP.Start = 0;
    LP.End = tracks[DragTrack]->waveGenerator.size();
    Scene.addItem(tracks[DragTrack]->waveGenerator.waveFormItem(waveRect,visibleRect.intersected(dragRect),m_Zoom,&LP));
}

long64 CWaveLane::handleMousePress(QPoint p) {
    StartPos = p;
    DragTrack = MouseOverTrack(p);
    if (DragTracks.isEmpty()) DragTracks.append(DragTrack);
    if (DragTrack > -1)
    {
        DragTrackStart = tracks[DragTrack]->start;
        DragTrackEnd = tracks[DragTrack]->end();
        waveRect = tracks[DragTrack]->geometry;
        waveRect.setLeft(sample2Pos(tracks[DragTrack]->waveStart()));
        waveRect.setRight(sample2Pos(tracks[DragTrack]->waveEnd()));
        if (StartPos.x() < sample2Pos(DragTrackStart) + 4)
        {
            DragTrackEdge = FrontEdge;
            return DragTrackStart;
        }
        else if (StartPos.x() > sample2Pos(DragTrackEnd) - 4)
        {
            DragTrackEdge = EndEdge;
            return tracks[DragTrack]->end();
        }
        else
        {
            DragTrackEdge = NoEdge;
            DragTrackStarts.clear();
            for (const int& i : std::as_const(DragTracks)) DragTrackStarts.append(tracks[i]->start);
            return DragTrackStart;
        }
    }
    return -1;
}

long64 CWaveLane::handleMouseMove(QPoint p, CTimeLine* timeLine) {
    if (DragTrack > -1)
    {
        CWaveTrack* d = tracks[DragTrack];
        if (p.x() != StartPos.x())
        {
            if (DragTrackEdge == FrontEdge)
            {
                long64 newStart = DragTrackStart - (ldouble(StartPos.x()-p.x())/m_Zoom);
                newStart = snapTo(newStart, DragTrackStart, timeLine);
                if (newStart < 0) newStart = 0;
                d->cutStart(newStart);
                if (d->length() < 1000) d->cutStart(d->end()-1000);
                return d->start;
            }
            else if (DragTrackEdge == EndEdge)
            {
                long64 newEnd = DragTrackEnd - (ldouble(StartPos.x()-p.x())/m_Zoom);
                newEnd = snapTo(newEnd, DragTrackEnd, timeLine);
                d->cutEnd(newEnd);
                if (d->length() < 1000) d->cutEnd(d->start+1000);
                return d->end();
            }
            else
            {
                for (int i = 0; i < DragTracks.size(); i++) {
                    long64 newStart = DragTrackStarts[i] - (ldouble(StartPos.x()-p.x())/m_Zoom);
                    long64 tLen = tracks[DragTracks[i]]->length();
                    newStart = snapTo(newStart, DragTrackStarts[i], timeLine);
                    newStart = snapTo(newStart + tLen, DragTrackEnd, timeLine) - tLen;
                    if (newStart < 0) newStart = 0;
                    tracks[DragTracks[i]]->start = newStart;
                }
                return d->start;
            }
        }
    }
    return -1;
}

CWaveTrack* CWaveLane::handleMouseRelease() {
    CWaveTrack* t = nullptr;
    if (DragTrack > -1)  t = tracks[DragTrack];
    DragTrack = -1;
    DragTrackEdge = NoEdge;
    DragTracks.clear();
    DragTrackStarts.clear();
    return t;
}

void CWaveLane::sanityCheck(CWaveTrack* d) {
        for (CWaveTrack* t : std::as_const(tracks)) {
            if (t) {
                if ((t != d) && (t->isValid) && (d->isValid)) {
                    if ((d->end() >= t->end()) && (d->start <= t->start)) {
                        //RemoveTrackAt(CurrentLane,i);
                        t->isValid = false;
                    }
                }
            }
        }
        for (CWaveTrack* t : std::as_const(tracks)) {
            if (t) {
                if ((t != d) && (t->isValid) && (d->isValid)) {
                    if (d->end() > t->end())
                    {
                        if (t->end() > d->start) {
                            t->cutEnd(d->start);
                        }
                    }
                    if (d->start < t->start)
                    {
                        if (t->start < d->end()) {
                            t->cutStart(d->end());
                        }
                    }
                }
            }
        }
        for (CWaveTrack* t : std::as_const(tracks)) {
            if (t) {
                if ((t != d) && (t->isValid) && (d->isValid)) {
                    if ((d->start >= t->start) && (d->end() <= t->end())) {
                        cloneTrack(t,m_Zoom)->cutStart(d->end());
                        t->cutEnd(d->start);
                    }
                }
            }
        }
        for (CWaveTrack* t : std::as_const(tracks)) {
            if (t) {
                if (t->isValid) {
                    if (t->length() <= 0) {
                        t->isValid = false;
                    }
                }
            }
        }
        destroyVideoWidget();
}


