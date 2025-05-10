#include "caddins.h"
#include <QDebug>
#include <QCoreApplication>

CAddIns::CAddIns()
{
    if (AddInList.isEmpty())
    {

        qDebug() << "PluginLoader created";
#ifndef BUILD_WITH_STATIC
#ifdef Q_OS_IOS
        QDir pluginsDir = QCoreApplication::applicationDirPath() + "/Frameworks";
#else
        QDir pluginsDir = QCoreApplication::applicationDirPath();
        //pluginsDir.cdUp();
        //pluginsDir.cdUp();
#endif
        qDebug() << pluginsDir;
        for (int i = 0; i < 5; i++)
        {
            if (!pluginsDir.exists()) break;
            LoadAddIns(pluginsDir);
            if (!AddInList.empty()) break;
            pluginsDir.cdUp();
        }
        /*
        QDir pluginsDir(getenv("DYLD_FALLBACK_LIBRARY_PATH"));
        LoadAddIns(pluginsDir);
        if (AddInList.empty())
        {
            pluginsDir = QCoreApplication::applicationDirPath();
            while (pluginsDir.exists())
            {
                LoadAddIns(pluginsDir);
                if (!AddInList.empty()) break;
                pluginsDir.cdUp();
            }
        }
        */
#endif
    }
}

CAddIns::~CAddIns()
{
    for (AddInType& AI : AddInList)
    {
        AI.Instance->unload();
        delete AI.Instance;
    }
    qDebug() << "PluginLoader destroyed";
}

void CAddIns::LoadAddIns(const QDir& pluginsDir)
{
    for (const QString& fileName : (const QStringList)pluginsDir.entryList(QStringList() << "*.dylib",QDir::Files | QDir::Hidden | QDir::NoSymLinks))
    {
        QString filepath=pluginsDir.absolutePath()+"/"+fileName;
        qDebug() << filepath;
        auto lib = new QLibrary(filepath);
        qDebug() << lib->fileName() << lib->errorString();
        lib->load();
        qDebug() << lib->isLoaded() << lib->errorString();
        if (lib->isLoaded())
        {
            auto initializer = reinterpret_cast<void*>(lib->resolve("createinstance"));
            auto name = reinterpret_cast<void*>(lib->resolve("name"));
            if ((initializer != nullptr) && (name != nullptr))
            {
                AddInType addin;
                addin.ClassName = *static_cast<char**>(name);
                addin.Instance=lib;
                addin.InstanceFunction = *static_cast<voidinstancefunc*>(initializer);
                addin.Path = filepath;
                AddInList.push_back(addin);
                qDebug() << "success" << addin.ClassName << addin.Path;
            }
            else
            {
                lib->unload();
                delete lib;
            }
        }
        else
        {
            delete lib;
        }
    }
    for (const QString& dirName : (const QStringList)pluginsDir.entryList(QDir::AllDirs | QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot))
    {
        const QDir dir(pluginsDir.absolutePath()+"/"+dirName);
        if (dir.exists()) LoadAddIns(dir);
    }
}
