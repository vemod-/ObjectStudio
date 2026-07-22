#ifndef CPARAMETERSCOMPONENT_H
#define CPARAMETERSCOMPONENT_H

#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include "cknobcontrol.h"
#include "../../LCDLabel/qlcdlabel.h"
#include "../../EffectLabel/effectlabel.h"
#include "idevice.h"
#include "qgraphicsitemlist.h"

#define rackUnitHeight 112

class CParametersComponent : public QObject
{
    Q_OBJECT
public:
    explicit CParametersComponent(QGraphicsScene* s);
    ~CParametersComponent();
    QString deviceID();
    void init(IDevice* Device);
    void showParameters(int index);
    bool swallowMousePress(QMouseEvent *event, QGraphicsItem* item, QWidget* parent);
    bool itemIsKnob(QGraphicsItem* item);
private:
    QList<CKnobControl*> Dials;
    QList<CParameter*> Parameters;
    IDevice* m_Device;
    //int m_Width = 0;
    //int m_Index = 0;
    EffectLabel* m_NameLabel;
    QLCDLabel* m_UILabel;
    QLCDLabel* m_IDLabel;
    QLCDLabel* m_PresetLabel;
    QGraphicsContainerItem m_ProxyDials;
    QGraphicsProxyWidget* m_ProxyNameLabel = nullptr;
    QGraphicsProxyWidget* m_ProxyUILabel = nullptr;
    QGraphicsContainerItem m_GroupList;
    QGraphicsContainerItem m_FrameList;
    QGraphicsPathItem m_CategoryFrame;
    QMenu* parametersMenu(QWidget* parent);
    //QRecursiveMutex mutex;
    QGraphicsProxyWidget* createProxyItem(QWidget* w);
    /*
    int calcY(int y) {
        return (m_Index * rackUnitHeight) + y;
    }
*/
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
public slots:
    void updateControls();
    void updateControl(const CParameter* Parameter);
signals:
    void popupTriggered(IDevice* Device, QPoint Pos);
    void parametersChanged(IDevice*);
    void aboutToChange(const QString&);
    void showAutomationRequested(IDevice*,int);
};

#endif // CPARAMETERSCOMPONENT_H
