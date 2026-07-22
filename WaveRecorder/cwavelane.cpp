#include "cwavelane.h"

CWaveLane::CWaveLane()
    : pitchShifterL(presets.SampleRate,presets.ModulationRate),pitchShifterR(presets.SampleRate,presets.ModulationRate)
{
    PS[0] = &pitchShifterL;
    PS[1] = &pitchShifterR;
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
    for (uint i = 0; i < PS.size(); i++) PS[i]->reset();
    if (videoItem) {
        CWaveTrack* firstTrack = nullptr;
        if (FromStart) {
            for (CWaveTrack* t : std::as_const(tracks)) {
                if (t->hasVideo()) {
                    firstTrack = t;
                    break;
                }
            }
        }
        videoItem->play(firstTrack);
        videoItem->setEnabled(false);
        videoItem->setRenderOpacity(1);
    }
    qDebug() << "play 2";
    if (FromStart) reset();
    //m_Playing = true;
    qDebug() << "play 3";
    IDevice::play(FromStart);
    qDebug() << "play 4";
}

void CWaveLane::pause()
{
    qDebug() << "pause 1";
    if (videoItem) videoItem->stop();
    qDebug() << "pause 2";
    IDevice::pause();
    qDebug() << "pause 3";
}

void CWaveLane::init(const int Index, QWidget* MainWindow)
{
    m_Name = "WaveLane";
    IDevice::init(Index,MainWindow);
    addJackStereoOut(0);
    addParameter(CParameterVars::dB,"Volume","dB",0,150,1,"",100);
    addParameterPan();
    for (int i = 0; i < parameterCount(); i++) parameters.append(new CParameterWrapper(parameter(i)));
}

CAudioBuffer* CWaveLane::getNextA(const int ProcIndex)
{
    if (!m_Playing) return nullptr;
    CStereoBuffer* OutBuffer = StereoBuffer(ProcIndex);
    const int ModRate = presets.ModulationRate;
    const ulong64 frameStart = Counter;
    const ulong64 frameEnd = Counter + ModRate;
    Counter = frameEnd;
    class segment {
    public:
        segment(CWaveTrack* t, ulong64 fs, bool v){
            track = t;
            ulong64 overlapStart = std::max(fs, t->start);
            ulong64 overlapEnd   = std::min(fs + CPresets::presets().ModulationRate, t->end());
            start = overlapStart - fs;
            length = overlapEnd - overlapStart;
            volume = t->fadeVolume(overlapStart);
            opacity = t->hasOpacity() ? t->fadeOpacity(overlapStart) : 1;
            visible = v;
        }
        CWaveTrack* track = nullptr;
        int start = 0;
        int length = CPresets::presets().ModulationRate;
        float volume = 0;
        float opacity = 0;
        bool visible = false;
    };

    segment* startingSegment = nullptr;
    segment* endingSegment = nullptr;
    segment* continuingSegment = nullptr;

    for (CWaveTrack* t : std::as_const(tracks)) {
        if ((t->start > frameEnd) || (t->end() < frameStart)) continue;
        if (t->start >= frameStart) {
            if (startingSegment == nullptr) startingSegment = new segment(t,frameStart,trackVisible(t));
            if (startingSegment->start == 0) break;
        }
        else if (t->end() < frameEnd) {
            if (endingSegment == nullptr) endingSegment = new segment(t,frameStart,trackVisible(t));
        }
        else {
            continuingSegment = new segment(t,frameStart,trackVisible(t));
            break;
        }
    }

    if ((startingSegment == nullptr) && (continuingSegment == nullptr) && (endingSegment == nullptr)) {
        if (videoItem) {
            if (videoItem->isEnabled()) {
                if (videoItem->videoPlayerIsPlaying()) videoItem->invokePause();
                videoItem->setEnabled(false);
                videoItem->setRenderOpacity(1);
            }
        }
        return nullptr;
    }
    if (continuingSegment) {
        segment* s = continuingSegment;
        CWaveTrack* t = s->track;
        if (videoItem) {
            videoItem->setEnabled(s->visible);
            videoItem->setRenderOpacity(s->opacity);
        }
        if (!t->hasImage()) {
            const ulong64 trackPos = t->pos(frameStart);
            const bool inSync = (trackPos == t->waveGenerator.currentSample());
            if (!inSync) t->waveGenerator.skipTo(trackPos);
            writeToOut(OutBuffer,t,ModRate,s->volume,s->start,s->length);
            if (videoItem) {
                if (s->visible) {
                    if (inSync & videoItem->videoPlayerIsPlaying()) {
                        if (syncCounter-- <= 0) {
                            videoItem->invokeVideoPlaySync(t,trackPos);
                            syncCounter = presets.SampleRate / (ModRate * 4);
                        }
                    }
                    else {
                        videoItem->invokeVideoPlayProperties(t,trackPos);
                        syncCounter = presets.SampleRate / (ModRate * 4);
                    }
                }
            }
        }
        else if (t->hasImage()) {
            if (videoItem) videoItem->setPlaybackImage(t->image);
        }
        delete s;
        return OutBuffer;
    }
    if (endingSegment) {
        segment* s = endingSegment;
        CWaveTrack* t = s->track;
        if (!t->hasImage()) {
            writeToOut(OutBuffer,t,ModRate,s->volume,s->start,s->length);
            if (videoItem) {
                if (startingSegment) {
                    if (startingSegment->track != t) videoItem->invokePause();
                }
            }
        }
        else if (t->hasImage()) {
            if (videoItem) videoItem->setVideoStillEmptyFrame();
        }
        if (videoItem) {
            videoItem->setEnabled(false);
            videoItem->setRenderOpacity(1);
        }
        delete s;
    }
    if (startingSegment) {
        segment* s = startingSegment;
        CWaveTrack* t = s->track;
        if (videoItem) {
            videoItem->setEnabled(s->visible);
            videoItem->setRenderOpacity(s->opacity);
        }
        if (!t->hasImage()) {
            t->waveGenerator.reset();
            writeToOut(OutBuffer,t,ModRate,s->volume,s->start,s->length);
            if (videoItem) {
                if (s->visible) {
                    videoItem->invokeVideoPlayProperties(t,t->startPos());
                    syncCounter = presets.SampleRate / (ModRate * 4);
                }
            }
        }
        else if (t->hasImage()) {
            if (videoItem) videoItem->setPlaybackImage(t->image);
        }
        delete s;
    }
    return OutBuffer;
}
/*
CAudioBuffer* CWaveLane::getNextA(const int ProcIndex)
{
    if (!m_Playing) return nullptr;
    float Vol = 0;
    CStereoBuffer* OutBuffer = StereoBuffer(ProcIndex);
    OutBuffer->zeroBuffer();
    const int ModRate = presets.ModulationRate;
    CWaveTrack* PlayingTrack = nullptr;
    CWaveTrack* StartingTrack = nullptr;
    for (CWaveTrack* t : std::as_const(tracks)) {
        if (t->end() > Counter) {
            if (t->start < Counter) {
                PlayingTrack = t;
                if (videoItem) {
                    videoItem->setEnabled(trackVisible(t));
                    if (t->hasOpacity()) videoItem->setRenderOpacity(t->fadeOpacity(Counter));
                }
                break;
            }
            else if (t->start < Counter + ModRate) {
                StartingTrack = t;
                if (videoItem) {
                    videoItem->setEnabled(trackVisible(t));
                    videoItem->setRenderOpacity((t->hasOpacity()) ? t->fadeOpacity(Counter) : 1);
                    if (t->hasImage()) videoItem->setPlaybackImage(t->image);
                }
                break;
            }
        }
    }
    if ((StartingTrack == nullptr) && (PlayingTrack == nullptr)) {
        ReadBuffer.makeNull();
        if (videoItem) {
            videoItem->setEnabled(false);
            videoItem->setRenderOpacity(1);
        }
    }
    //qDebug() << m_Index << ReadBufferPos << StartingTrack << PlayingTrack;
    for (int OutBufferPos = 0; OutBufferPos < ModRate; OutBufferPos++) {
        if (PlayingTrack) {
            if (Counter >= PlayingTrack->end()) {
                if (ReadBuffer.isValid()) {
                    ReadBuffer.makeNull();
                    if (videoItem) {
                        if (!StartingTrack) videoItem->invokePause();
                    }
                }
            }
            else if (ReadBufferPos == 0) {
                const ulong64 trackPos = PlayingTrack->pos(Counter);
                //qDebug() << "playingtrack" << m_Index << ReadBufferPos << OutBufferPos << Counter << trackPos << PlayingTrack->waveGenerator.currentSample();
                if (!PlayingTrack->hasImage()) {
                    if (PlayingTrack->waveGenerator.currentSample() != trackPos) {
                        PlayingTrack->waveGenerator.skipTo(trackPos);
                        if (videoItem) {
                            if (trackVisible(PlayingTrack)) {
                                videoItem->invokeVideoPlayProperties(PlayingTrack,trackPos);
                                syncCounter = presets.SampleRate / (ModRate * 4);
                            }
                        }
                    }
                    else {
                        if (videoItem) {
                            if (trackVisible(PlayingTrack)) {
                                if (syncCounter-- <= 0) {
                                    videoItem->invokeVideoPlaySync(PlayingTrack,trackPos);
                                    syncCounter = presets.SampleRate / (ModRate * 4);
                                }
                            }
                        }
                    }
                    //qDebug() << PlayingTrack->waveGenerator.currentSample();
                    ReadBuffer.fromRawData(PlayingTrack->getNext(),PlayingTrack->channels(),ModRate);
                    if (!isZero(PlayingTrack->loopParameters.PitchShift)) pitchShift(PlayingTrack);
                }
                else {
                    ReadBuffer.makeNull();
                }
                Vol = PlayingTrack->fadeVolume(Counter);
            }
        }
        if (StartingTrack) {
            if (Counter >= StartingTrack->end()) {
                if (ReadBuffer.isValid()) {
                    ReadBuffer.makeNull();
                    if (videoItem) videoItem->invokePause();
                }
            }
            else if (Counter == StartingTrack->start) {
                //qDebug() << "startingtrack" << m_Index << ReadBufferPos << OutBufferPos << Counter << StartingTrack->startPos();
                if (!StartingTrack->hasImage()) {
                    StartingTrack->waveGenerator.reset();
                    StartingTrack->waveGenerator.skipTo(StartingTrack->startPos());
                    if (videoItem) {
                        if (trackVisible(StartingTrack)) {
                            videoItem->invokeVideoPlayProperties(StartingTrack,StartingTrack->startPos());
                            syncCounter = presets.SampleRate / (ModRate * 4);
                        }
                        videoItem->setEnabled(trackVisible(StartingTrack));
                    }
                    //qDebug() << "Start" << StartingTrack->waveGenerator.currentSample();
                    ReadBuffer.fromRawData(StartingTrack->getNext(),StartingTrack->channels(),ModRate);
                    if (!isZero(StartingTrack->loopParameters.PitchShift)) pitchShift(StartingTrack);
                }
                else {
                    ReadBuffer.makeNull();
                }
                Vol = StartingTrack->fadeVolume(Counter);
                //ReadBufferPos = 0;
            }
        }
        if (ReadBuffer.isValid()) {
            if (ReadBuffer.channels() == 1) {
                OutBuffer->setAt(OutBufferPos,ReadBuffer.at(ReadBufferPos,0)*Vol);
            }
            else {
                OutBuffer->setAt(OutBufferPos,ReadBuffer.at(ReadBufferPos,0)*Vol,ReadBuffer.at(ReadBufferPos,1)*Vol);
            }
            //qDebug() << OutBufferPos << ReadBufferPos << OutBuffer->data() << ReadBuffer.data();
        }
        else {
            OutBuffer->zeroAt(OutBufferPos);
        }
        ReadBufferPos++;
        if (ReadBufferPos >= ModRate) ReadBufferPos = 0;
        Counter++;
    }
    return OutBuffer;
}
*/
void CWaveLane::pitchShift(CWaveTrack* T)
{
    if (!ReadBuffer.isValid()) return;
    for (int c = 0; c < T->channels(); c++) {
        float* b = ReadBuffer.data()+(c * ReadBuffer.size());
        PS[c]->process(cent2Factor(T->loopParameters.PitchShift * 100.0),b,b);
    }
}

void CWaveLane::reset()
{
    Counter = 0;
    ReadBufferPos = 0;
    ReadBuffer.makeNull();
    for (CWaveTrack* T : std::as_const(tracks)) {
        if (!T->isValid) {
            tracks.removeOne(T);
            delete T;
        }
    }
}

void CWaveLane::UpdateGeometry(ldouble ZoomFactor, long CanvasRight)
{
    m_Zoom = ZoomFactor;
    for (CWaveTrack* T : std::as_const(tracks)) {
        if (!T->isValid) {
            tracks.removeOne(T);
            delete T;
        }
        else {
            T->geometry=QRect(sample2Pos(T->start),geometry.top()+1,ldouble(T->loopParameters.playLength())*ZoomFactor,geometry.height()-2);
        }
    }
    geometry.setRight(sample2Pos(samples()) + CanvasRight);
}

void CWaveLane::paint(QGraphicsScene& Scene, ldouble ZoomFactor, QRect visibleRect, bool Active)
{
    QPainterPath path;
    path.addRoundedRect(geometry,6,6);
    Scene.addPath(path,QPen(Qt::darkGray),QBrush((Active) ? Qt::lightGray : Qt::gray));

    for (CWaveTrack* t : std::as_const(tracks)) {
        if (!t->isActive) t->paint(Scene,ZoomFactor,visibleRect,0);
    }
    for (CWaveTrack* t : std::as_const(tracks)) {
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
    for (CWaveTrack* T : std::as_const(tracks)) {
        if (QFileInfo(T->name)==QFileInfo(Filename)) {
            tracks.removeOne(T);
            delete T;
        }
    }
    destroyVideoWidget();
}

void CWaveLane::addFile(CWaveTrack *t) {
    for (CWaveTrack* track : std::as_const(tracks)) {
        if (track) {
            if ((track != t) && (track->isValid) && (t->isValid)) {
                if ((t->end() >= track->start) && (t->start <= track->start)) {
                    t->cropEnd(track->start);
                }
            }
        }
    }
    tracks.append(t);
    sanityCheck(t);
    createVideoWidget();
}

CWaveTrack* CWaveLane::unserializeTrack(const QDomLiteElement* xml, ldouble ZoomFactor)
{
    if (!xml) return nullptr;
    const int Position=xml->attributeValueInt("Position");
    ulong64 Start=xml->attributeValueULongLong("StartPoint");
    if (Start==0) Start=ldouble(Position)/ZoomFactor;
    const QString Name = CPresets::resolveFilename(xml->attribute("Path"));

    const auto WT=new CWaveTrack(Name,Start);
    if (WT->isValid) {
        uint savedRate = xml->attributeValueInt("OrigRate",44100);
        if (savedRate != presets.SampleRate) {
            ldouble rateFactor = ldouble(presets.SampleRate)/ldouble(savedRate);
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
    for (const QDomLiteElement* xmlTrack : (const QDomLiteElementList)xml->elementsByTag("Track")) {
        unserializeTrack(xmlTrack,ZoomFactor);
    }
    if (QDomLiteElement* p = xml->elementByTag("Parameters")) unserializeParameters(p);
    QString id = xml->attribute("ID");
    if (!id.isEmpty()) ID = id;
    if (alias().isEmpty()) setAlias(ID);
    createVideoWidget();
    if (videoItem) videoItem->name = alias();
    videoVisible = xml->attributeValueBool("VideoVisible",true);
    if (videoItem) videoItem->unserialize(xml);
}

void CWaveLane::serialize(QDomLiteElement* xml) const
{
    for (const CWaveTrack* WT : tracks) {
        serializeTrack(xml->appendChild("Track"),WT);
    }
    xml->setAttribute("ID",ID);
    serializeParameters(xml->appendChild("Parameters"));
    xml->setAttribute("VideoVisible",videoVisible);
    if (videoItem) videoItem->serialize(xml);
}

ulong CWaveLane::milliSeconds() const
{
    ulong retval = 0;
    for (const CWaveTrack* t : tracks) {
        retval = qMax<ulong>(t->end(),retval);
    }
    return qMax<ulong>(presets.samplesTomSecs(retval), IDevice::milliSeconds());
}

ulong64 CWaveLane::samples() const
{
    ulong64 retval = 0;
    for (const CWaveTrack* t : tracks) {
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
        if (t->end() > Counter) {
            if (t->start <= Counter) {
                qDebug() << "Track found" << trackVisible(t);
                t->waveGenerator.reset();
                t->waveGenerator.skipTo(t->pos(Counter));
                if (videoItem) {
                    videoItem->setEnabled(trackVisible(t));
                    videoItem->setRenderOpacity(t->fadeOpacity(Counter));
                    if (t->hasVideo()) videoItem->setVideoStillProperties(t,presets.samplesTomSecs(t->pos(Counter)));
                    if (t->hasImage()) videoItem->setStillImage(t->image);
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
    ulong64 samples = presets.mSecsToSamples(mSec);
    bool gotFrame = false;
    if (videoItem) {
        for (CWaveTrack* t : std::as_const(tracks)) {
            if (t->end() > samples) {
                if (t->start <= samples) {
                    if (trackVisible(t)) {
                        videoItem->setRenderOpacity(t->fadeOpacity(samples));
                        if (t->hasVideo()) videoItem->setVideoExportProperties(t,presets.samplesTomSecs(t->pos(Counter)));
                        if (t->hasImage()) videoItem->setExportImage(t->image);
                        gotFrame = true;
                        //qDebug() << "gotFrame";
                    }
                }
            }
        }
        if (!gotFrame) {
            videoItem->setVideoExportEmptyFrame();
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
    if (qAbs(sample2Pos(sample) - timeLine->currentPos()) < 10) return true;
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
    if (tracks[DragTrack]->hasImage()) return;
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
    CWaveGenerator::LoopParameters LP = tracks[DragTrack]->loopParameters;
    LP.Start = 0;
    LP.End = tracks[DragTrack]->size;
    Scene.addItem(tracks[DragTrack]->waveGenerator.waveFormItem(waveRect,visibleRect.intersected(dragRect),m_Zoom,&LP));
}

long64 CWaveLane::handleMousePress(QPoint p) {
    StartPos = p;
    DragTrack = MouseOverTrack(p);
    if (DragTracks.isEmpty()) DragTracks.append(DragTrack);
    if (DragTrack > -1) {
        DragTrackStart = tracks[DragTrack]->start;
        DragTrackEnd = tracks[DragTrack]->end();
        waveRect = tracks[DragTrack]->geometry;
        waveRect.setLeft(sample2Pos(tracks[DragTrack]->waveStart()));
        waveRect.setRight(sample2Pos(tracks[DragTrack]->waveEnd()));
        if (StartPos.x() < sample2Pos(DragTrackStart) + 4) {
            DragTrackEdge = FrontEdge;
            return DragTrackStart;
        }
        else if (StartPos.x() > sample2Pos(DragTrackEnd) - 4) {
            DragTrackEdge = EndEdge;
            return tracks[DragTrack]->end();
        }
        else {
            DragTrackEdge = NoEdge;
            DragTrackStarts.clear();
            qDebug() << DragTracks << tracks;
            for (const int& i : std::as_const(DragTracks)) DragTrackStarts.append(tracks[i]->start);
            return DragTrackStart;
        }
    }
    return -1;
}

long64 CWaveLane::handleMouseMove(QPoint p, CTimeLine* timeLine) {
    if (DragTrack > -1) {
        CWaveTrack* d = tracks[DragTrack];
        if (p.x() != StartPos.x()) {
            if (DragTrackEdge == FrontEdge) {
                long64 newStart = DragTrackStart - (ldouble(StartPos.x()-p.x())/m_Zoom);
                newStart = snapTo(newStart, DragTrackStart, timeLine);
                if (newStart < 0) newStart = 0;
                if (qApp->queryKeyboardModifiers() & Qt::ShiftModifier) {
                    d->stretchStart(newStart);
                    waveRect = tracks[DragTrack]->geometry;
                    waveRect.setLeft(sample2Pos(tracks[DragTrack]->waveStart()));
                    waveRect.setRight(sample2Pos(tracks[DragTrack]->waveEnd()));
                }
                else {
                    d->cropStart(newStart);
                    if (d->length() < 1000) d->cropStart(d->end()-1000);
                }
                return d->start;
            }
            else if (DragTrackEdge == EndEdge) {
                long64 newEnd = DragTrackEnd - (ldouble(StartPos.x()-p.x())/m_Zoom);
                newEnd = snapTo(newEnd, DragTrackEnd, timeLine);
                if (qApp->queryKeyboardModifiers() & Qt::ShiftModifier) {
                    d->stretchEnd(newEnd);
                    waveRect = tracks[DragTrack]->geometry;
                    waveRect.setLeft(sample2Pos(tracks[DragTrack]->waveStart()));
                    waveRect.setRight(sample2Pos(tracks[DragTrack]->waveEnd()));
                }
                else {
                    d->cropEnd(newEnd);
                    if (d->length() < 1000) d->cropEnd(d->start+1000);
                }
                return d->end();
            }
            else {
                for (int i = 0; i < DragTracks.size(); i++) {
                    long64 newStart = DragTrackStarts[i] - (ldouble(StartPos.x()-p.x())/m_Zoom);
                    const long64 tLen = tracks[DragTracks[i]]->length();
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
                    if (d->end() > t->end()) {
                        if (t->end() > d->start) {
                            t->cropEnd(d->start);
                        }
                    }
                    if (d->start < t->start) {
                        if (t->start < d->end()) {
                            t->cropStart(d->end());
                        }
                    }
                }
            }
        }
        for (CWaveTrack* t : std::as_const(tracks)) {
            if (t) {
                if ((t != d) && (t->isValid) && (d->isValid)) {
                    if ((d->start >= t->start) && (d->end() <= t->end())) {
                        cloneTrack(t,m_Zoom)->cropStart(d->end());
                        t->cropEnd(d->start);
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


