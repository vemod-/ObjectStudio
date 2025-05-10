#include "cprojectapp.h"
/*
void CFileDocument::init() {
    menuFile = new QMenu("File",m_MainWindow);
    actionProjects = menuFile->addAction("Projects");//,this,&CFileDocument::showProjects);
    actionNew = menuFile->addAction("New",this,&CFileDocument::New,QKeySequence::New);
    actionOpen = menuFile->addAction("Open...",this,&CFileDocument::Open,QKeySequence::Open);
    actionWizard = menuFile->addAction("Quick Start...",this,&CFileDocument::Wizard);
    actionSave = menuFile->addAction("Save",this,&CFileDocument::Save,QKeySequence::Save);
    actionSaveAs = menuFile->addAction("Save as...",this,&CFileDocument::SaveAs,QKeySequence::SaveAs);
    menuRecent = new CRecentMenu(m_Organization,m_AppName,m_MainWindow);
    connect(menuRecent,&CRecentMenu::RecentTriggered,this,&CFileDocument::Recent);
    menuFile->addMenu(menuRecent);

    UndoMenu = new CUndoMenu(this,m_MainWindow);
}

void CFileDocument::New() {
    qDebug() << "CFileDocument New";
    if (!CleanDoc()) return;
    CSpinLabel l(m_MainWindow);
    CloseDoc();
    NewDoc();
    FileName.clear();
    UndoMenu->clearItems();
}

void CFileDocument::Recent(QString path) {
    qDebug() << "CFileDocument Recent";
    if (!CleanDoc()) return;
    CSpinLabel l(m_MainWindow);
    CloseDoc();
    OpenDoc(path);
    FileName = path;
    menuRecent->AddRecentFile(path);
    UndoMenu->clearItems();
}

void CFileDocument::Open() {
    qDebug() << "CFileDocument Open";
    if (!CleanDoc()) return;
    QFileDialog d(m_MainWindow);
    d.setFileMode(QFileDialog::ExistingFile);
    if (NameFilter.isEmpty()) NameFilter = "XML files (*.xml *.zip)";
    d.setNameFilter(NameFilter);
    d.setDirectory(m_DocumentPath);
    if (!FileName.isEmpty()) d.selectFile(FileName);
    if (d.exec()!=QDialog::Accepted) return;
    if (!d.selectedFiles().empty()) {
        QString fn=d.selectedFiles().first();
        if (!fn.isEmpty()) {
            if (QFileInfo::exists(fn)) {
                CSpinLabel l(m_MainWindow);
                CloseDoc();
                OpenDoc(fn);
                FileName = fn;
                menuRecent->AddRecentFile(fn);
                UndoMenu->clearItems();
            }
        }
    }
}

void CFileDocument::Save() {
    qDebug() << "CFileDocument Save";
    if (FileName.isEmpty()) {
        SaveAs();
        return;
    }
    CSpinLabel l(m_MainWindow);
    SaveDoc(FileName);
    UndoMenu->resetDirty();
}

void CFileDocument::SaveAs() {
    qDebug() << "CFileDocument SaveAs";
    const QString FilePath = QFileDialog::getSaveFileName(m_MainWindow,"Save XML file",m_DocumentPath,"XML files (*.xml)");
    if (!FilePath.isEmpty()) {
        CSpinLabel l(m_MainWindow);
        SaveDoc(FilePath);
        FileName = FilePath;
        menuRecent->AddRecentFile(FilePath);
        UndoMenu->resetDirty();
    }
}

void CFileDocument::Wizard() {
    qDebug() << "CFileDocument Wizard";
    if (!CleanDoc()) return;
    CSpinLabel l(m_MainWindow);
    CloseDoc();
    WizardDoc();
    FileName.clear();
    UndoMenu->clearItems();
}

bool CFileDocument::CleanDoc() {
    qDebug() << "CFileDocument CleanDoc";
    if (UndoMenu->isDirty())
    {
        int r = nativeAlert(m_MainWindow,m_AppName,"Save changes to the current project?",{"Save","No","Cancel"});
        if (r == 1000) Save();
        if (r == 1001) UndoMenu->resetDirty();
    }
    return !UndoMenu->isDirty();
}
*/
CProjectApp::CProjectApp(QWidget* DocumentWidget, CMainMenu* FileDocument, CProjectPage* ProjectPage, QWidget* parent)
    : QFadingWidget(parent), m_DocumentWidget(DocumentWidget), m_FileDocument(FileDocument) {
    m_MainWindow = parent;
    m_ProjectPage=ProjectPage;
    QVBoxLayout* l = new QVBoxLayout();
    QWidget* layoutWidget = new QWidget(this);
    layoutWidget->setLayout(l);
    l->setContentsMargins(0,0,0,0);
    l->setSpacing(0);
    l->addWidget(m_DocumentWidget);
    l->addWidget(m_ProjectPage);
    this->setWidget(layoutWidget);

    connect(m_FileDocument->actionProjects,&QAction::triggered,this,&CProjectApp::projects,Qt::DirectConnection);
    connect(m_ProjectPage,&CProjectPage::newProjectTriggered,this,&CProjectApp::newTriggered,Qt::DirectConnection);
    connect(m_ProjectPage,&CProjectPage::openProjectTriggered,this,&CProjectApp::openTriggered,Qt::DirectConnection);
    connect(m_ProjectPage,&CProjectPage::quickStartTriggered,this,&CProjectApp::wizardTriggered,Qt::DirectConnection);

    m_ProjectPage->setVisible(projectView);
    m_DocumentWidget->setVisible(!projectView);
    setView(!projectView);
}

void CProjectApp::openInit(QString path) {
    qDebug() << "CProjectApp openInit";
    setView(true);
    m_FileDocument->OpenDoc(path);
    m_FileDocument->FileName = path;
}
void CProjectApp::newInit() {
    qDebug() << "CProjectApp newInit";
    setView(true);
    m_FileDocument->NewDoc();
    m_FileDocument->FileName.clear();
}
void CProjectApp::projects()
{
    qDebug() << "CProjectApp projects";
    if (m_FileDocument->CleanDoc()) {
        m_FileDocument->FileName.clear();
        m_FileDocument->CloseDoc();
        m_ProjectPage->fillView();
        setView(false);
    }
}

void CProjectApp::openTriggered(QString path) {
    qDebug() << "CProjectApp openTriggered";
    //m_FileDocument->Recent(path);
    setView(true);
    m_FileDocument->OpenDoc(path);
    m_FileDocument->FileName = path;
    m_FileDocument->RecentMenu->AddRecentFile(path);
    m_FileDocument->UndoMenu->clearItems();
    emit Changed();
}
void CProjectApp::newTriggered() {
    qDebug() << "CProjectApp newTriggered";
    setView(true);
    m_FileDocument->NewDoc();
    m_FileDocument->FileName.clear();
    m_FileDocument->UndoMenu->clearItems();
    emit Changed();
}

void CProjectApp::wizardTriggered() {
    qDebug() << "CProjectApp wizardTriggered";
    setView(true);
    m_FileDocument->WizardDoc();
    m_FileDocument->FileName.clear();
    m_FileDocument->UndoMenu->clearItems();
    emit Changed();
}

void CProjectApp::ensureDocumentView() {
    if (projectView) {
        newInit();
    }
}

void CProjectApp::setView(bool view)
{
    qDebug() << "CProjectApp setView";
    for (const QMenuBar* m : (const QList<QMenuBar*>)m_MainWindow->findChildren<QMenuBar*>("",Qt::FindDirectChildrenOnly)) {
        for (QAction* a : (const QList<QAction*>)m->actions()) {
            if (a->menu()) a->menu()->setEnabled(view);
        }
    }
    for (QToolBar* t : (const QList<QToolBar*>)m_MainWindow->findChildren<QToolBar*>("",Qt::FindDirectChildrenOnly)) t->setEnabled(view);
    if (view != projectView) return;
    if (view)
    {
        setTransitionType(QFadingWidget::CoverLeft);
    }
    else
    {
        setTransitionType(QFadingWidget::UncoverRight);
    }
    projectView = !view;
    prepare();
    m_ProjectPage->setVisible(!view);
    m_DocumentWidget->setVisible(view);
    fade();
}
