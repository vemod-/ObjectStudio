#ifndef CDRUMMACHINEFORM_H
#define CDRUMMACHINEFORM_H

#include <QDialog>
#include <QMenu>
#include "idevice.h"
#include "cbeatframe.h"

#define DRUMMACHINECLASS DEVICEFUNC(CDrumMachine)

namespace Ui {
class CDrumMachineForm;
}

class CDrumMachineForm : public CSoftSynthsForm
{
    Q_OBJECT

public:
    explicit CDrumMachineForm(IDevice* Device, QWidget *parent = 0);
    ~CDrumMachineForm();
    void Reset();
    void unserializeCustom(const QDomLiteElement* xml);
    void serializeCustom(QDomLiteElement* xml) const;
    void Flash(int Pattern, int Beat);
private:
    Ui::CDrumMachineForm *ui;
    void UpdatePatterns();
    void UpdateBeats();
    void UpdatePatternlist();
    void UpdateSounds();
    void RemovePattern(int Index);
    void RemovePatternInList(int Index);
    void AddPatternToList(int NewIndex, int PatternIndex, int Repeats);	// User declarations
    QList<CBeatFrame*> m_Beats;
    QMenu* PlayListMenu;
private slots:
    void AddPatternClick();
    void RemovePatternClick();
    void MenuAddPatternClick();
    void MenuRemovePatternClick();
    void MenuEditPatternClick();
    void ChangePatternIndex();
    void ChangeName();
    void ChangeInstrument();
    void ChangeNumOfBeats(int Value);
    void ChangeTempo(int Value);
    void PlayListPopup(QPoint Pos);
    void ChangeListIndex(int index);
};

#endif // CDRUMMACHINEFORM_H
