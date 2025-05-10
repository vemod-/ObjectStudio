#include "cpresets.h"
#include <QFileDialog>
#include <QMessageBox>

const QString CPresets::resolveFilename(const QString &Filename)
{
    if (Filename.isEmpty()) return Filename;
    const QFileInfo fi(Filename);
    if (fi.exists()) return Filename;
    QString absoluteFilePath=fi.canonicalFilePath();
    if (absoluteFilePath.isEmpty()) absoluteFilePath=fi.absoluteFilePath();
    if (getInstance()->m_ReplacementFiles.contains(absoluteFilePath.toLower())) return getInstance()->m_ReplacementFiles[absoluteFilePath.toLower()].toString();
    QMessageBox::critical(nullptr,"Missing file","The file "+fi.fileName()+" is missing");
    const QString FN=QFileDialog::getOpenFileName(nullptr,"Replace file "+fi.fileName(),fi.absolutePath());
    if (!FN.isEmpty()) getInstance()->m_ReplacementFiles.insert(absoluteFilePath.toLower(),FN);
    return FN;
}
