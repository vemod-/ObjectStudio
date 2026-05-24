#ifndef CUSTOMJACKS_H
#define CUSTOMJACKS_H

#include "idevice.h"
#include "cdesktopcomponent.h"

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
        //d->addHostJack(J);
        d->ownerList()->addJack(J);
        d->updateHostJacks();
        for (CDesktopComponent* desktop : *desktops) {
            desktop->addInsideJack(J,d,alias);
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
            //d->removeHostJack(J);
            d->ownerList()->disconnectJack(J);
            d->ownerList()->removeJack(J);
            d->updateHostJacks();
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

#endif // CUSTOMJACKS_H
