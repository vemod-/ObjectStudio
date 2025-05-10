#ifndef CCHANNELEFFECTS_H
#define CCHANNELEFFECTS_H

#include <QFrame>
#include "qsynthknob.h"
#include "cstereomixer.h"


namespace Ui {
class CChannelEffects;
}

class CChannelEffects : public QFrame
{
    Q_OBJECT

public:
    explicit CChannelEffects(QWidget *parent = nullptr);
    ~CChannelEffects();
    void init(CStereoMixerChannel* ch);
    void unserialize(const QDomLiteElement* xml);
    void serialize(QDomLiteElement* xml) const;
    bool isSolo();
public slots:
    void setPanValue(int v) {
        setPan(v + 100);
    }
    void setSolo(bool v);
    void setMute(bool v);
signals:
    void soloTriggered(bool v);
    void panValueChanged(int v);
private slots:
    void setPan(int v);
    void setEffect(int e, int v);
    void setEffect(int e);
    void setBypass(bool v);
    void triggerPanValueChanged(int v) {
        emit panValueChanged(v - 100);
    }
private:
    Ui::CChannelEffects *ui;
    QList<QSynthKnob*> Effect;
    CStereoMixerChannel* m_Ch;
};

#endif // CCHANNELEFFECTS_H
