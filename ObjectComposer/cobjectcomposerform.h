#ifndef COBJECTCOMPOSERFORM_H
#define COBJECTCOMPOSERFORM_H

#include <QWidget>
#include "csoftsynthsform.h"
#include "ijack.h"
#include "cscoredoc.h"
//#include "cdocumentloader.h"

namespace Ui {
class CObjectComposerForm;
}

class CObjectComposerForm : public CSoftSynthsForm
{
    Q_OBJECT

public:
    explicit CObjectComposerForm(IDevice* Device, QWidget *parent = nullptr);
    ~CObjectComposerForm();
    CInJack* insideIn;
    //bool event(QEvent *event);
    void UpdateVol(int v);
    void setMIDI2wavParameter(QString parameterName, int value);
    //QSize sizeHint() const;
    void serializeCustom(QDomLiteElement* xml) const;
    void unserializeCustom(const QDomLiteElement* xml);
    void initWithFile(const QString& path);
private:
    CScoreDoc* m_Document;
    //CProjectApp* m_ProjectApp;
    //CProjectPage* m_ProjectPage;
    //OCPlayControl* m_PlayControl;
    //void NoteOnOff(bool On, int Pitch, int MixerTrack, OCMIDIVars MIDIInfo);
    void SetVol(int v);
    Ui::CObjectComposerForm *ui;
};

#endif // COBJECTCOMPOSERFORM_H
