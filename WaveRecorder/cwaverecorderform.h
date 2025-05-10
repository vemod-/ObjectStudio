#ifndef CWAVERECORDERFORM_H
#define CWAVERECORDERFORM_H

#include "csoftsynthsform.h"
#include <QMenu>
#include <QListWidget>
#include <cdevicelist.h>
#include <QMenuBar>
#include "cwavedocument.h"

namespace Ui {
    class CWaveRecorderForm;
}

class CWaveRecorderForm : public CSoftSynthsForm
{
    Q_OBJECT

public:
    explicit CWaveRecorderForm(IDevice* Device, QWidget *parent = nullptr);
    ~CWaveRecorderForm();
    void serializeCustom(QDomLiteElement* xml) const;
    void unserializeCustom(const QDomLiteElement* xml);
    CAudioBuffer* getNextA(const int ProcIndex);
    void setHost(IHost* h);
    float volumeL();
    float volumeR();
    void showMixer(bool show);
    void setPlayIcon(bool v);
    void initWithFile(const QString& path);
protected:
    void timerEvent(QTimerEvent* e);
private slots:
    void addFile(QString path);
    void Import();
    void Remove();
    void FileMenuPopup(QPoint Pos);
    void Record(bool v);
    void RecordFromStart(bool v);
    void SkipToStart();
    void togglePlay();
    void Play(bool v);
    void DeleteFile();
    void RenameFile(QListWidgetItem* item);
    void CheckList();
    void SetMonitor(bool v);
    void SetMonitorLevel(int v);
private:
    Ui::CWaveRecorderForm *ui;
    //QMenu* Popup;
    //QMenu *menuFile;
    QString m_RecordPath;
    CWaveDocument* m_Document;
};

#endif // CWAVERECORDERFORM_H
