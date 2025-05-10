#ifndef CFILEIDENTIFIER_H
#define CFILEIDENTIFIER_H

#include <QFileInfo>
#include <QDateTime>

class CFileIdentifier
{
public:
    CFileIdentifier() {}
    CFileIdentifier(const QString& filePath) { init(filePath); }
    void init(const QString& filePath) {
        path = filePath;
        info.setFile(path);
        lastModified = info.lastModified();
    }
    QDateTime lastModified;
    QString path;
    QFileInfo info;
    inline bool matches(CFileIdentifier& other) const { return ((info == other.info) && (lastModified == other.lastModified)); }
    inline bool operator == (CFileIdentifier& other) const { return matches(other); }
    inline bool operator != (CFileIdentifier& other) const { return !matches(other); }
    inline CFileIdentifier& operator = (const QString& filePath) {
        init(filePath);
        return *this;
    }
};

#endif // CFILEIDENTIFIER_H
