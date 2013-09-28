#ifndef SINGLEVSTPLUGINLIST_H
#define SINGLEVSTPLUGINLIST_H

#include <QtCore>
#include <Cocoa/Cocoa.h>

class SingleVSTPlugInList : public QMap<QString,QStringList>
{
public:
    static SingleVSTPlugInList* getInstance()
    {
        static SingleVSTPlugInList    instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return &instance;
    }
private:
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
            if (iterator.fileInfo().isBundle())
            {
                QString filename = iterator.filePath();
                if ((filename.toLower().endsWith(".vst")) | (filename.toLower().endsWith(".vst3")))
                {
                    CFStringRef vstBundlePath =
                            CFStringCreateWithCString(kCFAllocatorDefault,
                                                      filename.toUtf8().constData(), kCFStringEncodingMacRoman );
                    CFURLRef vstBundleURL =
                            CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
                                                          vstBundlePath,
                                                          kCFURLPOSIXPathStyle,
                                                          true);
                    //CFBundleRef vstBundle = CFBundleCreate(kCFAllocatorDefault, vstBundleURL);
                    CFArrayRef archArrayRef = CFBundleCopyExecutableArchitecturesForURL(vstBundleURL);

                    if (archArrayRef)
                    {
                        BOOL isI386 = [(NSArray*)archArrayRef containsObject:[NSNumber numberWithInt:kCFBundleExecutableArchitectureI386]];
                        if (isI386)
                        {
                            qDebug() << filename;
                            l.append(filename);
                        }
                    }
                    CFRelease(vstBundlePath);
                    CFRelease(vstBundleURL);

                    /*
                    if (vstBundle != NULL)
                    {
                        void* main = CFBundleGetFunctionPointerForName(vstBundle, CFSTR("VSTPluginMain"));
                        if (!main) {
                            main = CFBundleGetFunctionPointerForName(vstBundle, CFSTR("main_macho"));
                        }
                        if (main)
                        {
                            qDebug() << filename;
                            l.append(filename);
                        }
                        CFRelease(vstBundle);
                        */
                }
            }
        }
        return l;
    }
};

#endif // SINGLEVSTPLUGINLIST_H
