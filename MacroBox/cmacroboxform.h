#ifndef CMACROBOXFORM_H
#define CMACROBOXFORM_H

#include <QDialog>
#include "cdesktopcontainer.h"

namespace Ui {
    class CMacroBoxForm;
}

class CMacroBoxForm : public CSoftSynthsForm
{
    Q_OBJECT

public:
    explicit CMacroBoxForm(IDevice* Device, QWidget *parent = 0);
    virtual ~CMacroBoxForm();
    CDesktopComponent* DesktopComponent;
    CDesktopContainer* DesktopContainer;
    void unserializeCustom(const QDomLiteElement* xml);
    void serializeCustom(QDomLiteElement* xml) const;
    void fillList(int CurrentProgram=-1);
    void setProgram(const int programIndex);
public slots:
    void PlugInIndexChanged();
private slots:
    void ChangeProgram(int programIndex);
    //void saveAsPreset();
    //void copyParameters();
    //void pasteParameters();
    //void showMap();
    //void hideUIs();
    void cascadeUIs();
private:
    Ui::CMacroBoxForm *ui;
    //QRecursiveMutex mutex;
    //QMenu* parametersMenu;
    //QAction* actionPasteParameters;
protected:
    bool event(QEvent *event);
    void timerEvent(QTimerEvent* event);
};

#endif // CMACROBOXFORM_H
