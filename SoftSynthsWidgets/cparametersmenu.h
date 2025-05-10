#ifndef CPARAMETERSMENU_H
#define CPARAMETERSMENU_H

#include <QAction>
#include <QMenu>
#include <QApplication>
#include <QMessageBox>
#include <QClipboard>
#include <QInputDialog>
#include "idevice.h"
#include "cdevicelist.h"
#include "qsignalmenu.h"
//#include "QtConcurrent/QtConcurrent"

class CPasteParametersAction : public QAction
{
    Q_OBJECT
public:
    CPasteParametersAction(IDevice* device, QWidget* parent)
        : QAction("Paste Parameters",parent)
    {
        m_DeviceList.clear();
        m_DeviceList.append(device);
        connect(this,&CPasteParametersAction::triggered,this,&CPasteParametersAction::PasteParameters);
        setEnabled(QApplication::clipboard()->text().startsWith("<Parameters"));
    }
    CPasteParametersAction(QList<IDevice*>& devices, QWidget* parent)
        : QAction("Paste Parameters",parent)
    {
        m_DeviceList.clear();
        m_DeviceList.append(devices);
        connect(this,&CPasteParametersAction::triggered,this,&CPasteParametersAction::PasteParameters);
        setEnabled(QApplication::clipboard()->text().startsWith("<Parameters"));
    }
private slots:
    void PasteParameters()
    {
        const QString TempClipBoard=QApplication::clipboard()->text();
        if (!TempClipBoard.isEmpty())
        {
            const QDomLiteElement xml = QDomLite::elementFromString(TempClipBoard);
            if (xml.childCount())
            {
                if (xml.matches("Parameters"))
                {
                    const QString ClassName=xml.attribute("Type");
                    if (!ClassName.isEmpty())
                    {
                        for (IDevice* d : std::as_const(m_DeviceList)) {
                            if (ClassName==d->name())
                            {
                                emit aboutToChange("Paste Parameters");
                                d->unserializeDevice(&xml);
                                emit updateControls();
                                emit parametersChanged(d);
                            }
                        }
                    }
                }
            }
        }
    }
signals:
    void updateControls();
    void aboutToChange(const QString&);
    void parametersChanged(IDevice*);
private:
    QList<IDevice*> m_DeviceList;
};

class CParametersMenu : public QMenu
{
    Q_OBJECT
public:
    CParametersMenu(IDevice* device, QWidget* parent, bool automation = true)
        : QMenu("Parameters",parent), m_Device(device)
    {
        ParameterPresetsMenu=new QSignalMenu("Load",this);
        connect(ParameterPresetsMenu,qOverload<QString>(&QSignalMenu::menuClicked),this,&CParametersMenu::LoadPreset);
        addMenu(ParameterPresetsMenu);
        addAction("Save as Preset",this,&CParametersMenu::SavePresetAs);
        addAction("Copy Parameters",this,&CParametersMenu::CopyParameters);
        addAction(pasteParametersAction());
        if (automation) {
            AutomationAction = addAction("Automation",this,&CParametersMenu::Automation);
            AutomationAction->setEnabled(m_Device->parameterCount() > 0);
        }
        setEnabled((m_Device->parameterCount() > 0) || (m_Device->hasUI()));
        fillParameterPresetsMenu();
    }
    static void OpenPreset(IDevice* device, QString PresetName)
    {
        if (const QDomLiteElement* Parameters = CProgramBank::getProgram(PresetName,device->name()))
        {
            device->unserializeDevice(Parameters);
            delete Parameters;
        }
    }
private:
    QAction* pasteParametersAction() {
        CPasteParametersAction* actionPasteParameters = new CPasteParametersAction(m_Device,this);
        connect(actionPasteParameters,&CPasteParametersAction::updateControls,this,&CParametersMenu::updateControls);
        connect(actionPasteParameters,&CPasteParametersAction::aboutToChange,this,&CParametersMenu::aboutToChange,Qt::DirectConnection);
        connect(actionPasteParameters,&CPasteParametersAction::parametersChanged,this,&CParametersMenu::parametersChanged);
        return actionPasteParameters;
    }
private slots:
    void LoadPreset(QString PresetName)
    {
        if (const QDomLiteElement* Parameters = CProgramBank::getProgram(PresetName,m_Device->name()))
        {
            emit aboutToChange("Load Preset");
            m_Device->unserializeDevice(Parameters);
            delete Parameters;
            emit updateControls();
            emit parametersChanged(m_Device);
        }
    }
    void SavePresetAs()
    {
        const QString InputString=QInputDialog::getText(this,"Name","Enter Preset Name",QLineEdit::Normal,"New Preset");
        if (m_Device->programNames().contains(InputString))
        {
            if (QMessageBox::question(this,"Replace Preset?","Replace the Preset "+InputString,QMessageBox::No,QMessageBox::Yes)!=QMessageBox::Yes)
            {
                return;
            }
            //QFuture<void>f1 = QtConcurrent::run(nativeMessage,this->parentWidget(),QString("Audio Buffer"),QString("The Audio Driver was disconnected, trying to start another one!"));
            //f1.waitForFinished();

            //int r = m_Device->showNativeAlert("Replace Preset?","Replace the Preset "+InputString + "?",{"Cancel","Yes"});
            //if (r == 1000) return;
        }
        m_Device->saveCurrentProgram(InputString);
        emit updateControls();
        emit parametersChanged(m_Device);
    }

    void CopyParameters()
    {
        QDomLiteElement xml("Parameters","Type",m_Device->name());
        m_Device->serializeProgram(&xml);
        QApplication::clipboard()->setText(xml.toString());
    }
    void Automation()
    {
        emit showAutomationRequested(m_Device);
    }
signals:
    void parametersChanged(IDevice*);
    void updateControls();
    void showAutomationRequested(IDevice*);
    void aboutToChange(const QString&);
private:
    IDevice* m_Device;
    QAction* AutomationAction;
    QSignalMenu* ParameterPresetsMenu;
    //QAction* actionPasteParameters;
    void fillParameterPresetsMenu() {
        ParameterPresetsMenu->clear();
        QString PresetName;
        if (m_Device != nullptr)
        {
            PresetName=m_Device->currentProgramMatches();
            const QStringList& l=m_Device->programNames();
            for (const QString& s : l)
            {
                QAction* a=ParameterPresetsMenu->addAction(s,s);
                a->setCheckable(true);
                if (s==PresetName) a->setChecked(true);
            }
        }
        if (ParameterPresetsMenu->isEmpty())
        {
            QAction* a=ParameterPresetsMenu->addAction("No Presets");
            a->setEnabled(false);
        }
    }
};

class CConnectionsMenu : public QSignalMenu
{
    Q_OBJECT
public:
    CConnectionsMenu(IJack* jack, CDeviceList* dl, QWidget* parent)
        : QSignalMenu("Connections",parent), m_DL(dl)
    {
        MenuJackID=jack->jackID();
        setAttribute(Qt::WA_DeleteOnClose,true);
        connect(this,qOverload<QString>(&QSignalMenu::menuClicked),this,&CConnectionsMenu::ToggleConnection);
        for (int i=0;i<m_DL->jackCount();i++)
        {
            IJack* J=m_DL->jack(i);
            if (jack->canConnectTo(J))
            {
                QAction* a=addAction(J->jackID(),J->jackID());
                a->setCheckable(true);
                a->setChecked(jack->isConnectedTo(J));
            }
        }
        if (actions().isEmpty())
        {
            QAction* a=addAction("(No Available Connections)");
            a->setEnabled(false);
        }
    }
private slots:
    void ToggleConnection(QString JackID)
    {
        QMutexLocker locker(&mutex);
        emit aboutToChange("Change Connections");
        (m_DL->isConnected(JackID,MenuJackID)) ? m_DL->disconnect(JackID,MenuJackID) :
            m_DL->connect(JackID,MenuJackID);
        emit connectionsChanged();
    }
signals:
    void connectionsChanged();
    void aboutToChange(const QString&);
private:
    CDeviceList* m_DL;
    QString MenuJackID;
    QRecursiveMutex mutex;
};

#endif // CPARAMETERSMENU_H
