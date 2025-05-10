#include <QApplication>
//#include <filesystem>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<QItemSelection>("QItemSelection");
    //setlocale(LC_ALL,"");
    //QSettings::setPath(QSettings::NativeFormat, QSettings::SystemScope, "/Users/Shared/Library/Preferences");
    CObjectStudioApplication a(argc, argv);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    a.setAttribute(Qt::AA_DontShowIconsInMenus);
    a.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    a.setWindowIcon(QIcon(":/ocicon.png"));
    a.setApplicationName("ObjectStudio");
    a.setOrganizationName("Veinge Musik och Data");
    a.setOrganizationDomain("http://www.musiker.nu/objectstudio");

    //QFile::copy(":/ocicon.icns", a.applicationDirPath() + "/../Resources/ocicon.icns");
    //QFile::remove(a.applicationDirPath() + "/../Info.plist");
    //QFile::copy(":/Info.plist", a.applicationDirPath() + "/../Info.plist");

    /*
    QSettings s(a.applicationDirPath() + "/../Info.plist",QSettings::NativeFormat);

    s.setValue("LSMultipleInstancesProhibited","True");
    s.setValue("CFBundleIdentifier","com.http-www-musiker-nu-objectstudio.ObjectStudio");
    //s.beginWriteArray("CFBundleDocumentTypes");
    s.beginGroup("CFBundleDocumentTypes");
    s.setValue("CFBundleTypeExtensions", QStringList() << "wav" << "mp3");
    s.setValue("CFBundleTypeRole","Viewer");
    s.endGroup();
    s.setValue("CFBundleIconFile","ocicon");
    s.setValue("NSPrincipalClass","NSApplication");
    s.setValue("NSHighResolutionCapable","True");
*/
    //s.sync();
    a.w.show();

    return a.exec();
}
