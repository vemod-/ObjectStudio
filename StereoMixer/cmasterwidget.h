#ifndef CMASTERWIDGET_H
#define CMASTERWIDGET_H

#include <QFrame>
#include <cstereomixer.h>
#include <softsynthsclasses.h>
#include <qsynthknob.h>
#include <QToolButton>
#include "qlcdlabel.h"
#include <qsignalmenu.h>

namespace Ui {
class CMasterWidget;
}

class CMasterWidget : public QFrame
{
    Q_OBJECT
    
public:
    explicit CMasterWidget(QWidget *parent = 0);
    ~CMasterWidget();
    void Init(CStereoMixer* mx, QList<IDevice*>* effects=NULL);
    void checkPeak();
    void checkEffects();
    const QString Save();
    void Load(const QString& XML);
    void setSoloChannel(int channel);
    void resetPeak();
private:
    Ui::CMasterWidget *ui;
    CStereoMixer* m_Mx;
    QList<IDevice*>* m_Fx;
    QList<QLCDLabel*> m_Buttons;
    QStringList m_Names;
    QSignalMapper* effMenuMapper;
    QSignalMapper* effShowMapper;
    QSignalMapper* dialMapper;
    QList<QSynthKnob*> dials;
    IDevice* currentEffect;
    QSignalMenu* effectMenu;
    QAction* unloadAction;
    QAction* showUIAction;
private slots:
    void setVolL(int vol);
    void setVolR(int vol);
    void showEffectMenu(int eff);
    void showEffect(int eff);
    void effectVol(int eff);
    void selectEffect(QString DeviceType);
protected:
    void showEvent(QShowEvent *);
};

#endif // CMASTERWIDGET_H
