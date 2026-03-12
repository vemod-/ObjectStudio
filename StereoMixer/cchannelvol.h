#ifndef CCHANNELVOL_H
#define CCHANNELVOL_H

#include <QFrame>
#include <QDomLite>
#include "cstereomixer.h"

namespace Ui {
class CChannelVol;
}

class CChannelVol : public QFrame
{
    Q_OBJECT

public:
    explicit CChannelVol(QWidget *parent = nullptr);
    ~CChannelVol();
    void init(CStereoMixerChannel* ch) {
        m_Ch = ch;
    }
    int vol();

    void unserialize(const QDomLiteElement* xml);
    void serialize(QDomLiteElement* xml) const;
signals:
    void volChanged(int v);
public slots:
    void setVol(int v);
    void peak(float l, float r);
    void resetPeak();
protected:
    void showEvent(QShowEvent* e);
private:
    Ui::CChannelVol *ui;
    CStereoMixerChannel* m_Ch;
};

#endif // CCHANNELVOL_H
