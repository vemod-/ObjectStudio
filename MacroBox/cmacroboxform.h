#ifndef CMACROBOXFORM_H
#define CMACROBOXFORM_H

#include <QDialog>
#include "cdesktopcontainer.h"
#include <QTreeWidget>

namespace Ui {
    class CMacroBoxForm;
}

class QListWidgetEx : public QListWidget {
    Q_OBJECT
public:
    using QListWidget::QListWidget;

signals:
    void itemsReordered();   // Egen signal

protected:
    void dropEvent(QDropEvent *event) override {
        QListWidget::dropEvent(event);  // låt Qt göra flytten
        emit itemsReordered();          // meddela att ordningen ändrats
    }
};

class QTreeWidgetEx : public QTreeWidget {
    Q_OBJECT
public:
    using QTreeWidget::QTreeWidget;

signals:
    void itemsReordered();   // Egen signal

protected:
    void dropEvent(QDropEvent *event) override
    {
        QTreeWidgetItem *targetItem = itemAt(event->position().toPoint());

        if (!targetItem) {
            event->ignore();
            return;
        }

        // Om man droppar "mellan" items, använd parent
        QTreeWidgetItem *targetParent =
            targetItem->parent() ? targetItem->parent() : targetItem;

        QList<QTreeWidgetItem *> draggedItems = selectedItems();
        if (draggedItems.isEmpty()) {
            event->ignore();
            return;
        }

        QTreeWidgetItem *draggedItem = draggedItems.first();
        QTreeWidgetItem *sourceParent = draggedItem->parent();

        // Tillåt bara om source och target har samma parent
        if (sourceParent && sourceParent == targetParent) {
            QTreeWidget::dropEvent(event);
            emit itemsReordered();          // meddela att ordningen ändrats
        } else {
            event->ignore();
        }
    }
};

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
    const QString defaultCaption() const {
        return DeviceID + " " + ParameterName;
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
    }
    const QString defaultCaption() const {
        if (!additionalParameters.isEmpty()) return additionalParameters.first().defaultCaption();
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
    bool containsAdditional(const QString& deviceID, const QString& parameterName) {
        for (CCustomParameter& p : *this) {
            if (p.containsAdditional(deviceID, parameterName)) return true;
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
        for (const CCustomParameter& c : std::as_const(*this)) {
            l->removeCustomParameter(d,c.masterParameter->Name);
        }
        clear();
        for (QDomLiteElement* p : xml->childElements) {
            push_back(CCustomParameter());
            last().unserialize(p,l,d);
        }
    }
    void serialize(QDomLiteElement* xml) const {
        for (const CCustomParameter& p : *this) {
            QDomLiteElement* xp = xml->appendChild("CustomParameter");
            p.serialize(xp);
        }
    }
};

class CJackCompare {
public:
    CJackCompare(const IJack* j) {
        direction = j->direction;
        attachMode = j->attachMode;
        id = j->jackID();
        name = j->name();
        alias = j->alias();
    }
    CJackCompare(const QDomLiteElement* xml, const IDevice* d) {
        direction = xml->attributeValueInt("Direction");
        attachMode = xml->attributeValueInt("AttachMode");
        name = xml->attribute("Name");
        id = d->deviceID() + " " + name;
        alias = xml->attribute("Alias");
    }
    bool matches(const IJack* j) const {
        if (id == j->jackID()) {
            if (attachMode == j->attachMode) {
                if (direction == j->direction) return true;
            }
        }
        return false;
    }
    bool matches(const QDomLiteElement* xml, const IDevice* d) const {
        if (id == d->deviceID() + " " + xml->attribute("Name")) {
            if (attachMode == xml->attributeValueInt("AttachMode")) {
                if (direction == xml->attributeValueInt("Direction")) return true;
            }
        }
        return false;
    }
    bool matches(const CJackCompare& c) const {
        if (c.id == id) {
            if (c.attachMode == attachMode) {
                if (c.direction == direction) return true;
            }
        }
        return false;
    }
    void serialize(QDomLiteElement* xml) const
    {
        xml->setAttribute("Name",name);
        xml->setAttribute("Alias",alias);
        xml->setAttribute("AttachMode",attachMode);
        xml->setAttribute("Direction",direction);
    }
    int direction;
    int attachMode;
    QString id;
    QString name;
    QString alias;
};

class CJackCompareList : public QList<CJackCompare> {
public:
    CJackCompareList(const QDomLiteElement* xml, const IDevice* d) {
        for (const QDomLiteElement* jack : xml->elementsByTag("Jack")) {
            append(CJackCompare(jack,d));
        }
    }
    CJackCompareList(const IDevice* d) {
        for (int i = 0; i < d->jackCount(); i++) {
            append(CJackCompare(d->jack(i)));
        }
    }
    bool contains(const IJack* j) const {
        for (const CJackCompare& c : *this) {
            if (c.matches(j)) return true;
        }
        return false;
    }
    bool contains(const QDomLiteElement* xml, const IDevice* d) const {
        for (const CJackCompare& c : *this) {
            if (c.matches(xml, d)) return true;
        }
        return false;
    }
    bool contains(const CJackCompare& j) const {
        for (const CJackCompare& c : *this) {
            if (c.matches(j)) return true;
        }
        return false;
    }
    const CJackCompare* item(const CJackCompare& j) const {
        for (const CJackCompare& c : *this) {
            if (c.matches(j)) return &c;
        }
        return nullptr;
    }
    void serialize(QDomLiteElement* xml) const
    {
        for (const CJackCompare& c : *this) {
            c.serialize(xml->appendChild("Jack"));
        }
    }
};

class CCustomJackList {
public:
    void serialize(QDomLiteElement* xml, IDevice* d) const
    {
        const CJackCompareList deviceJacks(d);
        deviceJacks.serialize(xml);
        qDebug() << xml->toString();
    }
    void unserialize(const QDomLiteElement* xml, IDevice* d, QList<CDesktopComponent*>* desktops) {
        QMutexLocker l(&m);
        const CJackCompareList xmlJacks(xml,d);
        CJackCompareList deviceJacks(d);
        for (int i = deviceJacks.size() - 1; i >= 0; i--) {
            if (!xmlJacks.contains(deviceJacks[i])) {
                removeJack(d->jack(i),d,desktops);
                deviceJacks.removeAt(i);
            }
            else {
                const QString alias = xmlJacks.item(deviceJacks[i])->alias;
                d->jack(i)->setAlias(alias);
                for (CDesktopComponent* desktop : std::as_const(*desktops)) {
                    desktop->JacksCreated[i]->setAlias(alias);
                }
            }
        }
        for (const CJackCompare& c : xmlJacks) {
            if (!deviceJacks.contains(c)) addJack(c,d,desktops);
        }
        bool reorder = false;
        for (int i = 0; i < xmlJacks.size(); i++) {
            const QString s = xmlJacks[i].id;
            if (s != d->jackID(i)) {
                reorder = true;
                int idx = d->jackIndex(s);
                if (idx != -1) {
                    d->swapJacks(i,idx);
                    for (CDesktopComponent* desktop : std::as_const(*desktops)) {
                        desktop->JacksCreated.swapItemsAt(i,idx);
                        desktop->InsideJacks.swapItemsAt(i,idx);
                    }
                }
            }
        }
        if (reorder) {
            for (CDesktopComponent* desktop : std::as_const(*desktops)) {
                QList<IJack*> l;
                for (int j = 0; j < d->jackCount(); j++) {
                    l.append(desktop->JacksCreated[j]);
                }
                desktop->reorderJackbarJacks(&l);
            }
            updateProcIndexes(d,desktops);
        }
        d->updateHostJacks();
        for (CDesktopComponent* desktop : std::as_const(*desktops)) {
            desktop->DrawConnections();
            desktop->connectionsChanged();
        }
    }
    void addJack(const CJackCompare& j, IDevice* d, QList<CDesktopComponent*>* desktops) {
        addJack(j.name,j.alias,(IJack::AttachModes)j.attachMode,(IJack::Directions)j.direction,d,desktops);
    }
    void addJack(const QDomLiteElement* xml, IDevice* d, QList<CDesktopComponent*>* desktops) {
        addJack(xml->attribute("Name"),xml->attribute("Alias"),(IJack::AttachModes)xml->attributeValueInt("AttachMode"),(IJack::Directions)xml->attributeValueInt("Direction"),d,desktops);
    }
    void addJack(const QString& name, const QString& alias, const IJack::AttachModes attachMode, const IJack::Directions direction, IDevice* d, QList<CDesktopComponent*>* desktops) {
        IJack* J = d->addJack(name,attachMode,direction);
        J->setAlias(alias);
        d->addHostJack(J);
        for (CDesktopComponent* desktop : *desktops) {
            desktop->addInsideJack(J,d,alias);
            /*
            IJack* J1 = desktop->addJack(J->createInsideJack(d->jackCount() - 1,d),0);
            J1->setAlias(alias);
            desktop->JacksCreated.append(J1);
            (J->isOutJack()) ? desktop->InsideJacks.append(dynamic_cast<CInJack*>(J1)) : desktop->InsideJacks.append(dynamic_cast<CInJack*>(J));
            */
        }
        //qDebug() << created->size() << inside->size() << desktops->size() << d->jack(0)->jackID();
        if (d->outJackCount()) qDebug() << d->outJack(0)->procIndex;
    }
    void removeJack(IJack* J, IDevice* d, QList<CDesktopComponent*>* desktops) {
        QMutexLocker l(&m);
        if (J) {
            for (CDesktopComponent* desktop : *desktops) {
                IJack* I = desktop->deviceList()->jack("This " + J->name());
                desktop->InsideJacks.removeOne(J);
                if (I) {
                    desktop->JacksCreated.removeOne(I);
                    desktop->InsideJacks.removeOne(I);
                    desktop->removeJack(I,0);
                    delete I;
                }
            }
            d->removeJack(J);
            d->removeHostJack(J);
            delete J;
        }
        updateProcIndexes(d,desktops);
    }
private:
    QRecursiveMutex m;
    void updateProcIndexes(IDevice* d, QList<CDesktopComponent*>* desktops) {
        for (int i = 0; i < d->jackCount(); i++) {
            if (d->jack(i)->isOutJack()) ((COutJack*)d->jack(i))->procIndex = i;
            for (CDesktopComponent* desktop : std::as_const(*desktops)) {
                if (desktop->JacksCreated[i]->isOutJack()) static_cast<COutJack*>(desktop->JacksCreated[i])->procIndex = i;
            }
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
    QList<CDesktopComponent*> DesktopComponents;
    QList<CDesktopContainer*> DesktopContainers;
    void unserializeCustom(const QDomLiteElement* xml);
    void serializeCustom(QDomLiteElement* xml) const;
    void fillList(int CurrentProgram=-1);
    void setProgram(const int programIndex);
    void updateDeviceParameter(const CParameter* p);
    bool allowCustomParameters = false;
    bool allowCustomJacks = false;
    //QList<IJack*>* JacksCreated;
    //QList<CInJack*>* InsideJacks;
public slots:
    void PlugInIndexChanged();
private slots:
    void ChangeProgram(int programIndex);
    void cascadeUIs();
    void addParameterMenu(QMenu* m);
    void showParameterDialog();
    void showJackDialog();
    void addCustomParameter(QString id);
    void addCustomParameter(IDevice* d, CParameter* p);
    void addJackMenu(QMenu* m);
    void removeJackMenu(QMenu* m);
    void addCustomJack(QString id);
    void removeCustomJack(QString id);
public slots:
    void removeDeviceParameters(IDevice* device);
    void removeAllParameters();
private:
    Ui::CMacroBoxForm *ui;
    CCustomParameterList m_CustomParameterList;
    CCustomJackList m_CustomJackList;
    void updateParameter(int index, int v);
protected:
    bool event(QEvent *event);
    void timerEvent(QTimerEvent* event);
};

#endif // CMACROBOXFORM_H
