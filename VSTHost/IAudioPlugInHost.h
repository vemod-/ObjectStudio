#ifndef IAUDIOPLUGINHOST_H
#define IAUDIOPLUGINHOST_H

#include "softsynthsclasses.h"
#include "cmacwindow.h"
#include "qsignalmenu.h"
#include "MacTypes.h"

class IAudioPlugInHost : public CMacWindow
{
    Q_OBJECT
public:
    IAudioPlugInHost(unsigned int sampleRate, unsigned int bufferSize, QWidget* parent=0):CMacWindow(parent)
    {
        m_Samplerate=sampleRate;
        m_Buffersize=bufferSize;
        m_MIDIChannel=0;
    }
    virtual const QString ProgramName(){ return QString(); }
    virtual const QStringList ProgramNames(){ return QStringList(); }
    virtual void SetProgram(const long index) {}
    virtual const long CurrentProgram(){ return -1; }
    virtual const QString SaveXML(){ return QString(); }
    virtual void LoadXML(const QString& XML){ Q_UNUSED(XML) }
    virtual void AllNotesOff(){}
    int MIDIChannel(){ return m_MIDIChannel; }
    void setMIDIChannel(int value){ m_MIDIChannel=value; }
    const QString Filename(){ return m_Filename; }
    virtual const bool Process(){ return false; }
    virtual void DumpMIDI(CMIDIBuffer* MB, bool PatchChange) { Q_UNUSED(MB) Q_UNUSED(PatchChange) }
    float** InBuffers;
    float** OutBuffers;
public slots:
    virtual void Popup(QPoint pos){ Q_UNUSED(pos) }
signals:
    void PlugInChanged();
protected:
    Float64 m_Samplerate;
    unsigned int m_Buffersize;
    int m_MIDIChannel;
    QString m_Filename;
    QStringList m_ProgramNames;
    QString m_ProgramName;
    QMenu* mMainMenu;
    QList<QSignalMenu*> mSubMenus;
};

#endif // IAUDIOPLUGINHOST_H
