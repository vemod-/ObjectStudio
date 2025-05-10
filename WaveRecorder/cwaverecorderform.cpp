#include "cwaverecorderform.h"
#include "ui_cwaverecorderform.h"
#include "cwaverecorder.h"
#include "cwavefile.h"
#include <QFileDialog>
#include <QScrollBar>
#include <QFileSystemModel>
#include <QMenuBar>
#include <QMainWindow>
#include <QDrag>
#include "qmacsplitter.h"

#define _DocumentPath QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/Object Waves/"

CWaveRecorderForm::CWaveRecorderForm(IDevice* Device, QWidget *parent) :
    CSoftSynthsForm(Device,false,parent),
    ui(new Ui::CWaveRecorderForm)
{
    ui->setupUi(this);

    if (!QDir(_DocumentPath).exists()) QDir(QStandardPaths::writableLocation(QStandardPaths::MusicLocation)).mkdir("Object Waves");

#ifdef Q_OS_IOS
    m_Document = new CWaveDocument(ui->WaveLanes,"Veinge Musik och Data","ObjectWaves",_DocumentPath,this);
#else
    m_Document = new CWaveDocument(ui->WaveLanes,"http://www.musiker.nu/objectstudio","ObjectWaves",_DocumentPath,this);
#endif
    CProjectPage* proj = new CProjectPage(_DocumentPath,false,":/Brushed Aluminium 3 Tile.bmp","xml",this);
    CProjectApp* projectApp = new CProjectApp(ui->BaseWidget,m_Document->MainMenu,proj,this);
    layout()->addWidget(projectApp);

    m_Document->MainMenu->FileMenu->addSeparator();
    m_Document->MainMenu->FileMenu->addAction("Skip to Start",QKeySequence::MoveToStartOfLine,this,&CWaveRecorderForm::SkipToStart);
    QAction* actionMoveToStart = new QAction(this);
    actionMoveToStart->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Left));
    addAction(actionMoveToStart);
    connect(actionMoveToStart,&QAction::triggered,this,&CWaveRecorderForm::SkipToStart);

    QAction* actionTogglePlay = m_Document->MainMenu->FileMenu->addAction("Toggle Play");
    actionTogglePlay->setShortcut(Qt::Key_Space);
    connect(actionTogglePlay,&QAction::triggered,this,&CWaveRecorderForm::togglePlay,Qt::DirectConnection);
    QAction* actionTogglePlayFromStart = m_Document->MainMenu->FileMenu->addAction("Toggle Play From Start");
    actionTogglePlayFromStart->setShortcut(Qt::SHIFT | Qt::Key_Space);
    connect(actionTogglePlayFromStart,&QAction::triggered,ui->TogglePlayButton,&QAbstractButton::toggle,Qt::DirectConnection);

    m_Document->MainMenu->FileMenu->addSeparator();
    m_Document->MainMenu->FileMenu->addAction("&Import...",this,&CWaveRecorderForm::Import);
    m_Document->MainMenu->FileMenu->addAction("&Remove..",this,&CWaveRecorderForm::Remove);

    ui->FileList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->FileList->setDragEnabled(true);
    ui->FileList->setDragDropMode(QListWidget::DragOnly);
    ui->RecordTree->setDragEnabled(true);
    ui->RecordTree->setDragDropMode(QListWidget::DragOnly);
    m_RecordPath =QStandardPaths::locate(QStandardPaths::MusicLocation, QString(), QStandardPaths::LocateDirectory)+"/WaveRecorder/Recorded Files";
    auto recordfilemodel = new QFileSystemModel(this);
    recordfilemodel->setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    recordfilemodel->setNameFilterDisables(false);
    recordfilemodel->setRootPath(m_RecordPath);
    ui->RecordTree->setModel(recordfilemodel);
    ui->RecordTree->setRootIndex(recordfilemodel->setRootPath(m_RecordPath));

    ui->FileTree->setDragEnabled(true);
    ui->FileTree->setDragDropMode(QTreeView::DragOnly);
    QString path =QStandardPaths::locate(QStandardPaths::MusicLocation, QString(), QStandardPaths::LocateDirectory);
    auto filemodel = new QFileSystemModel(this);
    filemodel->setFilter(QDir::Files | QDir::Dirs);
    filemodel->setNameFilterDisables(false);
    filemodel->setRootPath(path);
    ui->FileTree->setModel(filemodel);
    ui->FileTree->setRootIndex(filemodel->setRootPath(path));

    connect(ui->RecordButton,&QSynthCheckbox::toggled,this,&CWaveRecorderForm::Record);
    connect(ui->FileList,&QWidget::customContextMenuRequested,this,&CWaveRecorderForm::FileMenuPopup);
    connect(ui->FileList,&QListWidget::itemChanged,this,&CWaveRecorderForm::RenameFile);
    connect(ui->AddButton,&QAbstractButton::clicked,this,&CWaveRecorderForm::Import);
    connect(ui->RemoveButton,&QAbstractButton::clicked,this,&CWaveRecorderForm::Remove);
    connect(ui->DeleteButton,&QAbstractButton::clicked,this,&CWaveRecorderForm::DeleteFile);
    connect(ui->WaveLanes,&CWaveLanes::FileAdded,this,&CWaveRecorderForm::addFile,Qt::QueuedConnection);
    connect(ui->WaveLanes,&CWaveLanes::FileRemoved,this,&CWaveRecorderForm::CheckList,Qt::QueuedConnection);

    connect(ui->ZoomInButton,&QAbstractButton::clicked,ui->WaveLanes,&CWaveLanes::zoomIn);
    connect(ui->ZoomOutButton,&QAbstractButton::clicked,ui->WaveLanes,&CWaveLanes::zoomOut);
    connect(ui->ZoomMinButton,&QAbstractButton::clicked,ui->WaveLanes,&CWaveLanes::zoomMin);
    connect(ui->ZoomMaxButton,&QAbstractButton::clicked,ui->WaveLanes,&CWaveLanes::zoomMax);

    connect(ui->TogglePlayButton,&QAbstractButton::toggled,this,&CWaveRecorderForm::Play);
    connect(ui->ToggleRecordButton,&QAbstractButton::toggled,this,&CWaveRecorderForm::RecordFromStart);
    connect(ui->MonitorButton,&QToggleButton::toggled,this,&CWaveRecorderForm::SetMonitor);
    connect(ui->InVolKnob,&QSynthKnob::valueChanged,this,&CWaveRecorderForm::SetMonitorLevel);

    ui->InVolKnob->setNotchStyle(QSynthKnob::dBNotch);
    ui->WaveLanes->setAutoFillBackground(false);
    startTimer(40);
    ui->RecordVol->setLock(true);
    ui->RecordVol->setLeftVol(100);
    ui->WaveLanes->m_MixerWidget = new CMixerWidget(this);
    ui->MixerLayout_2->addWidget(ui->WaveLanes->m_MixerWidget);
    ui->scrollAreaWidgetContents->setLayout(ui->MixerLayout_2);
    ui->WaveLanes->init(1,parent);
    ui->MixerLayoutWidget->setMaximumHeight(ui->WaveLanes->m_MixerWidget->sizeHint().height());

    QMenuBar* menuBar = new QMenuBar(this);
    menuBar->setNativeMenuBar(false);
    menuBar->addMenu(m_Document->MainMenu->FileMenu);
    QMenu* EditMenu = menuBar->addMenu("Edit");
    EditMenu->addActions(m_Document->MainMenu->EditMenu->actions());

    layout()->setMenuBar(menuBar);

    Device->addTickerDevice(ui->WaveLanes);
    ui->ScrollBar->setVisible(false);
    ui->NavigationLayout->replaceWidget(ui->ScrollBar,ui->WaveLanes->horizontalScrollBar());

    auto s = new QMacSplitter(ui->BaseWidget);
    ui->BaseWidget->layout()->addWidget(s);
    s->setOrientation(Qt::Horizontal);
    s->addWidget(ui->FileListWidget);
    ui->splitter->setHandleWidth(0);
    s->addWidget(ui->splitter);
    s->setStretchFactor(0,1);
    s->setStretchFactor(1,100);

    ui->scrollArea->setStyleSheet("QScrollArea{background:url(:/Black Aluminium Tile.jpg);}");
    setStyleSheet("QTreeView,QListWidget{background:url(:/Brushed Aluminium 3 Tile.bmp);}");
    m_TimerID =  startTimer(50);
    projectApp->newInit();
}

CWaveRecorderForm::~CWaveRecorderForm()
{
    if (m_TimerID) killTimer(m_TimerID);
    m_TimerID=0;
    m_Document->MainMenu->CleanDoc();
    m_Document->CloseDoc();

    delete ui;
}

void CWaveRecorderForm::setHost(IHost* h)
{
    ui->WaveLanes->setHost(h);
}

float CWaveRecorderForm::volumeL()
{
    return ui->RecordVol->leftVol() * 0.01f;
}

float CWaveRecorderForm::volumeR()
{
    return ui->RecordVol->rightVol() * 0.01f;
}

void CWaveRecorderForm::showMixer(bool show)
{
    ui->WaveLanes->execute(show);
}

void CWaveRecorderForm::SetMonitor(bool v) {
    auto w = DEVICEFUNC(CWaveRecorder);
    w->m_Monitor = v;
}

void CWaveRecorderForm::SetMonitorLevel(int v) {
    auto w = DEVICEFUNC(CWaveRecorder);
    w->m_MonitorLevel = v*0.01;
}

void CWaveRecorderForm::RecordFromStart(bool v) {
    Play(v);
    ui->RecordButton->setValue(v);
}

void CWaveRecorderForm::Record(bool v)
{
    auto w = DEVICEFUNC(CWaveRecorder);
    if (v)
    {
        ui->ToggleRecordButton->setIcon(QIcon(":/save"));
        w->startRecording();
    }
    else
    {
        ui->ToggleRecordButton->setIcon(QIcon(":/record"));
        w->finishRecording();
        QDateTime now = QDateTime::currentDateTime();
        QDir dir(m_RecordPath);
        if (!dir.exists()) dir.mkpath(dir.absolutePath());
        QString path = m_RecordPath + "/" + now.toString("yyMMdd HH-mm-ss") + ".wav";
        if (w->saveAs(path)) addFile(path);
    }
}

void CWaveRecorderForm::SkipToStart() {
    auto w = DEVICEFUNC(CWaveRecorder);
    w->requestPause();
    w->requestSkip(0);
}

void CWaveRecorderForm::togglePlay() {
    auto w = DEVICEFUNC(CWaveRecorder);
    if (w->requestIsPlaying()) {
        w->requestPause();
        return;
    }
    w->requestPlay(w->requestCurrentSample() == 0);
}

void CWaveRecorderForm::Play(bool v) {
    auto w = DEVICEFUNC(CWaveRecorder);
    (v) ? w->requestPlay(true) : w->requestPause();
}

void CWaveRecorderForm::setPlayIcon(bool v) {
    (v) ? ui->TogglePlayButton->setIcon(QIcon(":/stop")) : ui->TogglePlayButton->setIcon(QIcon(":/play"));
}

void CWaveRecorderForm::timerEvent(QTimerEvent* /*e*/)
{
    if (!m_TimerID) return;
    auto w = DEVICEFUNC(CWaveRecorder);
    ui->RecordVol->peak(w->PeakL,w->PeakR);
    w->PeakL=0;
    w->PeakR=0;
}

void CWaveRecorderForm::initWithFile(const QString& path) {
    if (path.endsWith(".aup",Qt::CaseInsensitive)) {
        ui->WaveLanes->MainMenu->Recent(path);
        ui->WaveLanes->paint();
    }
    else {
        ui->WaveLanes->AddFile(path,0);
    }
}

void CWaveRecorderForm::Import()
{
    QFileDialog d(this);
    d.setFileMode(QFileDialog::ExistingFiles);
    d.setNameFilter(WaveFile::WaveFilter);
    d.setWindowTitle("Import Wave");
    d.setDirectory(QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
    if (d.exec() != QFileDialog::Accepted) return;
    for (const QString& p : (const QStringList)d.selectedFiles()) addFile(p);
}

void CWaveRecorderForm::addFile(QString path)
{
    if (!path.isEmpty())
    {
        if (!QFileInfo::exists(path)) return;
        QListWidget* w = ui->FileList;
        for (int i = 0; i < w->count(); i++)
        {
            if (QFileInfo(w->item(i)->data(34).toString()) == QFileInfo(path))
            {
                CheckList();
                return;
            }
        }
        auto item=new QListWidgetItem;
        item->setText(QFileInfo(path).baseName());
        item->setData(34,path);
        item->setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
        w->addItem(item);
        CheckList();
    }
}

void CWaveRecorderForm::CheckList()
{
    QListWidget* w = ui->FileList;
    for (int i = 0; i < w->count(); i++)
    {
        w->item(i)->setCheckState(ui->WaveLanes->fileInUse(w->item(i)->data(34).toString()) ? Qt::Checked : Qt::Unchecked);
    }
}

void CWaveRecorderForm::Remove()
{
    const int Index=ui->FileList->currentRow();
    if (Index>-1)
    {
        QString path = ui->FileList->item(Index)->data(34).toString();
        if (ui->WaveLanes->fileInUse(path))
        {
            int r = nativeAlert(this,"Object Waves","The File is In Use! Do you want to remove it anyway?",{"Cancel","Yes"});
            if (r == 1000) return;
        }
        ui->WaveLanes->removeFile(path);
        delete ui->FileList->takeItem(Index);
    }
}

void CWaveRecorderForm::RenameFile(QListWidgetItem* item)
{
    QString path = item->data(34).toString();
    QFile f(path);
    QString newpath = QFileInfo(f).path() + "/" + item->text()+"."+QFileInfo(f).completeSuffix();
    f.rename(newpath);
    item->setData(34,QFileInfo(f).filePath());
    item->setText(QFileInfo(f).baseName());
    ui->WaveLanes->renameFile(path,QFileInfo(f).filePath());
    ui->WaveLanes->paint();
}

void CWaveRecorderForm::DeleteFile()
{
    int Index=ui->FileList->currentRow();
    if (Index>-1)
    {
        QString path = ui->FileList->item(Index)->data(34).toString();
        if (ui->WaveLanes->fileInUse(path))
        {
            int r = nativeAlert(this,"The File is In Use!","Do you want to delete it anyway?",{"Cancel","Yes"});
            if (r == 1000) return;
        }
        ui->WaveLanes->removeFile(path);
        QFile(path).remove();
        delete ui->FileList->takeItem(Index);
    }
}

void CWaveRecorderForm::FileMenuPopup(QPoint Pos)
{
    m_Document->MainMenu->FileMenu->popup(ui->FileList->mapToGlobal(Pos));
}

void CWaveRecorderForm::serializeCustom(QDomLiteElement* xml) const
{
    ui->WaveLanes->serialize(xml->appendChild("Lanes"));
    xml->setAttribute("RecordLock",ui->RecordVol->lock());
    xml->setAttribute("RecordVolumeL",ui->RecordVol->leftVol());
    xml->setAttribute("RecordVolumeR",ui->RecordVol->rightVol());
    xml->setAttribute("View",ui->BaseWidget->isVisible());
    QDomLiteElement* xmlfiles=xml->appendChild("Files");
    for (int i = 0; i < ui->FileList->count(); i++)
    {
        xmlfiles->appendChild("File","Path",ui->FileList->item(i)->data(34).toString());
    }
}

void CWaveRecorderForm::unserializeCustom(const QDomLiteElement* xml)
{
    if (!xml) return;
    ui->WaveLanes->unserialize(xml->elementByTag("Lanes"));
    ui->RecordVol->setLock(xml->attributeValueBool("RecordLock",false));
    ui->RecordVol->setLeftVol(xml->attributeValueInt("RecordVolumeL",100));
    ui->RecordVol->setRightVol(xml->attributeValueInt("RecordVolumeR",100));
    ui->FileList->clear();
    if (const QDomLiteElement* xmlfiles=xml->elementByTag("Files"))
    {
        for (const QDomLiteElement* e : (const QDomLiteElementList)xmlfiles->elementsByTag("File"))
        {
            addFile(CPresets::resolveFilename(e->attribute("Path")));
        }
    }
    QList<QFileInfo> fileList;
    for (int i = 0; i < ui->FileList->count(); i++)
    {
        fileList.append(QFileInfo(ui->FileList->item(i)->data(34).toString()));
    }
    for (const QString& s : ui->WaveLanes->fileList())
    {
        if (!fileList.contains(QFileInfo(s))) addFile(CPresets::resolveFilename(s));
    }
}

CAudioBuffer* CWaveRecorderForm::getNextA(const int ProcIndex)
{
    return ui->WaveLanes->getNextA(ProcIndex);
}

