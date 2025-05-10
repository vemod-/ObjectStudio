#ifndef SINGLEVSTPLUGINLIST_H
#define SINGLEVSTPLUGINLIST_H

#include <QtCore>
#include <QProcess>
//#include <Cocoa/Cocoa.h>
#include "macbundles.h"

#ifndef Q_OS_IOS
void runAppleScript(const QString& script)
{
    QProcess::execute("osascript", {"-e", script});
}
#endif

class SingleVSTPlugInList : public QMap<QString,QStringList>
{
public:
    static const QStringList categories()
    {
        return getInstance()->keys();
    }
    static const QStringList files(const QString& category)
    {
        return getInstance()->value(category);
    }
private:
    static SingleVSTPlugInList* getInstance()
    {
        static SingleVSTPlugInList    instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return &instance;
    }
    SingleVSTPlugInList()
    {
        if (isEmpty())
        {
            QStringList l=StringsFromDir(QDir("/Library/Audio/Plug-Ins/VST"));
            if (!l.isEmpty()) insert("VST",l);
            l=StringsFromDir(QDir("/Library/Audio/Plug-Ins/VST3"));
            if (!l.isEmpty()) insert("VST3",l);
            l=StringsFromDir(QDir(QDir::homePath()+"/Library/Audio/Plug-Ins/VST/PluginsBridgedFor32BitVSTHosts"));
            if (!l.isEmpty()) insert("JBridged",l);
        }
    }                   // Constructor? (the {} brackets) are needed here.
    SingleVSTPlugInList(SingleVSTPlugInList const&);              // Don't Implement
    void operator=(SingleVSTPlugInList const&); // Don't implement
    QStringList StringsFromDir(QDir dir)
    {
        QStringList l;
        QDirIterator iterator(dir.absolutePath(), QDirIterator::Subdirectories);
        while (iterator.hasNext())
        {
            iterator.next();
            qDebug() << iterator.fileName();
            if (iterator.fileInfo().isBundle())
            {
                qDebug() << iterator.fileName() << "isBundle";
                QString filename = iterator.filePath();
                if ((filename.endsWith(".vst",Qt::CaseInsensitive)) || (filename.endsWith(".vst3",Qt::CaseInsensitive)))
                {
                    qDebug() << iterator.fileName() << "ends with vst";
                    const CFBundleRef TempBundle = pathToCFBundleRef(filename);
                    qDebug() << iterator.fileName() << "path to bundle" << TempBundle;
                    try {
                        if ((functionPointerInBundle("VSTPluginMain",TempBundle)) || (functionPointerInBundle("main_macho",TempBundle))  || (functionPointerInBundle("main",TempBundle))  || (functionPointerInBundle("main_plugin",TempBundle)))
                        {
                            qDebug() << iterator.fileName() << "has pointer";
    #ifndef __x86_64
                            if (bundleIsI386(filename))
                            {
                                qDebug() << "x86" << filename;
                                l.append(filename);
                            }
                            else if (bundleIsX86_64(filename))
                            {
                                qDebug() << "x86_64" << filename;
                            }
                            else
                            {
                                qDebug() << "Unknown" << filename;
                            }
    #else
                            if (bundleIsX86_64(filename))
                            {
                                qDebug() << "x86_64" << filename;
                                l.append(filename);
                            }
                            else if (bundleIsI386(filename))
                            {
                                qDebug() << "x86" << filename;
                                moveFile(filename);
                            }
                            else
                            {
                                qDebug() << "Unknown" << filename;
                                moveFile(filename);
                            }
#endif
                        }
                        else
                        {
                            qDebug() << "No function" << filename;
                            moveFile(filename);
                        }
                        CFRelease(TempBundle);
                    }
                    catch(...) {
                        qDebug() << iterator.fileName() << "catch error!";
                    }
                }
            }
        }
        return l;
    }
    void moveFile(const QString& filename)
    {
        QString p = QString(filename).replace("/Library/Audio/Plug-Ins/","/Library/Audio/Plug-Ins/Incopatible/");
        if (!p.startsWith(QDir::homePath())) p = QDir::homePath()+p;
        QString d = QFileInfo(p).absoluteDir().absolutePath();
        if (!QDir(d).exists()) qDebug() << "Create path" << d << QDir(d).mkpath(d);
#ifdef Q_OS_IOS
        QFile(filename).rename(p);
#else
        runAppleScript("tell application \"Finder\"\n"
                      "move POSIX file \""+filename+"\" to POSIX file \""+d+"\" with replacing \n"
                      "end tell\n");
#endif
    }
};

#endif // SINGLEVSTPLUGINLIST_H
