#ifndef CMASTERWIDGET_H
#define CMASTERWIDGET_H

#include <QFrame>
#include "cstereomixer.h"
#include "cdevicecontainer.h"
#include "qsynthknob.h"
#include <QToolButton>
#include "qlcdlabel.h"
#include "qsignalmenu.h"

namespace Ui {
class CMasterWidget;
}

class CMasterWidget : public QFrame
{
    Q_OBJECT
    
public:
    explicit CMasterWidget(QWidget *parent = nullptr);
    ~CMasterWidget();
    void init(CStereoMixer* mx, QList<CDeviceContainer*>* effects=nullptr);
    void clear();
    void checkPeak();
    void checkEffects();
    void serialize(QDomLiteElement* xml) const;
    void unserialize(const QDomLiteElement* xml);
    void setSoloChannel(int channel);
    void resetPeak();
private:
    Ui::CMasterWidget *ui;
    CStereoMixer* m_Mx;
    QList<CDeviceContainer*>* m_Fx;
    QList<QLCDLabel*> m_Buttons;
    QStringList m_Names;
    QList<QSynthKnob*> dials;
    CDeviceContainer* currentEffect;
    QSignalMenu* effectMenu;
    QAction* unloadAction;
    QAction* showUIAction;
    bool m_Active;
private slots:
    void setVolL(int vol);
    void setVolR(int vol);
    void showEffectMenu(int eff);
    void showEffect(int eff);
    void effectVol(int eff);
    void selectEffect(QString DeviceType);
};

#endif // CMASTERWIDGET_H
