#ifndef CRECENTMENU_H
#define CRECENTMENU_H

#include "qsignalmenu.h"
#include <QSettings>
#include <QFileInfo>

class CRecentMenu : public QSignalMenu {
    Q_OBJECT
public:
    CRecentMenu(const QString& Organization, const QString& AppName, QWidget* parent = nullptr) :
        QSignalMenu("Recent",parent) {
        m_Organization = Organization;
        m_AppName = AppName;
        connect(this,SIGNAL(menuClicked(QString)),this,SIGNAL(RecentTriggered(QString)));
        QSettings s(m_Organization,m_AppName);
        m_RecentDocuments = s.value("RecentFiles").toStringList();
        FillMenu(m_RecentDocuments);
        s.setValue("RecentFiles",m_RecentDocuments);
    }
    void AddRecentFile(const QString path) {
        if ((path.startsWith(":/") || (!QFileInfo::exists(path)))) return;
        QSettings s(m_Organization,m_AppName);
        m_RecentDocuments = s.value("RecentFiles").toStringList();
        m_RecentDocuments.removeOne(path);
        m_RecentDocuments.prepend(path);
        while (m_RecentDocuments.size() > 20) m_RecentDocuments.removeLast();
        clear();
        FillMenu(m_RecentDocuments);
        s.setValue("RecentFiles",m_RecentDocuments);
    }
    QStringList recentDocuments() {
        return m_RecentDocuments;
    }
signals:
    void RecentTriggered(QString path);
private:
    QString m_Organization;
    QString m_AppName;
    QStringList m_RecentDocuments;
    void FillMenu(QStringList& l) {
        for (const QString& f : l)
        {
            if ((f.startsWith(":/") || (!QFileInfo::exists(f)))) {
                l.removeOne(f);
            }
            else {
                const QString DisplayString = (f.length()>60) ? QStringLiteral("...")+f.right(57) : f;
                QSignalMenu::addAction(DisplayString,f);
            }
        }
    }
};

#endif // CRECENTMENU_H
