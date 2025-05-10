#ifndef CFILEPARAMETER_H
#define CFILEPARAMETER_H

#include "cfileidentifier.h"
#include "cpresets.h"
#include <QDomLite>
#include <QFileDialog>
#include "idevicebase.h"

class IFileLoader
{
public:
    virtual ~IFileLoader();
    virtual bool loadFile(const QString& /*path*/) { return false; }
};

class CFileParameter
{
public:
    CFileParameter() : m_OwnerDevice(nullptr), m_Loader(nullptr), m_Resolve(true) {}
    CFileParameter(IDeviceBase* owner, IFileLoader* loader) : CFileParameter() {
        m_OwnerDevice = owner;
        m_Loader = loader;
    }
    CFileParameter(IDeviceBase* owner) : CFileParameter(owner,dynamic_cast<IFileLoader*>(owner)) {}
    CFileParameter(IDeviceBase* owner, IFileLoader* loader, const QString path) : CFileParameter(owner,loader) {
        setFilename(path);
    }
    virtual ~CFileParameter();
    virtual const QString filename() const { return m_FileName.path; }
    virtual CFileIdentifier fileID() const { return m_FileName; }
    virtual void setFilename(const QString& filename) {
        m_FileName = filename;
        //qDebug() << "setFilename" << filename << m_FileName.path << m_FileName.lastModified << m_FileName.info.absolutePath() << m_FileName.path << m_BasePath;
    }
    virtual void clear() { setFilename(QString()); }
    bool fileIsValid(const QString& path) const {
        if (path.isEmpty()) return false;
        CFileIdentifier id(path);
        return (!id.info.exists()) ? false : (fileID() != id);
    }
    virtual bool openFile(const QString& filename) {
        if (m_Loader)
        {
            QMutexLocker locker(&mutex);
            QString fn = CPresets::resolveFilename(filename);
            if (fileIsValid(fn))
            {
                if (m_Loader->loadFile(fn))
                {
                    setFilename(fn);
                    if (m_OwnerDevice) m_OwnerDevice->updateParameter();
                    return true;
                }
                else
                {
                    clear();
                    if (m_OwnerDevice) m_OwnerDevice->updateParameter();
                }
            }
        }
        return false;
    }
    virtual const QString selectFile(const QString& Filter) {
        QMutexLocker locker(&mutex);
        QFileDialog d; //m_MainWindow
        d.setFileMode(QFileDialog::ExistingFile);
        d.setNameFilter(Filter);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        d.setDirectory(QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
#endif
        if (!filename().isEmpty()) d.selectFile(filename());
        if (d.exec()!=QDialog::Accepted) return QString();
        return d.selectedFiles().first();
    }
    virtual void unserialize(const QDomLiteElement* xml) {
        if (!xml) return;
        if (!xml->attributeExists("File")) return;
        QMutexLocker locker(&mutex);
        QString fn = xml->attribute("File");
        //qDebug() << "Fileparameter open 1" << fn;
        if (m_Resolve) if (fn.isEmpty()) return;
        if (!m_BasePath.isEmpty()) fn = QDir(m_BasePath).absoluteFilePath(fn);
        //qDebug() << "Fileparameter open 2" << fn;
        openFile(fn);
    }
    virtual void serialize(QDomLiteElement* xml) const {
        QString fn = filename();
        //qDebug() << "Fileparameter save 1" << fn;
        if (!m_BasePath.isEmpty()) fn = QDir(m_BasePath).relativeFilePath(filename());
        //qDebug() << "Fileparameter save 2" << fn;
        xml->setAttribute("File",fn);
    }
    void setResolve(const bool r) { m_Resolve = r; }
    void setBasePath(const QString& path) { m_BasePath = path; }
private:
    CFileIdentifier m_FileName;
    IDeviceBase* m_OwnerDevice;
    IFileLoader* m_Loader;
    QRecursiveMutex mutex;
    bool m_Resolve;
    QString m_BasePath;
};

#endif // CFILEPARAMETER_H
