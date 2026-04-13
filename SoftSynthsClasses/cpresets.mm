#include "cpresets.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>
#include <QApplication>

bool isFileReadyMac(const QString& path)
{
    @autoreleasepool {
        NSURL *url = [NSURL fileURLWithPath:path.toNSString()];

        NSNumber *isUbiquitous = nil;
        [url getResourceValue:&isUbiquitous
                      forKey:NSURLIsUbiquitousItemKey
                       error:nil];

        if (isUbiquitous.boolValue)
        {
            NSNumber *isDownloaded = nil;
            [url getResourceValue:&isDownloaded
                          forKey:NSURLUbiquitousItemIsDownloadedKey
                           error:nil];
            return isDownloaded.boolValue;
        }
        return true;
    }
}

bool isFileReadyIOS(const QString& path)
{
    @autoreleasepool {
        NSURL *url = [NSURL fileURLWithPath:path.toNSString()];

        [url startAccessingSecurityScopedResource];

        NSNumber *isUbiquitous = nil;
        [url getResourceValue:&isUbiquitous
                      forKey:NSURLIsUbiquitousItemKey
                       error:nil];

        if (isUbiquitous.boolValue)
        {
            NSNumber *isDownloaded = nil;
            [url getResourceValue:&isDownloaded
                          forKey:NSURLUbiquitousItemIsDownloadedKey
                           error:nil];

            [url stopAccessingSecurityScopedResource];
            return isDownloaded.boolValue;
        }

        [url stopAccessingSecurityScopedResource];
        return true;
    }
}

void startDownloadMac(const QString& path)
{
    @autoreleasepool {
        NSURL *url = [NSURL fileURLWithPath:path.toNSString()];
        [[NSFileManager defaultManager]
            startDownloadingUbiquitousItemAtURL:url
                                          error:nil];
    }
}

int getICloudDownloadProgress(const QString& path) {
    @autoreleasepool {
        NSURL *url = [NSURL fileURLWithPath:path.toNSString()];

        NSNumber *percent = nil;
        NSError *error = nil;

        BOOL success = [url getResourceValue:&percent
                                      forKey:NSURLUbiquitousItemPercentDownloadedKey
                                       error:&error];

        if (!success || percent == nil) {
            return -1; // okänd
        }

        return percent.intValue; // 0–100
    }
}
/*
bool isDownloading(const QString& path) {
    @autoreleasepool {
        NSURL *url = [NSURL fileURLWithPath:path.toNSString()];

        NSString *status = nil;
        [url getResourceValue:&status
                       forKey:NSURLUbiquitousItemDownloadingStatusKey
                        error:nil];

        return [status isEqualToString:NSURLUbiquitousItemDownloadingStatusCurrent];
    }
}
*/
/*
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
*/
const QString CPresets::resolveFilename(const QString &Filename)
{
    if (Filename.isEmpty()) return Filename;

    QFileInfo fi(Filename);
    if (fi.exists())
    {
#ifdef Q_OS_MAC
/*
        if (!isFileReadyMac(Filename)) {

            startDownloadMac(Filename);

            showNativeICloudProgress(qApp->activeWindow(),
                "Downloading " + QFileInfo(Filename).fileName());

            QTimer* timer = new QTimer(nullptr);

            QObject::connect(timer, &QTimer::timeout, [=]() {

                int p = getICloudDownloadProgress(Filename);

                updateNativeICloudProgress(p); // -1 → spinner

                if (isFileReadyMac(Filename)) {
                    timer->stop();
                    hideNativeICloudProgress();
                }
            });

            timer->start(500);
*/
        if (!isFileReadyMac(Filename)) {
            // försök trigga nedladdning
            startDownloadMac(Filename);

            QProgressDialog* progress = new QProgressDialog();
            progress->setWindowFlag(Qt::WindowStaysOnTopHint);
            progress->setWindowFlag(Qt::Tool);
            progress->setWindowModality(Qt::NonModal);
            progress->setMinimumDuration(0); // visa direkt
            progress->setLabelText("Downloading from iCloud...\n" + QFileInfo(Filename).fileName());
            progress->setRange(0, 0);
            progress->show();

            QTimer* timer = new QTimer();

            QObject::connect(timer, &QTimer::timeout, [=]() {
                qApp->processEvents();

/*
                int p = getICloudDownloadProgress(Filename);
                qDebug() << p;
                if (p < 0) {
                    progress->setRange(0, 0);
                } else {
                    progress->setRange(0, 100);
                    progress->setValue(p);
                    progress->setLabelText(
                        QString("Downloading %1...\n%2%")
                            .arg(QFileInfo(Filename).fileName())
                            .arg(p)
                    );
                }
*/
                if (isFileReadyMac(Filename)) {
                    timer->stop();
                    progress->setValue(100);
                    progress->close();
                    progress->deleteLater();
                    timer->deleteLater();
                    qApp->processEvents();
                }
            });

            QObject::connect(progress, &QProgressDialog::canceled, [=]() {
                timer->stop();
                progress->close();
                timer->deleteLater();
            });

            timer->start(100);
            qApp->processEvents();
/*
                if (isFileReadyMac(Filename)) {
                    timer->stop();
                    progress->close();
                    progress->deleteLater();
                }
            });

            QObject::connect(progress, &QProgressDialog::canceled, [=]() {
                timer->stop();
                progress->close();
            });

            timer->start(500);
*/
        }
#endif
#ifdef Q_OS_IOS
        if (!isFileReadyIOS(Filename)) {
            // försök trigga nedladdning
            startDownloadMac(Filename);

            QMessageBox::information(nullptr, "iCloud",
                "The file is stored in iCloud and is being downloaded");
            while (!isFileReadyIOS(Filename)) QThread::msleep(500);
        }
#endif
        return Filename;
    }

    QString absoluteFilePath = fi.canonicalFilePath();
    if (absoluteFilePath.isEmpty())
        absoluteFilePath = fi.absoluteFilePath();

    if (getInstance()->m_ReplacementFiles.contains(absoluteFilePath.toLower()))
        return getInstance()->m_ReplacementFiles[absoluteFilePath.toLower()].toString();

    QMessageBox::critical(nullptr,"Missing file","The file "+fi.fileName()+" is missing");

    const QString FN = QFileDialog::getOpenFileName(nullptr,
        "Replace file "+fi.fileName(), fi.absolutePath());

    if (!FN.isEmpty())
        getInstance()->m_ReplacementFiles.insert(absoluteFilePath.toLower(), FN);

    return FN;
}
