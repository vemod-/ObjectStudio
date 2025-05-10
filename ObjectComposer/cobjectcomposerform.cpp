#include "cobjectcomposerform.h"
#include "ui_cobjectcomposerform.h"
//#include <QVBoxLayout>
#include <QFontDatabase>
//#include <QGuiApplication>
//#include <QScreen>

CObjectComposerForm::CObjectComposerForm(IDevice* Device, QWidget *parent) :
    CSoftSynthsForm(Device,false,parent),
    ui(new Ui::CObjectComposerForm)
{
    ui->setupUi(this);

#ifdef __STYLESHEETS__
    QFile qss(":/mac.qss");
    if (qss.open( QIODevice::ReadOnly ))
    {
        this->setStyleSheet( qss.readAll() );
        qss.close();
    }
#endif
    QFontDatabase::addApplicationFont(":/OCFMAC.TTF");
#ifdef __Lelandfont
    QFontDatabase::addApplicationFont(":/Leland.otf");
#endif

    if (!QDir(_DocumentPath).exists()) QDir(QStandardPaths::writableLocation(QStandardPaths::MusicLocation)).mkdir("Object Composer");

    OCSettings s;
    if (s.value("OCStuff").toString().isEmpty())
    {
        QDomLiteDocument XMLSettings(settingsfile);
        if (XMLSettings.documentElement->childCount())
        {
            s.setValue("OCStuff",XMLSettings.toString());
        }
    }

    setWindowTitle(apptitle);

    m_Document = new CScoreDoc(this, this);
    //m_DocLoader = new CDocumentLoader(doc,this);

    CProjectPage* proj = new CProjectPage(_DocumentPath,true,":/grey-paper-texture.jpg","mus",this);
    //proj->setStyleSheet("QListWidget{background:url(:/grey-paper-texture.jpg);}");
    //proj->setExtension("mus");
    //proj->setPath(_DocumentPath);
    //proj->fillView();

    CProjectApp* projectApp = new CProjectApp(m_Document,m_Document->MainMenu,proj,this);

    QVBoxLayout* lyo = new QVBoxLayout(this);
    lyo->setContentsMargins(0,0,0,0);
    lyo->setSpacing(0);
    this->setLayout(lyo);
    lyo->addWidget(projectApp);

    Device->addTickerDevice(&m_Document->MIDI2wav->DeviceList);
    Device->setDeviceParent(&m_Document->MIDI2wav->DeviceList);

    insideIn = (CInJack*)Device->jack(0)->createInsideJack(1,Device);
    m_Document->MIDI2wav->DeviceList.addJack(insideIn);
    m_Document->MIDI2wav->DeviceList.connect("This Out","MIDIFile2Wave Out");
    connect(m_Document->playControl,&OCPlayControl::VolChanged,this,&CObjectComposerForm::SetVol);
/*
    CZoomWidget* ZoomWidget = new CZoomWidget(this);
    connect(ZoomWidget,&CZoomWidget::valueChanged,m_DocLoader->document(),&CScoreDoc::SetZoom);
    connect(m_DocLoader->document(),&CScoreDoc::ZoomChanged,ZoomWidget,&CZoomWidget::setValue);

    CToolBar* leftToolBar=new CToolBar(this);
    leftToolBar->addWidget(ZoomWidget);

    CToolBar* playToolBar = new CToolBar(this);
    playToolBar->addAction(m_DocLoader->document()->playControl->getPlayButton());
    playToolBar->addAction(m_DocLoader->document()->playControl->getMixerButton());

    CToolBar* rightToolBar=new CToolBar(this);
    rightToolBar->addActions(m_DocLoader->document()->ToolBarActions->actions());
    rightToolBar->addActions(m_DocLoader->UndoMenu->actions());
    rightToolBar->addActions(m_DocLoader->document()->ToolBarActions2->actions());
    rightToolBar->addAction(m_DocLoader->document()->RightSideButton);

    CStatusBar* statusBar = new CStatusBar(this);
    statusBar->addSpacing(10);
    statusBar->addWidget(leftToolBar,1,Qt::AlignLeft);
    statusBar->addWidget(playToolBar,0,Qt::AlignHCenter);
    statusBar->addWidget(rightToolBar,1,Qt::AlignRight);

    lyo->addWidget(statusBar,1);
*/
    QMenuBar* menuBar = new QMenuBar(this);
    menuBar->setNativeMenuBar(false);
    lyo->setMenuBar(menuBar);

    //QMenu* fileMenu = menuBar->addMenu("File");
    //fileMenu->addActions(m_Document->MainMenu->FileMenu->actions());
    menuBar->addActions(m_Document->mainMenu->actions());

    projectApp->newInit();
}

CObjectComposerForm::~CObjectComposerForm()
{
    //m_DocLoader->canClose();
    m_Document->MainMenu->CleanDoc();
    m_Document->CloseDoc();
    //delete m_DocLoader;
    delete ui;
}

void CObjectComposerForm::initWithFile(const QString& path) {
    m_Document->MainMenu->Recent(path);
}

void CObjectComposerForm::SetVol(int v) {
    setParameter("Volume",v);
}

void CObjectComposerForm::UpdateVol(int v) {
    m_Document->playControl->SetVol(v);
}

void CObjectComposerForm::setMIDI2wavParameter(QString parameterName, int value) {
    m_Document->MIDI2wav->setParameter(parameterName, value);
}

void CObjectComposerForm::serializeCustom(QDomLiteElement *xml) const {
    m_Document->serialize(xml);
}

void CObjectComposerForm::unserializeCustom(const QDomLiteElement *xml) {
    m_Document->unserialize(xml);
}
