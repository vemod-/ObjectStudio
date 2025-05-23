#ifndef CSEQUENSERFORM_H
#define CSEQUENSERFORM_H

#include <QDialog>
#include <QMenu>
#include "cbeatframe.h"
#include "csoftsynthsform.h"

#define SEQUENSERCLASS DEVICEFUNC(CSequenser)

namespace Ui {
    class CSequenserForm;
}

class CSequenserForm : public CSoftSynthsForm
{
    Q_OBJECT

public:
    explicit CSequenserForm(IDevice* Device, QWidget *parent = 0);
    ~CSequenserForm();
    void unserializeCustom(const QDomLiteElement* xml);
    void serializeCustom(QDomLiteElement* xml) const;
    void Flash(int Pattern, int Beat);
private:
    Ui::CSequenserForm *ui;
        void UpdatePatterns();
        void UpdateBeats();
        void UpdatePatternlist();
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
        void ChangeNumOfBeats(int Value);
        void ChangeTempo(int Value);
        void PlayListPopup(QPoint Pos);
        void ChangeListIndex(int index);
};

#endif // CSEQUENSERFORM_H
