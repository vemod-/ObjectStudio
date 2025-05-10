#ifndef CDESKTOPCONTAINER_H
#define CDESKTOPCONTAINER_H

#include <QWidget>
#include "cdesktopcomponent.h"
#include "../../ObjectComposerXML/qmacsplitter.h"
#include <QMenu>

namespace Ui {
    class CDesktopContainer;
}

class CDesktopContainer : public QWidget
{
    Q_OBJECT

public:
    explicit CDesktopContainer(QWidget *parent = 0);
    ~CDesktopContainer();
    CDesktopComponent* Desktop;
    //QMenu* parametersMenu(IDevice*);
public slots:
    void showMap();
    void hideMap();
    void hideUIs();
    void cascadeUIs(QPoint& p);
    void showParameters(IDevice* d);
    //void showAutomation(IDevice* d);
    void updateControls(IDevice* d);
    //void getParametersMenu(QMenu*, IDevice*);
    //void getPasteParameters(QMenu*, IDevice*);
protected:
    void resizeEvent(QResizeEvent* event);
private:
    Ui::CDesktopContainer *ui;
private slots:
    void resizeContent();
    void addDevice(IDevice* d);
    void removeDevice(IDevice* d);
    void clear();
private:
    QMacSplitter* splitter;
    QRecursiveMutex mutex;
    void duplicateMenu(QMenu* dst, QMenu& origin);
};

#endif // CDESKTOPCONTAINER_H
