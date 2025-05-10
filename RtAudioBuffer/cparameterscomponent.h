#ifndef CPARAMETERSCOMPONENT_H
#define CPARAMETERSCOMPONENT_H

#include <QWidget>
#include "cknobcontrol.h"
#include <QLabel>
#include "idevice.h"

namespace Ui {
    class CParametersComponent;
}

class CParametersComponent : public QWidget
{
    Q_OBJECT

public:
    explicit CParametersComponent(QWidget *parent = nullptr);
    ~CParametersComponent();
    QString deviceID();
    void init(IDevice* Device);
    void showParameters();
protected:
    void wheelEvent(QWheelEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent* event);
    bool eventFilter(QObject* obj, QEvent* event);
private:
    Ui::CParametersComponent *ui;
    QList<CKnobControl*> Dials;
    QList<CParameter*> Parameters;
    QWidget* Spacer;
    IDevice* m_Device;
    int m_Width;
    /*
    QMenu* ParametersMenu;
    QAction* AutomationAction;
    QSignalMenu* ParameterPresetsMenu;
    QAction* actionPasteParameters;
    void fillParameterPresetsMenu();
*/
    //QAction* pasteParameters();
    QMenu* parametersMenu();
    QRecursiveMutex mutex;
private slots:
    void updateParameterValue(int i);
    void showAutomation(CParameter* p) {
        for (int i = 0; i < m_Device->parameterCount(); i++) {
            if (p == m_Device->parameter(i)) {
                emit showAutomationRequested(m_Device,i);
                return;
            }
        }
    }
    void showDefaultAutomation(IDevice* d) {
        emit showAutomationRequested(d,0);
    }
     /*
    void OpenPreset(QString PresetName);
    void SavePresetAs();
    void CopyParameters();
    void PasteParameters();
    void Automation();
*/
public slots:
    void updateControls();
    void updateControl(const CParameter* Parameter);
signals:
    void popupTriggered(IDevice* Device, QPoint Pos);
    void mousePress(IDevice*, QPoint);
    void parametersChanged(IDevice*);
    void aboutToChange(const QString&);
    void showAutomationRequested(IDevice*,int);
};

#endif // CPARAMETERSCOMPONENT_H
