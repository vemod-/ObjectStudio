#ifndef IAUDIOPLUGINHOST_H
#define IAUDIOPLUGINHOST_H

#include "cmacwindow.h"
#include "MacTypes.h"
#include "csimplebuffer.h"
#include "imidiparser.h"
#include "cfileparameter.h"
#include <QMenu>

class IAudioPlugInHost : public CMacWindow, public IMIDIParser, public IFileLoader, protected IPresetRef
{
    Q_OBJECT
public:
    IAudioPlugInHost(QWidget* parent=nullptr);
    virtual const QString bankPresetName() const { return QString(); }
    virtual const QStringList bankPresetNames() const { return QStringList(); }
    virtual void setBankPreset(const long /*index*/) {}
    virtual long currentBankPreset(const int /*channel*/=-1) const { return -1; }
    virtual void serialize(QDomLiteElement* /*xml*/) const {}
    virtual void unserialize(const QDomLiteElement* /*xml*/) {}
    virtual const QPixmap* picture();
    virtual const QString filename() const { return QString(); }
    virtual bool loadFile(const QString& /*path*/) { return false; }
    virtual bool process(){ return false; }
    virtual bool isMono() const { return false; }
    CChannelBuffer InBuffers;
    CChannelBuffer OutBuffers;
    virtual const QSize UISize() const { return QSize(); }
public slots:
    virtual void popup(QPoint /*pos*/){}
signals:
    void PlugInChanged();
protected:
    QMenu* mMainMenu;
    Float64 m_Samplerate;
    uint m_Buffersize;
    QStringList m_ProgramNames;
    QString m_ProgramName;
    QPixmap backPix;
    void resizeEvent(QResizeEvent *e);
    void showEvent(QShowEvent *e);
};

#endif // IAUDIOPLUGINHOST_H
