#ifndef CEDITMENU_H
#define CEDITMENU_H

#include <QMenu>
#include <QApplication>
#include "qdomlite.h"
#include <QClipboard>

class IEditDocument {
public:
    virtual void CopyDoc(QDomLiteElement* /*xml*/) {}
    virtual void DeleteDoc() {}
    virtual void PasteDoc(const QDomLiteElement* /*xml*/) {}
};

class CEditMenu : public QMenu {
    Q_OBJECT
public:
    CEditMenu(IEditDocument* editDoc, QString appName, QWidget* parent = nullptr)
        : QMenu(parent){
        m_EditDocument = editDoc;
        m_Appname = appName;
        setSelectionStatus(false);
        actionPaste->setEnabled(QApplication::clipboard()->text().startsWith("<"+m_Appname+"CopyData"));
    }
    void setSelectionStatus(bool selected) {
        actionCut->setEnabled(selected);
        actionCopy->setEnabled(selected);
        actionDelete->setEnabled(selected);
    }
    void cutTriggered() {
        copyTriggered();
        deleteTriggered();
    }
    void copyTriggered() {
        QDomLiteElement e(m_Appname+"CopyData");
        m_EditDocument->CopyDoc(&e);
        if (!e.childCount()) return;
        QApplication::clipboard()->setText(e.toString());
        actionPaste->setEnabled(true);
    }
    void deleteTriggered() {
        m_EditDocument->DeleteDoc();
    }
    void pasteTriggered() {
        QDomLiteElement e("CopyData");
        e.fromString(QApplication::clipboard()->text());
        if (e.childCount()) m_EditDocument->PasteDoc(&e);
    }
    QAction* actionCut = addAction("Cut",QKeySequence::Cut,this,&CEditMenu::cutTriggered);
    QAction* actionCopy = addAction("Copy",QKeySequence::Copy,this,&CEditMenu::copyTriggered);
    QAction* actionPaste = addAction("Paste",QKeySequence::Paste,this,&CEditMenu::pasteTriggered);
    QAction* actionDelete = addAction("Delete",QKeySequence(Qt::Key_Backspace),this,&CEditMenu::deleteTriggered);
private:
    IEditDocument* m_EditDocument;
    QString m_Appname;
};

#endif // CEDITMENU_H
