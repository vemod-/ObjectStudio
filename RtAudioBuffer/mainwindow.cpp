#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_Playing=false;
    ui->dial->setKnobStyle(QSynthKnob::AluminiumStyle);
    ui->dial->setNotchStyle(QSynthKnob::LEDNotch);

    ui->DesktopContainer->Desktop->MainWindow=this;
    ui->DesktopContainer->Desktop->SetFileMenu(ui->menuFile);
    connect(ui->actionNew,SIGNAL(triggered()),ui->DesktopContainer->Desktop,SLOT(New()));
    connect(ui->actionOpen,SIGNAL(triggered()),ui->DesktopContainer->Desktop,SLOT(Open()));
    connect(ui->actionSave,SIGNAL(triggered()),ui->DesktopContainer->Desktop,SLOT(Save()));
    connect(ui->actionSave_as,SIGNAL(triggered()),ui->DesktopContainer->Desktop,SLOT(SaveAs()));

    connect(ui->TogglePlayButton,SIGNAL(toggled(bool)),this,SLOT(TogglePlay(bool)));
    connect(ui->ToggleRecordButton,SIGNAL(toggled(bool)),this,SLOT(ToggleRecord(bool)));
    connect(ui->DesktopContainer->Desktop,SIGNAL(StopPlaying()),this,SLOT(Stop()));
    connect(ui->dial,SIGNAL(valueChanged(int)),this,SLOT(SetVolume(int)));
    startTimer(50);

    MainBuffers.SetHost(ui->DesktopContainer->Desktop);
    MainBuffers.Init(0,this);
    for (int i=0;i<MainBuffers.JackCount();i++)
    {
        ui->DesktopContainer->Desktop->AddJack(MainBuffers.GetJack(i),0);
        qDebug() << i;
    }

    ui->AudioInDriverCombo->addItems(MainBuffers.DeviceList(1));
    ui->AudioInDriverCombo->setAttribute(Qt::WA_MacShowFocusRect,false);
    ui->AudioOutDriverCombo->addItems(MainBuffers.DeviceList(0));
    ui->AudioOutDriverCombo->setAttribute(Qt::WA_MacShowFocusRect,false);

    qDebug() << "Create main buffer";

    MainBuffers.CreateBuffer();

    qDebug() << "Create buffer finished";
}

MainWindow::~MainWindow()
{
    MainBuffers.Finish();
    ui->DesktopContainer->Desktop->Clear();
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent* /*event*/)
{
    float L,R;
    MainBuffers.getPeak(L,R);
    ui->PeakMeter->SetValues(L,R);
}

void MainWindow::Play()
{
    m_Playing=true;
    ui->PeakMeter->Reset();
    ui->TogglePlayButton->blockSignals(true);
    ui->TogglePlayButton->setChecked(true);
    ui->TogglePlayButton->blockSignals(false);
    ui->TogglePlayButton->setIcon(QIcon(":/stop"));
    //ui->StopButton->setChecked(false);
    //ui->PlayButton->setChecked(true);
    ui->DesktopContainer->Desktop->Play(true);
}

void MainWindow::Stop()
{
    m_Playing=false;
    ui->TogglePlayButton->blockSignals(true);
    ui->TogglePlayButton->setChecked(false);
    ui->TogglePlayButton->blockSignals(false);
    ui->TogglePlayButton->setIcon(QIcon(":/play"));
    //ui->StopButton->setChecked(true);
    //ui->PlayButton->setChecked(false);
    ui->DesktopContainer->Desktop->Pause();
    MainBuffers.Panic();
}

void MainWindow::Record()
{
    ui->PeakMeter->Reset();
    ui->ToggleRecordButton->blockSignals(true);
    ui->ToggleRecordButton->setChecked(true);
    ui->ToggleRecordButton->blockSignals(false);
    ui->ToggleRecordButton->setIcon(QIcon(":/save"));
    MainBuffers.StartRecording();
    //ui->RecordButton->setChecked(true);
    //ui->StopRecordButton->setChecked(false);
    if (!m_Playing) Play();
}

void MainWindow::StopRecording()
{
    //ui->RecordButton->setChecked(false);
    //ui->StopRecordButton->setChecked(true);
    ui->ToggleRecordButton->blockSignals(true);
    ui->ToggleRecordButton->setChecked(false);
    ui->ToggleRecordButton->blockSignals(false);
    ui->ToggleRecordButton->setIcon(QIcon(":/record"));
    MainBuffers.StopRecording();
    QString fileName=QFileDialog::getSaveFileName(this,"Save Wave File",QStandardPaths::writableLocation(QStandardPaths::MusicLocation),WaveFile::WaveFilter);
    if (!fileName.isEmpty()) MainBuffers.SaveRecording(fileName);
}

void MainWindow::TogglePlay(bool value)
{
    if (value)
    {
        Play();
    }
    else
    {
        Stop();
    }
}

void MainWindow::ToggleRecord(bool value)
{
    if (value)
    {
        Record();
    }
    else
    {
        StopRecording();
    }
}

void MainWindow::SetVolume(int vol)
{
    MainBuffers.outputVol=vol*0.01;
}
