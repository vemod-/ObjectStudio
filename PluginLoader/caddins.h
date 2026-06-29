#ifndef CADDINS_H
#define CADDINS_H

#include <QStringList>
#include <QDir>
//#include <QtCore>
#include <QLibrary>

typedef void*(*voidinstancefunc)();

class CAddIns
{
public:
    inline static voidinstancefunc addInInstanceFunction(const int index)
    {
        return getInstance()->AddInList[index].InstanceFunction;
    }
    inline static voidinstancefunc addInInstanceFunction(const QString& Name)
    {
        return addInInstanceFunction(indexOf(Name));
    }
    static const QStringList addInNames(const int category = 0, const QString& filter = QString())
    {
        QStringList l;
        if (category == 0) {
            for (const CAddIns::AddInType& AI : std::as_const(getInstance()->AddInList)) l.append(AI.ClassName);
        }
        else if (category == 127) {
            for (const CAddIns::AddInType& AI : std::as_const(getInstance()->AddInList)) {
                if (AI.Category == category) l.append(AI.ClassName);
            }
        }
        else {
            for (const CAddIns::AddInType& AI : std::as_const(getInstance()->AddInList)) {
                if (AI.Category != 127) {
                    if (AI.Category & category) l.append(AI.ClassName);
                }
            }
        }
        if (filter.isEmpty()) return l;
        const QStringList f = filter.split(",");
        for (const QString& n : std::as_const(l)) {
            const int i = indexOf(n);
            const QString s = getInstance()->AddInList[i].Jacks;
            for (const QString& j : f) {
                if (!s.contains(j)) {
                    l.removeOne(n);
                    break;
                }
            }
        }
        return l;
    }
    inline static const QString addInName(const int index)
    {
        return getInstance()->AddInList[index].ClassName;
    }
    inline static const QStringList jacks(const int index) {
        return getInstance()->AddInList[index].Jacks.split(",");
    }
    static int indexOf(const QString& Name)
    {
        return getInstance()->addInNames().indexOf(Name);
    }
    static int addInCategory(const QString& name) {
        const int i = indexOf(name);
        if (i < 0) return 0;
        return getInstance()->AddInList[i].Category;
    }
    static void registerAddIn(voidinstancefunc f, QString n, int c, QString j = QString()) {
        AddInType addin;
        addin.ClassName = n;
        addin.Jacks = j;
        addin.Category = c;
        addin.InstanceFunction = f;
        getInstance()->AddInList.append(addin);
    }
private:
    CAddIns();
    ~CAddIns();
    CAddIns(CAddIns const&);              // Don't Implement
    void operator=(CAddIns const&); // Don't implement
    inline static CAddIns* getInstance() {
        static CAddIns instance;
        return &instance;
    }
    //QVector<AddInType> AddInList;
    void LoadAddIns(const QDir& pluginsDir);
    struct AddInType
    {
        QString Path;
        QString ClassName;
        QString Jacks;
        int Category;
        voidinstancefunc InstanceFunction;
        QLibrary* Instance;
    };
    QVector<AddInType> AddInList;
};

#endif // CADDINS_H
