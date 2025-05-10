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
    static const QStringList addInNames()
    {
        QStringList l;
        for (const CAddIns::AddInType& AI : std::as_const(getInstance()->AddInList)) l.append(AI.ClassName);
        return l;
    }
    inline static const QString addInName(const int index)
    {
        return getInstance()->AddInList[index].ClassName;
    }
    static int indexOf(const QString& Name)
    {
        return getInstance()->addInNames().indexOf(Name);
    }
    static void registerAddIn(voidinstancefunc f, QString n) {
        AddInType addin;
        addin.ClassName = n;
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
        voidinstancefunc InstanceFunction;
        QLibrary* Instance;
    };
    QVector<AddInType> AddInList;
};

#endif // CADDINS_H
