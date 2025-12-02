#ifndef CMACROBOXFORM_H
#define CMACROBOXFORM_H

#include <QDialog>
#include "cdesktopcontainer.h"

namespace Ui {
    class CMacroBoxForm;
}

class CParameterID {
public:
    QString DeviceID;
    QString ParameterName;
    CParameterID(const QString& deviceID, const QString& parameterName) {
        DeviceID = deviceID;
        ParameterName = parameterName;
    }
    CParameterID(const QString& deviceID, CParameter* p) : CParameterID(deviceID, p->Name) {}
    CParameterID(const QString& id) {
        const QStringList s = id.split("&&&&&");
        DeviceID = s[0];
        ParameterName = s[1];
    }
    static const QString parameterID(const QString& deviceID, const QString& parameterName) {
        return deviceID + "&&&&&" + parameterName;
    }
    static const QString parameterID(const QString& deviceID, CParameter* p) {
        return parameterID(deviceID, p->Name);
    }
    const QString parameterID() const {
        return parameterID(DeviceID, ParameterName);
    }
    CParameter* parameter(CDeviceList* l) const {
        return l->device(DeviceID)->parameter(ParameterName);
    }
};

class CCustomParameter {
public:
    CCustomParameter() {}
    CCustomParameter(const QString& deviceID, CParameter* p) {
        additionalParameters.append(CParameterID(deviceID,p));
    }
    bool containsAdditional(const QString& deviceID, const QString& parameterName) {
        const QString id = CParameterID::parameterID(deviceID,parameterName);
        for (CParameterID& ap : additionalParameters) {
            if (id == ap.parameterID()) return true;
        }
        return false;
    }
    bool containsAdditional(const QString& deviceID, CParameter* p) {
        return containsAdditional(deviceID, p->Name);
    }
    bool containsAdditional(CDeviceList* l, CParameter* p) {
        for (CParameterID& ap : additionalParameters) {
            if (p == ap.parameter(l)) return true;
        }
        return false;
    }
    CParameter* parameter(CDeviceList* l, const QString& deviceID, const QString& parameterName) const {
        const QString id = CParameterID::parameterID(deviceID,parameterName);
        for (const CParameterID& ap : additionalParameters) {
            if (id == ap.parameterID()) return ap.parameter(l);
        }
        return nullptr;
    }
    void removeDevice(const QString& deviceID) {
        for (int i = additionalParameters.size() - 1; i >= 0; i--) {
            if (additionalParameters[i].DeviceID == deviceID) additionalParameters.removeAt(i);
        }
    }
    void unserialize(const QDomLiteElement* xml, CDeviceList* l, IDevice* d) {
        qDebug() << xml->toString();
        masterParameter = l->addCustomParameter(d,(CParameter::ParameterTypes)xml->attributeValueInt("Type"),xml->attribute("Name"),xml->attribute("Unit"),xml->attributeValueInt("Min"),xml->attributeValueInt("Max"),xml->attributeValueInt("DecimalFactor"),xml->attribute("ListString"),xml->attributeValueInt("Value"));
        for (QDomLiteElement* p : xml->elementsByTag("Parameter")) {
            additionalParameters.append(CParameterID(p->attribute("ParameterID")));
        }
        for (const QDomLiteElement* e : xml->elementsByTag("AutomationEvent")) {
            masterParameter->events.push_back(CParameterEvent(e));
        }
    }
    void serialize(QDomLiteElement* xml) const {
        for (const CParameterEvent& e : masterParameter->events) {
            e.serialize(xml->appendChild("AutomationEvent"));
        }
        xml->setAttribute("Type",masterParameter->Type);
        xml->setAttribute("Name",masterParameter->Name);
        xml->setAttribute("Unit",masterParameter->Unit);
        xml->setAttribute("Min",masterParameter->Min);
        xml->setAttribute("Max",masterParameter->Max);
        xml->setAttribute("DecimalFactor",masterParameter->DecimalFactor);
        xml->setAttribute("ListString",masterParameter->List);
        xml->setAttribute("Value",masterParameter->Value);
        for (const CParameterID& p : additionalParameters) {
            xml->appendChild("Parameter","ParameterID",p.parameterID());
        }
        qDebug() << xml->toString();
    }
    QString Caption() {
        if (masterParameter) return masterParameter->Name;
        return QString();
    }
    QList<CParameterID> additionalParameters;
    CParameter* masterParameter = nullptr;
};

class CCustomParameterList : public QList<CCustomParameter> {
public:
    CCustomParameter* append() {
        push_back(CCustomParameter());
        return &last();
    }
    CCustomParameter* append(const QString& deviceID, CParameter* p) {
        push_back(CCustomParameter(deviceID,p));
        return &last();
    }
    void removeDevice(const QString& deviceID) {
        for (CCustomParameter& p : *this) {
            p.removeDevice(deviceID);
        }
    }
    bool containsAdditional(CDeviceList* l, CParameter* parameter) {
        for (CCustomParameter& p : *this) {
            if (p.containsAdditional(l, parameter)) return true;
        }
        return false;
    }
    void setValue(CDeviceList* l, const int index, const int value) {
        if (index < size()) {
            for (CParameterID& p : (*this)[index].additionalParameters) {
                p.parameter(l)->setValue(value);
            }
        }
    }
    void unserialize(const QDomLiteElement* xml, CDeviceList* l, IDevice* d) {
        for (QDomLiteElement* p : xml->childElements) {
            QList<CCustomParameter>::append(CCustomParameter());
            (*this).last().unserialize(p,l,d);
        }
    }
    void serialize(QDomLiteElement* xml) const {
        for (const CCustomParameter& p : *this) {
            QDomLiteElement* xp = xml->appendChild("CustomParameter");
            p.serialize(xp);
        }
    }
};

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
    void updateDeviceParameter(const CParameter* p);
    bool allowCustomParameters = false;
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
    void addParameterMenu(QMenu* m);
    void showParameterDialog();
    void toggleCustomParameter(QString id);
    void addCustomParameter(IDevice* d, CParameter* p);
    //void removeCustomParameter(CParameter* p);
    void removeDeviceParameters(IDevice* device);
    void removeAllParameters();
private:
    Ui::CMacroBoxForm *ui;
    //QStringList parameterIDs;
    CCustomParameterList m_CustomParameterList;
    void updateParameter(int index, int v);
    //QRecursiveMutex mutex;
    //QMenu* parametersMenu;
    //QAction* actionPasteParameters;
protected:
    bool event(QEvent *event);
    void timerEvent(QTimerEvent* event);
};

#endif // CMACROBOXFORM_H
