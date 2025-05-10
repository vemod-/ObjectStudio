#ifndef CWAVEDOCUMENT_H
#define CWAVEDOCUMENT_H

//#include "cwavelanes.h"
#include "csoftsynthsform.h"
#include "cprojectapp.h"
#include "cauploader.h"

class CWaveDocument : public IFileDocument {
    //Q_OBJECT
public:
    CWaveDocument(CWaveLanes* Document, QString Organization, QString AppName, QString DocumentPath, QWidget* parent = nullptr)
        : m_MainWindow((CSoftSynthsForm*)parent), m_Document(Document), m_DocumentPath(DocumentPath) {
        MainMenu = new CMainMenu(this,m_MainWindow,Organization,AppName,"XML files (*.xml *.aup)",DocumentPath,parent);
        m_Document->MainMenu = MainMenu;

        MainMenu->actionWizard->setEnabled(false);
        MainMenu->actionWizard->setVisible(false);

        //setVisible(false);
        /*
        QList<QAction*> l;
        l.append(m_Document->Popup->actions());
        for (QAction* a: l) m_Document->Popup->removeAction(a);
        //m_Document->Popup->addActions(MainMenu->UndoMenu->actions());
        //m_Document->Popup->addSeparator();
        m_Document->Popup->addActions(MainMenu->EditActions->actions());
        m_Document->Popup->addSeparator();
        m_Document->Popup->addActions(l);
*/
        //connect(m_Document,&CWaveLanes::aboutToChange,UndoMenu,&CUndoMenu::addItem,Qt::DirectConnection);
        //connect(m_Document,&CWaveLanes::aboutToChangeElement,UndoMenu,&CUndoMenu::addElement,Qt::DirectConnection);
    }
    void CloseDoc() {
        if (m_Document->requestIsPlaying()) m_Document->requestPause();
    }
    void NewDoc() {
        QDomLiteElement e("Custom");
        m_MainWindow->unserializeCustom(&e);
    }
    void OpenDoc(QString path) {
        if (path.endsWith(".aup",Qt::CaseInsensitive)) {
            CAupLoader(m_Document,path);
            m_Document->paint();
            return;
        }
        m_MainWindow->unserializeCustom(CProjectPage::openFile(path).documentElement);
        //FileName=path;
    }
    void SaveDoc(QString path) {
        //QString SavePath=path;
        //if (SavePath.isEmpty()) SavePath=FileName;
        //if (SavePath.isEmpty()) return;
        QFileInfo f(path);
        QString p = m_DocumentPath + f.baseName() + ".zip";
        QDomLiteDocument Doc("ObjectWavesProject","Custom");
        m_MainWindow->serializeCustom(Doc.documentElement);
        CProjectPage::saveFile(p,&Doc, m_MainWindow->grab());
    }
    void undoSerialize(QDomLiteElement* xml) const {
        m_Document->serialize(xml);
    }
    void undoUnserialize(const QDomLiteElement* xml) {
        m_Document->unserialize(xml);
    }
    void CopyDoc(QDomLiteElement* xml) {
        m_Document->CopyDoc(xml);
    }
    void DeleteDoc() {
        m_Document->DeleteDoc();
    }
    void PasteDoc(const QDomLiteElement* xml) {
        m_Document->PasteDoc(xml);
    }
    CSoftSynthsForm* m_MainWindow;
    CWaveLanes* m_Document;
    QString m_DocumentPath;
    CMainMenu* MainMenu;
};

#endif // CWAVEDOCUMENT_H
