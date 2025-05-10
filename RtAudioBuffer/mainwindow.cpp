#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include <QFileDialog>
//#include <QStandardPaths>
#include "cpresetsform.h"
//#include "cthreadedfunction.h"
//#include <QGridLayout>
//#include <QGuiApplication>
//#include <QScreen>
//#include <QMenuBar>
#include "cprojectapp.h"

#ifdef Q_OS_IOS
#include "generated_plugin_registration.h"
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qDebug() << "mainwindow construct";
    ui->setupUi(this);
#ifdef Q_OS_IOS
    IOSPluginRegistration();
#endif
    setAcceptDrops(true);
    setWindowTitle("Object Studio");
    showMaximized();

    if (!QDir(_DocumentPath).exists()) QDir(QStandardPaths::writableLocation(QStandardPaths::MusicLocation)).mkdir("Object Studio");
    CProjectPage* proj = new CProjectPage(_DocumentPath,false,":/paper-texture.jpg","xml",this);
    m_ProjectApp = new CProjectApp(ui->centralWidget,ui->DesktopContainer->Desktop->MainMenu,proj,this);
    m_ProjectApp->m_ProjectPage->setCustomProjects(QStringList({":/New Wave Project.zip",":/New Composer Project.zip"}));
    m_ProjectApp->m_ProjectPage->fillView();
    this->setCentralWidget(m_ProjectApp);

    ui->dial->setKnobStyle(QSynthKnob::AluminiumStyle);
    ui->dial->setNotchStyle(QSynthKnob::LEDNotch);

    ui->DesktopContainer->Desktop->init(this,this);

    QMenuBar* menuBar = new QMenuBar(this);
    //centralWidget()->layout()->setMenuBar(menuBar);

    setMenuBar(menuBar);
    menuBar->setNativeMenuBar(true);
    menuFile = menuBar->addMenu("File");
    menuEdit = menuBar->addMenu("Edit");
    menuView = menuBar->addMenu("View");

    menuFile->addActions(ui->DesktopContainer->Desktop->MainMenu->FileMenu->actions());
    menuFile->addSeparator();
    actionExportWav = menuFile->addAction("Export Audio...",this,&MainWindow::exportWav);
    menuFile->addSeparator();
    actionPreferences = menuFile->addAction("Preferences...",QKeySequence::Preferences,this,&MainWindow::Preferences);
    actionPreferences->setMenuRole(QAction::ApplicationSpecificRole);
    menuFile->addSeparator();

    //menuEdit->addActions(ui->DesktopContainer->Desktop->MainMenu->UndoMenu->actions());
    //menuEdit->addSeparator();
    menuEdit->addActions(ui->DesktopContainer->Desktop->MainMenu->EditMenu->actions());

    actionDrivers = new QAction("Drivers",this);
    actionDrivers->setCheckable(true);
    connect(actionDrivers,&QAction::toggled,ui->DriverFrame,&QWidget::setVisible);
    menuView->addAction(actionDrivers);
    actionUImap = menuView->addAction("UI map",ui->DesktopContainer,&CDesktopContainer::showMap);
    actionHideUIs = menuView->addAction("Hide UIs",ui->DesktopContainer,&CDesktopContainer::hideUIs);
    actionCascadeUIs = menuView->addAction("Cascade UIs",this,&MainWindow::cascadeUIs);

    for (QAction* a : (const QList<QAction*>)menuBar->actions()) {
        if (a->menu()) a->menu()->setEnabled(false);
    }
    connect(ui->DesktopContainer->Desktop,&CDesktopComponent::MilliSecondsChanged,this,&MainWindow::Clear);
    connect(ui->TogglePlayButton,&QAbstractButton::toggled,this,&MainWindow::TogglePlay);
    connect(ui->ToggleRecordButton,&QAbstractButton::toggled,this,&MainWindow::ToggleRecord);
    connect(ui->DesktopContainer->Desktop,&CDesktopComponent::playStopped,this,&MainWindow::Stop);
    connect(ui->dial,&QAbstractSlider::valueChanged,this,&MainWindow::SetVolume);

    MainBuffers.init(0,this);
    MainBuffers.addTickerDevice(ui->DesktopContainer->Desktop->deviceList());
    for (int i=0;i<MainBuffers.jackCount();i++) ui->DesktopContainer->Desktop->addJack(MainBuffers.jack(i),0);

    ui->AudioInDriverCombo->addItems(MainBuffers.inDriverNames());
    ui->AudioInDriverCombo->setAttribute(Qt::WA_MacShowFocusRect,false);
    ui->AudioOutDriverCombo->addItems(MainBuffers.outDriverNames());
    ui->AudioOutDriverCombo->setAttribute(Qt::WA_MacShowFocusRect,false);

    qDebug() << "Create main buffer" << CPresets::presets().BufferSize << CPresets::presets().ModulationRate;
    MainBuffers.createBuffer();

    m_TimeLineSlider = new CTimeLineSlider(this);
    m_TimeLineSlider->init(&MainBuffers);
    ui->mainLayout->addWidget(m_TimeLineSlider);

    qDebug() << "Create buffer finished";
    ui->AudioInDriverCombo->blockSignals(true);
    ui->AudioInDriverCombo->setCurrentText(MainBuffers.inDriverName());
    ui->AudioInDriverCombo->blockSignals(false);

    ui->AudioOutDriverCombo->blockSignals(true);
    ui->AudioOutDriverCombo->setCurrentText(MainBuffers.outDriverName());
    ui->AudioOutDriverCombo->blockSignals(false);

    connect(ui->AudioInDriverCombo,&QComboBox::currentTextChanged,this,&MainWindow::InDriverChange);
    connect(ui->AudioOutDriverCombo,&QComboBox::currentTextChanged,this,&MainWindow::OutDriverChange);

    ui->DriverFrame->setVisible(false);

    qDebug() << MainBuffers.sampleRates();
    m_TimerID=startTimer(50);
    if (qApp->arguments().size() > 1) {
        if (QFileInfo(qApp->arguments().at(1)).exists()) dropfile(qApp->arguments().at(1));
    }
}

MainWindow::~MainWindow()
{
    killTimer(m_TimerID);
    m_TimerID=0;
    ui->DesktopContainer->Desktop->CloseDoc();
    MainBuffers.finish();
    ui->AudioInDriverCombo->clear();
    ui->AudioOutDriverCombo->clear();
#ifdef Q_OS_IOS
    CPresets::destroyInstance();
#endif
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent* /*event*/)
{
    if (!m_TimerID) return;
    if (!MainBuffers.driverCheck()) {
        ui->AudioInDriverCombo->blockSignals(true);
        ui->AudioOutDriverCombo->blockSignals(true);
        ui->AudioInDriverCombo->clear();
        ui->AudioOutDriverCombo->clear();

        ui->AudioInDriverCombo->addItems(MainBuffers.inDriverNames());
        ui->AudioInDriverCombo->setAttribute(Qt::WA_MacShowFocusRect,false);
        ui->AudioOutDriverCombo->addItems(MainBuffers.outDriverNames());
        ui->AudioOutDriverCombo->setAttribute(Qt::WA_MacShowFocusRect,false);

        ui->AudioInDriverCombo->setCurrentText(MainBuffers.inDriverName());
        ui->AudioOutDriverCombo->setCurrentText(MainBuffers.outDriverName());
        ui->AudioInDriverCombo->blockSignals(false);
        ui->AudioOutDriverCombo->blockSignals(false);
    }
    float L,R;
    MainBuffers.getPeak(L,R);
    ui->PeakMeter->setValues(L,R);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (!ui->DesktopContainer->Desktop->MainMenu->CleanDoc()) {
        event->ignore();
        return;
    }
    ui->DesktopContainer->Desktop->CloseDoc();
    event->accept();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e) {
    e->acceptProposedAction();
}

bool MainWindow::dropfile(const QString& path, QPoint pos) {
    m_ProjectApp->ensureDocumentView();
    if (pos==QPoint()) pos = ui->DesktopContainer->Desktop->rect().center();
    return ui->DesktopContainer->Desktop->initWithFile(path,pos);

}

void MainWindow::dropEvent(QDropEvent *e) {
    QPoint Pos(e->position().toPoint());
    const QMimeData* d = e->mimeData();
    qDebug() << d->urls() << d->html() << d->formats();
    if (d->urls().size()) {
        QString path = d->urls().first().toLocalFile();
        if (QFileInfo::exists(path)) {
            if (dropfile(path,Pos)) e->acceptProposedAction();
        }
    }
}

void MainWindow::setButton(QToolButton* b,const bool checked,const QString& icon)
{
    QMutexLocker locker(&mutex);
    b->blockSignals(true);
    b->setChecked(checked);
    b->blockSignals(false);
    b->setIcon(QIcon(icon));
}

void MainWindow::Clear()
{
    Stop();
    StopRecording();
    m_TimeLineSlider->draw();
}

void MainWindow::Play()
{
    ui->PeakMeter->reset();
    setButton(ui->TogglePlayButton,true,":/stop");
    MainBuffers.play(true);
    m_TimeLineSlider->draw();
}

void MainWindow::Continue()
{
    ui->PeakMeter->reset();
    setButton(ui->TogglePlayButton,true,":/stop");
    MainBuffers.play(false);
    m_TimeLineSlider->draw();
}

void MainWindow::Skip(const unsigned long long samples)
{
    setButton(ui->TogglePlayButton,true,":/stop");
    MainBuffers.skip(samples);
    m_TimeLineSlider->skip(samples);
    m_TimeLineSlider->draw();
}

void MainWindow::Stop()
{
    setButton(ui->TogglePlayButton,false,":/play");
     MainBuffers.pause();
    MainBuffers.panic();
    m_TimeLineSlider->draw();
}

void MainWindow::Record()
{
    ui->PeakMeter->reset();
    ui->ToggleRecordButton->blockSignals(true);
    ui->ToggleRecordButton->setChecked(true);
    ui->ToggleRecordButton->blockSignals(false);
    ui->ToggleRecordButton->setIcon(QIcon(":/save"));
    MainBuffers.startRecording();
    if (!MainBuffers.isPlaying()) Play();
    m_TimeLineSlider->draw();
}

void MainWindow::StopRecording()
{
    if (!MainBuffers.isRecording()) return;
    ui->ToggleRecordButton->blockSignals(true);
    ui->ToggleRecordButton->setChecked(false);
    ui->ToggleRecordButton->blockSignals(false);
    ui->ToggleRecordButton->setIcon(QIcon(":/record"));
    MainBuffers.stopRecording();
    QString fileName=QFileDialog::getSaveFileName(this,"Save Wave File",QStandardPaths::writableLocation(QStandardPaths::MusicLocation),WaveFile::WaveFilter);
    if (!fileName.isEmpty()) MainBuffers.saveRecording(fileName);
}

void MainWindow::exportWav()
{
    QString fileName=QFileDialog::getSaveFileName(this,"Export Wave File",QStandardPaths::writableLocation(QStandardPaths::MusicLocation),WaveFile::WaveFilter);
    if (!fileName.isEmpty())
    {
        CConcurrentDialog::run(&MainBuffers,&CCoreMainBuffers::render,fileName);
    }
}

void MainWindow::TogglePlay(bool value)
{
    (value) ? Play() : Stop();
}

void MainWindow::ToggleRecord(bool value)
{
    (value) ? Record() : StopRecording();
}

void MainWindow::SetVolume(int vol)
{
    MainBuffers.outputVol=vol*0.01f;
}

void MainWindow::InDriverChange(QString inDriverName)
{
    MainBuffers.setInDriver(inDriverName);
    ui->AudioInDriverCombo->blockSignals(true);
    ui->AudioInDriverCombo->setCurrentText(MainBuffers.inDriverName());
    ui->AudioInDriverCombo->blockSignals(false);
}

void MainWindow::OutDriverChange(QString outDriverName)
{
    MainBuffers.setOutDriver(outDriverName);
    ui->AudioOutDriverCombo->blockSignals(true);
    ui->AudioOutDriverCombo->setCurrentText(MainBuffers.outDriverName());
    ui->AudioOutDriverCombo->blockSignals(false);
}

void MainWindow::Preferences()
{
    CPresetsForm* f=new CPresetsForm(this);
    f->fill(MainBuffers.sampleRates());
    f->setModal(true);
    if (f->exec())
    {
        QCoreApplication::quit();
        //close();
        //QProcess::startDetached(qApp->applicationFilePath(), qApp->arguments());
    }
}

void MainWindow::cascadeUIs()
{
    QPoint p(geometry().topLeft()+QPoint(24,24));
    ui->DesktopContainer->cascadeUIs(p);
}

void MainWindow::skip(const ulong64 samples) {
    qDebug() << "MainSkip" << samples;
    Skip(samples);
}

void MainWindow::play(const bool FromStart) {
    qDebug() << "MainPlay";
    if (FromStart) {
        Play();
    }
    else
    {
        Continue();
    }
}

void MainWindow::pause() {
    qDebug() << "MainPause";
    Stop();
}

ulong MainWindow::ticks() const {
    return MainBuffers.ticks();
}

ulong MainWindow::milliSeconds() const {
    return MainBuffers.milliSeconds();
}

ulong64 MainWindow::samples() const {
    return MainBuffers.samples();
}

bool MainWindow::isPlaying() const {
    return MainBuffers.isPlaying();
}

ulong64 MainWindow::currentSample() const {
    return MainBuffers.currentSample();
}

ulong MainWindow::currentMilliSecond() const {
    return MainBuffers.currentMilliSecond();
}

void MainWindow::renderWaveFile(const QString path) {
    MainBuffers.render(path);
}
