#ifndef CAUDIOPLUGINHOST_H
#define CAUDIOPLUGINHOST_H

#include "cdevicecontainer.h"
#include "qsignalmenu.h"

class CMenuWidget : public QWidget
{
    Q_OBJECT
public:
    CMenuWidget(CDeviceContainer* parent) : QWidget(nullptr)
    {
        currentEffect = parent;
        effectMenu=new QSignalMenu(this);
        showUIAction=effectMenu->addAction("Show UI","Show UI");
        for (const QString& s:CDeviceContainer::effectList()) effectMenu->addAction(s,s);
        unloadAction=effectMenu->addAction("Unload","Unload");
        connect(effectMenu,SIGNAL(menuClicked(QString)),this,SLOT(selectEffect(QString)));
    }
    void effectMenuPopup()
    {
        showUIAction->setVisible(!currentEffect->deviceType().isEmpty());
        unloadAction->setVisible(!currentEffect->deviceType().isEmpty());
        effectMenu->popup(QCursor::pos());
    }
    QSignalMenu* effectMenu;
    QAction* showUIAction;
    QAction* unloadAction;
private slots:
    void selectEffect(QString DeviceType)
    {
        if (DeviceType=="Show UI")
        {
            currentEffect->execute(true);
            currentEffect->raiseForm();
            return;
        }
        if (currentEffect->deviceType()!=DeviceType)
        {
            currentEffect->setDeviceType(DeviceType);
        }
        currentEffect->execute(true);
    }
    private:
    CDeviceContainer* currentEffect;
};

class CAudioPlugInHost : public CDeviceContainer
{
public:
    CAudioPlugInHost();
    void execute(const bool Show) override
    {
        if (!m_Device)
        {
            effectMenuPopup();
        }
        else
        {
            CDeviceContainer::execute(Show);
        }
    }
    void effectMenuPopup()
    {
        menuWidget->effectMenuPopup();
    }
private:
    CMenuWidget* menuWidget;
};

#endif // CAUDIOPLUGINHOST_H
