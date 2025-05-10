#ifndef CPROJECTAPP_H
#define CPROJECTAPP_H
#include "../../QFadingWidget/qfadingwidget.h"
#include "../Projectpage/cprojectpage.h"
#include <QMenuBar>
#include <QToolBar>
#include <QVBoxLayout>
#include "cundomenu.h"
#include "crecentmenu.h"
#include <QFileDialog>
#include "idevice.h"
#include <QGraphicsView>
#include "cundomenu.h"
#include "ceditmenu.h"
#include "cthreadedfunction.h"

#ifndef __FileDocumentBaseClass
#define __FileDocumentBaseClass QGraphicsView
#endif

class IFileDocument : public IUndoSerializer, public IEditDocument {
public:
    virtual void OpenDoc(QString) {
        qDebug() << "Virtual opendoc";
    }
    virtual void SaveDoc(QString) {
        qDebug() << "Virtual savedoc";
    }
    virtual void CloseDoc() {
        qDebug() << "Virtual closedoc";
    }
    virtual void NewDoc() {
        qDebug() << "Virtual newdoc";
    }
    virtual void WizardDoc() {
        qDebug() << "Virtual wizarddoc";
    }
};

class CMainMenu : public QMenu {
    Q_OBJECT
public:
    CMainMenu(IFileDocument* doc, QWidget* mainWindow, QString Organization, QString AppName, QString Filter, QString DocumentPath, QWidget* parent = nullptr)
        : QMenu("Menu",parent), m_Document(doc), m_MainWindow(mainWindow), m_Organization(Organization), m_AppName(AppName), m_NameFilter(Filter), m_DocumentPath(DocumentPath) {
        if (m_NameFilter.isEmpty()) m_NameFilter = "XML files (*.xml *.zip)";
        FileMenu = new QMenu("File",m_MainWindow);
        actionProjects = FileMenu->addAction("Projects");//,this,&CFileMenu::showProjects);
        actionNew = FileMenu->addAction("New");
        actionNew->setShortcut(QKeySequence::New);
        connect(actionNew,&QAction::triggered,this,&CMainMenu::New,Qt::DirectConnection);
        actionOpen = FileMenu->addAction("Open...",QKeySequence::Open,this,&CMainMenu::Open);
        actionWizard = FileMenu->addAction("Quick Start...",this,&CMainMenu::Wizard);
        actionSave = FileMenu->addAction("Save");
        actionSave->setShortcut(QKeySequence::Save);
        connect(actionSave,&QAction::triggered,this,&CMainMenu::Save,Qt::DirectConnection);
        actionSaveAs = FileMenu->addAction("Save as...",QKeySequence::SaveAs,this,&CMainMenu::SaveAs);
        RecentMenu = new CRecentMenu(m_Organization,m_AppName,m_MainWindow);
        connect(RecentMenu,&CRecentMenu::RecentTriggered,this,&CMainMenu::Recent,Qt::DirectConnection);
        FileMenu->addMenu(RecentMenu);

        UndoMenu = new CUndoMenu(doc,m_MainWindow);
        EditMenu = new CEditMenu(doc,m_AppName,m_MainWindow);

        EditMenu->insertActions(EditMenu->actionCut,UndoMenu->actions());
        EditMenu->insertSeparator(EditMenu->actionCut);
        /*
        QList<QAction*> l;
        l.append(EditMenu->actions());
        for (QAction* a : EditMenu->actions()) EditMenu->removeAction(a);
        EditMenu->addActions(UndoMenu->actions());
        EditMenu->addSeparator();
        EditMenu->addActions(l);
*/
    }
    void New() {
        qDebug() << "CFileDocument New";
        if (!CleanDoc()) return;
        CSpinLabel l(m_MainWindow);
        m_Document->CloseDoc();
        m_Document->NewDoc();
        FileName.clear();
        UndoMenu->clearItems();
    }
    void Recent(QString path) {
        qDebug() << "CFileDocument Recent";
        if (!CleanDoc()) return;
        CSpinLabel l(m_MainWindow);
        m_Document->CloseDoc();
        m_Document->OpenDoc(path);
        FileName = path;
        RecentMenu->AddRecentFile(path);
        UndoMenu->clearItems();
    }
    void Open() {
        qDebug() << "CFileDocument Open";
        if (!CleanDoc()) return;
        QFileDialog d(m_MainWindow);
        d.setFileMode(QFileDialog::ExistingFile);
        d.setNameFilter(m_NameFilter);
        d.setDirectory(m_DocumentPath);
        if (!FileName.isEmpty()) d.selectFile(FileName);
        if (d.exec()!=QDialog::Accepted) return;
        if (!d.selectedFiles().empty()) {
            QString fn=d.selectedFiles().first();
            if (!fn.isEmpty()) {
                if (QFileInfo::exists(fn)) {
                    CSpinLabel l(m_MainWindow);
                    m_Document->CloseDoc();
                    m_Document->OpenDoc(fn);
                    FileName = fn;
                    RecentMenu->AddRecentFile(fn);
                    UndoMenu->clearItems();
                }
            }
        }
    }
    void Save() {
        qDebug() << "CFileDocument Save";
        if (FileName.isEmpty()) {
            SaveAs();
            return;
        }
        CSpinLabel l(m_MainWindow);
        m_Document->SaveDoc(FileName);
        UndoMenu->resetDirty();
    }
    void SaveAs() {
        qDebug() << "CFileDocument SaveAs";
        const QString FilePath = QFileDialog::getSaveFileName(m_MainWindow,"Save XML file",m_DocumentPath,"XML files (*.xml)");
        if (!FilePath.isEmpty()) {
            CSpinLabel l(m_MainWindow);
            m_Document->SaveDoc(FilePath);
            FileName = FilePath;
            RecentMenu->AddRecentFile(FilePath);
            UndoMenu->resetDirty();
        }
    }
    void Wizard() {
        qDebug() << "CFileDocument Wizard";
        if (!CleanDoc()) return;
        CSpinLabel l(m_MainWindow);
        m_Document->CloseDoc();
        m_Document->WizardDoc();
        FileName.clear();
        UndoMenu->clearItems();
    }
    bool CleanDoc() {
        qDebug() << "CFileDocument CleanDoc";
        if (UndoMenu->isDirty())
        {
            int r = nativeAlert(m_MainWindow,m_AppName,"Save changes to the current project?",{"Save","No","Cancel"});
            if (r == 1000) Save();
            if (r == 1001) UndoMenu->resetDirty();
        }
        return !UndoMenu->isDirty();
    }
    void NewDoc() { m_Document->NewDoc(); }
    void OpenDoc(QString path) { m_Document->OpenDoc(path); }
    void WizardDoc() { m_Document->WizardDoc(); }
    void CloseDoc() { m_Document->CloseDoc(); }
    QMenu* FileMenu;
    CRecentMenu* RecentMenu;
    CUndoMenu* UndoMenu;
    CEditMenu* EditMenu;

    QAction* actionProjects;
    QAction* actionNew;
    QAction* actionOpen;
    QAction* actionWizard;
    QAction* actionSave;
    QAction* actionSaveAs;

    IFileDocument* m_Document;
    QWidget* m_MainWindow;
    QString m_Organization;
    QString m_AppName;
    QString m_NameFilter;
    QString m_DocumentPath;

    QString FileName;
};
/*
class CFileDocument : public __FileDocumentBaseClass, public IUndoSerializer {
    Q_OBJECT
public:
    CFileDocument(QString Organization,QString AppName,QString DocumentPath, QWidget* parent = nullptr)
        : __FileDocumentBaseClass(parent), m_MainWindow(parent), m_Organization(Organization), m_AppName(AppName), m_DocumentPath(DocumentPath) {
        init();
    }
    void init();
    virtual void OpenDoc(QString) {
        qDebug() << "Virtual opendoc";
    }
    virtual void SaveDoc(QString) {
        qDebug() << "Virtual savedoc";
    }
    virtual void CloseDoc() {
        qDebug() << "Virtual closedoc";
    }
    virtual void NewDoc() {
        qDebug() << "Virtual newdoc";
    }
    virtual void WizardDoc() {
        qDebug() << "Virtual wizarddoc";
    }

    void New();
    void Recent(QString path);
    void Open();
    void Save();
    void SaveAs();
    void Wizard();
    bool CleanDoc();

    QWidget* m_MainWindow;
    CRecentMenu* menuRecent;
    CUndoMenu* UndoMenu;
    QMenu* menuFile;
    QAction* actionProjects;
    QAction* actionNew;
    QAction* actionOpen;
    QAction* actionWizard;
    QAction* actionSave;
    QAction* actionSaveAs;

    QString FileName;
    QString NameFilter;

    QString m_Organization;
    QString m_AppName;
    QString m_DocumentPath;
//signals:
//    void showProjects();
};
*/
class CProjectApp : public QFadingWidget {
    Q_OBJECT
public:
    CProjectApp(QWidget* DocumentWidget, CMainMenu* FileDocument, CProjectPage* ProjectPage, QWidget* parent = nullptr);
    void openInit(QString path);
    void newInit();
    void projects();
    void ensureDocumentView();
    CProjectPage* m_ProjectPage;
    QWidget* m_DocumentWidget;
signals:
    void Changed();
private:
    void openTriggered(QString path);
    void newTriggered();
    void wizardTriggered();
    void setView(bool view);
    bool projectView = true;
    QWidget* m_MainWindow;
    CMainMenu* m_FileDocument;
};

#endif // CPROJECTAPP_H
