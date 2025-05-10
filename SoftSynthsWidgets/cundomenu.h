#ifndef CUNDOMENU_H
#define CUNDOMENU_H

#include "qdomlite.h"
#include <QMenu>

class IUndoSerializer {
public:
    virtual void undoSerialize(QDomLiteElement* /*xml*/) const {}
    virtual void undoUnserialize(const QDomLiteElement* /*xml*/) {}
};

class CUndoMenu : public QMenu {
    Q_OBJECT
public:
    CUndoMenu(IUndoSerializer* doc, QWidget* parent = nullptr, const QIcon& undoIcon = QIcon(), const QIcon& redoIcon = QIcon())
        : QMenu(parent) {
        m_FileDoc = doc;
        actionUndo = addAction("Undo");
        actionUndo->setShortcut(QKeySequence::Undo);
        actionUndo->setIcon(undoIcon);
        actionUndo->setEnabled(false);
        connect(actionUndo,&QAction::triggered,this,&CUndoMenu::undo,Qt::DirectConnection);
        actionRedo = addAction("Redo");
        actionRedo->setIcon(redoIcon);
        actionRedo->setShortcut(QKeySequence::Redo);
        actionRedo->setEnabled(false);
        connect(actionRedo,&QAction::triggered,this,&CUndoMenu::redo,Qt::DirectConnection);
    }
    ~CUndoMenu() {
        clearItems();
    }
    QAction* actionUndo;
    QAction* actionRedo;
    QDomLiteElementList undoItems;
    int undoIndex = -1;
    bool dirty = false;
    bool isDirty() {
        return dirty;
    }
    void resetDirty() {
        dirty = false;
    }
    bool canUndo() {
        return (undoIndex > -1);
    }
    bool canRedo() {
        return (undoItems.size() > undoIndex+1);
    }
    void updateActions() {
        actionUndo->setEnabled(undoIndex > -1);
        if (actionUndo->isEnabled()) {
            if (undoItems[undoIndex])
            actionUndo->setText("Undo " + undoItems[undoIndex]->text.string());
        }
        else {
            actionUndo->setText("Undo");
        }
        actionRedo->setEnabled(undoItems.size() > undoIndex+1);
        if (actionRedo->isEnabled()) {
            actionRedo->setText("Redo " + undoItems[undoIndex+1]->text.string());
        }
        else {
            actionRedo->setText("Redo");
        }
    }
    void clearItems() {
        if (!undoItems.empty()) {
            qDeleteAll(undoItems);
            undoItems.clear();
        }
        undoIndex=-1;
        dirty = false;
        updateActions();
    }
    void addItem(const QString& caption) {
        if (caption.length()==0) return;
        QDomLiteElement s("UndoItem");
        m_FileDoc->undoSerialize(&s);
        addElement(&s, caption);
    }
    void addElement(QDomLiteElement* s, const QString& caption) {
        s->text = caption;
        while (undoItems.size() > undoIndex+1)
        {
            delete undoItems.takeLast();
        }
        undoItems.append(s->clone());
        undoIndex++;
        while (undoItems.size() > 20)
        {
            delete undoItems.takeFirst();
            undoIndex--;
        }
        dirty = true;
        updateActions();
    }
    void undo() {
        QDomLiteElement* e=undoItems[undoIndex];
        QDomLiteElement* t = new QDomLiteElement(e->tag);
        t->text = e->text;
        m_FileDoc->undoSerialize(t);
        m_FileDoc->undoUnserialize(e);
        undoItems.replace(undoIndex,t);
        delete e;
        undoIndex--;
        updateActions();
    }
    void redo() {
        QDomLiteElement* e = undoItems[undoIndex+1];
        QDomLiteElement* t = new QDomLiteElement(e->tag);
        t->text = e->text;
        m_FileDoc->undoSerialize(t);
        m_FileDoc->undoUnserialize(e);
        undoItems.replace(undoIndex+1,t);
        delete e;
        undoIndex++;
        updateActions();
    }
private:
    IUndoSerializer* m_FileDoc;
};

#endif // CUNDOMENU_H
