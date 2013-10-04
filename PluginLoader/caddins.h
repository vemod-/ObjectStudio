#ifndef CADDINS_H
#define CADDINS_H

#include <QtCore>

typedef void*(*voidinstancefunc)();

class CAddIns
{
public:
    struct AddInType
    {
        QString Path;
        QString ClassName;
        voidinstancefunc InstanceFunction;
        void* Instance;
    };
    CAddIns();
    ~CAddIns();
    static QVector<AddInType> AddInList;
    static const QStringList AddInNames();
    static unsigned int AddInIndex(const QString& Name);
    static int instances;
private:
    void LoadAddIns(QDir& pluginsDir);
};

#endif // CADDINS_H
